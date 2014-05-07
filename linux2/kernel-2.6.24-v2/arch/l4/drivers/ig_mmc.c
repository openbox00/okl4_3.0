/*
 * @LICENCE("Open Kernel Labs, Inc.", "2007")@
 */
/*
 * Author: Peter Howard
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/mmc/host.h>
#include <linux/mmc/protocol.h>
#include <asm-l4/scatterlist.h>
#include <linux/scatterlist.h>

#include <interfaces/devicecore_client.h>
#include <interfaces/vmmc_client.h>

#include <iguana/types.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/env.h>

#include <assert.h>

#include "ig_mmc.h"

#define DRIVER_NAME "ig_mmc"
#define DRIVER_VERSION "0.2"

//#ifdef CONFIG_MMC_DEBUG
#if 1
#define DBG(x...) \
    printk(KERN_INFO DRIVER_NAME ": " x)
#define DBGF(f, x...) \
    printk(KERN_INFO DRIVER_NAME " [%s()]: " f, __func__ , ##x)
#else
#define DBG(x...)   do { } while (0)
#define DBGF(x...)  do { } while (0)
#endif

extern int iguana_alloc_irq(void);

static inline void mmc_delay(unsigned int ms)
{
	if (ms < HZ / 1000) {
		yield();
		mdelay(ms);
	} else {
		msleep_interruptible (ms);
	}
}

/*
 * Translating from Linux's return values to Iguana's
 */

static enum mmc_response_fmt 
resp_tr(int num)
{
    /*
     * Given the "sparseness" of the valid values, and array based 
     * translation will waste a lot of space, and the switch based form
     * shouldn't be too ugly.
     */

    switch (num) {
    case MMC_RSP_NONE:
        return eMMC_NO_RESPONSE;
    case MMC_RSP_R1:
        return eMMC_FR1;
    case MMC_RSP_R1B:
        return eMMC_FR1_B;
    case MMC_RSP_R2:
        return eMMC_FR2;
    case MMC_RSP_R3:
        return eMMC_FR3;
        /* Unfortunately, MMC_RSP_R1 and MMC_RSP_R1 have identical values */
        /* Have to "hack" the response type further down */
//    case MMC_RSP_R6:
//        return eMMC_FR6;
    default:
        assert(0);
        return 0; /* To avoid compiler warning */
    }
}

/*
 * MMC Callbacks
 */

static void ig_mmc_request(struct mmc_host* mmc, struct mmc_request* mrq)
{
    struct ig_mmc_host* host = mmc_priv(mmc);
    struct mmc_command* cmd;
    struct vmmc_request *req = (struct vmmc_request *)host->shared_base;
    

    /*
     * Disable tasklets to avoid a deadlock.
     */
    spin_lock_bh(&host->lock);

    BUG_ON(host->mrq != NULL);

    cmd = mrq->cmd;

    host->mrq = mrq;
    

    req->cmd_no = mrq->cmd->opcode;
    req->arg = mrq->cmd->arg;
    req->response_type = resp_tr(mrq->cmd->flags);

    /* Is there data ? */
    if (mrq->cmd->data) {
        struct mmc_data *data = mrq->cmd->data;
        req->dirn = (data->flags & MMC_DATA_WRITE ? 
                     eMMC_BLK_WRITE : 
                     eMMC_BLK_READ);
        req->block_count = data->blocks;
        req->block_size = 1 << data->blksz_bits; 
        req->block_current = 0;
        req->block_incr = 0;

        /* Now map in scatterlist */
        {
            struct scatterlist *list = data->sg;
            size_t cur_off = list[0].offset;
            size_t elt = 0;
            size_t i = 0;
            do {
                req->block_data[i++].data = (void *)(page_to_phys(list[elt].page) + 
                    cur_off);
                cur_off += req->block_size;
                if (cur_off == ONE_PAGE) {
                    elt++;
                    cur_off = 0;
                }
            } while (i < req->block_count); 
        }
    } else {
        req->dirn = eMMC_BLK_NONE;
    }


    if (virtual_mmc_request(host->server,
                            host->dev,
                            NULL)) {
        /* Indicates that we're not allowed to access the MMC.  This would be
           because another wombat instance currently "owns" the MMC.  Fake a
           "TIMEOUT" response so that the framework will treat the situation
           as "no card". */
        host->mrq = NULL;
        mrq->cmd->error = MMC_ERR_TIMEOUT;
        mmc_request_done(host->mmc, mrq);
    }

    spin_unlock_bh(&host->lock);
}

static void ig_mmc_set_ios(struct mmc_host* mmc, struct mmc_ios* ios)
{
    /* Setting the clock seems to stuff things up; ignore */
     struct ig_mmc_host* host = mmc_priv(mmc);
    
    DBGF("clock %uHz busmode %u powermode %u Vdd %u buswidth %u\n",
           ios->clock,
           ios->bus_mode,
           ios->power_mode,
           ios->vdd,
           ios->bus_width);

    spin_lock_bh(&host->lock);

    virtual_mmc_set_bus_width(host->server,
                              host->dev,
                              ios->bus_width,
                              NULL);
/* Note: at this time, there are still issues with letting the linux mmc
   framework set the clock rate.  Primarily around the system locking up when
   you change cards.  So for now it's set in the iguana code and never
   changed. */
#if 0
    virtual_mmc_set_clock(host->server,
                          host->dev,
                          ios->clock,
                          NULL);
#endif

    spin_unlock_bh(&host->lock);
}


/*
 * The only "interrupt" we care about at this level is being informed of a 
 * card being inserted/removed. 
 */

static irqreturn_t 
ig_mmc_irq(int irq, void *dev_id)
{
    struct ig_mmc_host *host = (struct ig_mmc_host *)dev_id;

    if (irq == host->irq_gpio) {
        mmc_detect_change(host->mmc, 0);
    } else if (irq == host->irq_mmc) {
        struct mmc_request *mrq = host->mrq;
        struct vmmc_request *req = (struct vmmc_request *)host->shared_base;
        assert(mrq != NULL);
        /* Was this a data transfer requiring a stop command? */
        if (mrq->stop && ! host->fin_wait && (mrq->cmd->data->blocks > 1)) {
            mrq->data->bytes_xfered = req->bytes_xfered;
            host->fin_wait = 1;
            queue_work(host->queue, &host->finish);
        } else {
            /* Check for the single block xfer */
            if (mrq->cmd->data && (mrq->cmd->data->blocks == 1)) {
                mrq->data->bytes_xfered = req->bytes_xfered;
            }
            /* Get the status back into mrq */
            host->mrq = NULL;
            host->fin_wait = 0;
            if (req->resp_size > 0) {
                if (req->resp_size == 16) {
                    /* The linux MMC code appears to have been written for
                       big-endian on 32 bit words.  Given this code is being
                       used on a little-endian system (ARM) the result is
                       MIXED ENDIAN on 32 bit words; i.e. each 32 bit word is
                       little endian but groups of 32 bit words are in big
                       endian ordering. Refer to the macro UNSTUFF_BITS in
                       mmc.c for how I came to this conclusion. */
                    uint32_t buf[4];
                    memcpy (buf,
                            req->response, 
                            req->resp_size);
                    mrq->cmd->resp[3] = buf[0];
                    mrq->cmd->resp[2] = buf[1];
                    mrq->cmd->resp[1] = buf[2];
                    mrq->cmd->resp[0] = buf[3];
                } else {
                    memcpy (mrq->cmd->resp,
                            &req->response[1], 
                            req->resp_size - 1);
                }
            }
            mrq->cmd->error = req->result;
            mmc_request_done(host->mmc, mrq);
        }
    }
    return IRQ_HANDLED;
}

/*
 * Support functions for probe
 */

static int 
ig_mmc_scan(struct ig_mmc_host* host)
{
    /* For now, we know what hardware we're running on and 
       what is there, just return 0.
       Next version; probe Iguana server to be sure it's there */
    return 0;
}

static void 
send_finish(void *data)
{
    struct ig_mmc_host *host = (struct ig_mmc_host *)data;
    struct vmmc_request *req = (struct vmmc_request *)host->shared_base;

    /*printk(KERN_INFO "in send_finish()\n"); */
    req->cmd_no = host->mrq->stop->opcode;
    req->arg = host->mrq->stop->arg;
    req->response_type = resp_tr(host->mrq->stop->flags);
    req->dirn = eMMC_BLK_NONE;
    
    virtual_mmc_request(host->server,
                        host->dev,
                        NULL);
}

static struct mmc_host_ops ig_mmc_ops = {
    .request    = ig_mmc_request,
    .set_ios    = ig_mmc_set_ios,
};

/*
 * Device probe
 */

extern L4_ThreadId_t timer_thread;

static int
ig_mmc_probe(struct device* dev)
{
    struct ig_mmc_host* host = NULL;
    struct mmc_host* mmc = NULL;
    int ret;

    /* Note: iguana driver will automatically do "real" initialisation.
       We just have to connect to the Iguana server */

    thread_ref_t server_;
    L4_ThreadId_t server;
    CORBA_Environment env;
    uint32_t mask;

    /* Get access to devicecore server */
    memsection_lookup(env_memsection_base(iguana_getenv("OKL4_CORE_DEVICE_SERVER")),
                      &server_);
    server = thread_l4tid(server_);
    
    /*
     * Allocate and initialise MMC structure.
     */
    if ((mmc = mmc_alloc_host(sizeof(struct ig_mmc_host), dev)) == NULL) {
        return -ENOMEM;
    }
    host = mmc_priv(mmc);
    host->mmc = mmc;
    mmc->ocr_avail = 0x00ffc000; /**< From example code */
    mmc->ops = &ig_mmc_ops;
    mmc->caps = MMC_CAP_4_BIT_DATA;
    host->shared_ms = alloc_shared_memsection(host->shared_base);

    /* Workqueue preparation */
    {
        host->fin_wait = 0;
        host->queue = create_singlethread_workqueue("vmmc");
        INIT_WORK(&host->finish, send_finish, host);
    }

    /* Allocate iguana interrupts */
    host->irq_gpio = iguana_alloc_irq();
    printk("IG_MMC-GPIO on IRQ %d\n", host->irq_gpio);
    host->irq_mmc = iguana_alloc_irq();
    printk("IG_MMC on IRQ %d\n", host->irq_mmc);
	request_irq(host->irq_gpio, ig_mmc_irq, 0, "MMC", host);
	request_irq(host->irq_mmc, ig_mmc_irq, 0, "MMC", host);
    mask = IGUANA_IRQ_NOTIFY_MASK(host->irq_gpio) | IGUANA_IRQ_NOTIFY_MASK(host->irq_mmc);

    /* Now get access to real device, and give it our shared mem section */
    host->dev = device_core_get_mmc(server,
                                    &(host->server),
                                    &timer_thread, 
                                    mask,
                                    &env);
    virtual_mmc_add_memsection(host->server,
                               host->dev,
                               host->shared_ms,
                               NULL);

    /*
     * Scan for hardware.
     */
    ret = ig_mmc_scan(host);
    if (ret)
        goto freemmc;

    
    
    /*
     * Maximum number of segments. Worst case is one sector per segment
     * so this will be 64kB/512.
     */
    //mmc->max_hw_segs = 128;
    //mmc->max_phys_segs = 128;
    
    /*
     * Maximum number of sectors in one transfer. Also limited by 64kB
     * buffer.
     */
    //mmc->max_sectors = 128;
    
    /*
     * Maximum segment size. Could be one segment with the maximum number
     * of segments.
     */
    //mmc->max_seg_size = mmc->max_sectors * 512;
    
    
    
    /*
     * Add host to MMC layer.
     */
    mmc_add_host(mmc);

    return 0;

freemmc:
    mmc_free_host(mmc);

    return ret;
}

/*
 * Device remove
 */

static int ig_mmc_remove(struct device* dev)
{
    struct mmc_host* mmc = dev_get_drvdata(dev);
    struct ig_mmc_host* host;
    
    printk(KERN_INFO DRIVER_NAME ": ig_mmc_remove\n");
    if (!mmc)
        return 0;


    host = mmc_priv(mmc);
    flush_workqueue(host->queue);
    
    /*
     * Unregister host with MMC layer.
     */
    mmc_remove_host(mmc);

    mmc_free_host(mmc);

    return 0;
}

/*
 * Power management
 */


#define ig_mmc_suspend NULL
#define ig_mmc_resume NULL

static void ig_release(struct device *dev)
{
    printk(KERN_INFO DRIVER_NAME ":ig_release()\n");
    return; /* No action */
}

static struct platform_device ig_device = {
    .name       = DRIVER_NAME,
    .id         = -1,
    .dev        = {
        .release = ig_release,
    },
};

static struct device_driver ig_driver = {
    .name       = DRIVER_NAME,
    .bus        = &platform_bus_type,
    .probe      = ig_mmc_probe,
    .remove     = ig_mmc_remove,
    
    .suspend    = ig_mmc_suspend,
    .resume     = ig_mmc_resume,
};

/*
 * Module loading/unloading
 */

static int __init ig_mmc_drv_init(void)
{
    int result;
    
    printk(KERN_INFO DRIVER_NAME
           ": Iguana virtual SD/MMC interface driver, "
        DRIVER_VERSION "\n");
    printk(KERN_INFO DRIVER_NAME ": Copyright(c) Open Kernel Labs Inc.\n");


    /* Finally, register driver and device with linux */
    result = driver_register(&ig_driver);
    if (result < 0)
        return result;

    result = platform_device_register(&ig_device);
    if (result < 0)
        return result;

    return 0;
}

static void __exit ig_mmc_drv_exit(void)
{
    platform_device_unregister(&ig_device);
    
    driver_unregister(&ig_driver);

    DBG("ig_mmc unloaded\n");
}

module_init(ig_mmc_drv_init);
module_exit(ig_mmc_drv_exit);

MODULE_LICENSE("OZPLB");
MODULE_DESCRIPTION("Iguana virtual mmc interface driver");
MODULE_VERSION(DRIVER_VERSION);
