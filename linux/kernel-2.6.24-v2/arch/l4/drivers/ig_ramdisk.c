/*
 * A sample, extra-simple block driver.
 *
 * Copyright 2003 Eklektix, Inc.  Redistributable under the terms
 * of the GNU GPL.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

#if defined(CONFIG_IGUANA)
#include <iguana/env.h>
#include <iguana/memsection.h>
#else
#include <okl4/env.h>
#endif

MODULE_LICENSE("Dual BSD/GPL");
//static char *Version = "1.3";

static int major_num = 0;

module_param(major_num, int, 0);
static int hardsect_size = 512;

module_param(hardsect_size, int, 0);
static int nsectors;  /* How big the drive is */

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE 512

static uintptr_t addr;

static int __init igms_name(char *str)
{
#if defined(CONFIG_IGUANA)
	thread_ref_t unused;
   addr = (uintptr_t) env_memsection_base(iguana_getenv("ROOTFS"));
	nsectors = memsection_size(memsection_lookup(addr, &unused)) / KERNEL_SECTOR_SIZE;
#else
	word_t seg;
	okl4_env_segments_t *segs;
	int i;

	seg = *(word_t *)okl4_env_get("MAIN_ROOTFS");
	segs = OKL4_ENV_GET_SEGMENTS("SEGMENTS");
	for (i = 0; i < segs->segments; i++) {
		if (segs->segments[i].segment == seg) {
			addr = segs->segments[i].virt_addr;
			nsectors = segs->segments[i].size / KERNEL_SECTOR_SIZE;
			break;
		}
	}
#endif
	return 1;
}

__setup("igms_name", igms_name);

/*
 * Our request queue.
 */
static struct request_queue *Queue;

/*
 * The internal representation of our device.
 */
static struct igms_device {
    unsigned long size;
    spinlock_t lock;
    u8 *data;
    struct gendisk *gd;
} Device;


/*
 * Handle an I/O request.
 */
static void igms_transfer(struct igms_device *dev, unsigned long sector,
		unsigned long nsect, char *buffer, int write)
{
    unsigned long offset = sector*hardsect_size;
    unsigned long nbytes = nsect*hardsect_size;

#if 0    
    if ((offset + nbytes) > dev->size) {
	printk (KERN_NOTICE "igms: Beyond-end write (%ld %ld)\n", offset, nbytes);
	return;
    }
#endif
    if (write) {
	memcpy((void*) (addr + offset), buffer, nbytes);
    } else {
	memcpy(buffer, (void*) (addr + offset), nbytes);
    }
}

static void igms_request(request_queue_t *q)
{
    struct request *req;
    while ((req = elv_next_request(q)) != NULL) {
	if (! blk_fs_request(req)) {
	    printk (KERN_NOTICE "Skip non-CMD request\n");
	    end_request(req, 0);
	    continue;
	}
	igms_transfer(&Device, req->sector, req->current_nr_sectors,
			req->buffer, rq_data_dir(req));
	end_request(req, 1);
    }
}

/*
 * Ioctl.
 */
int igms_ioctl (struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
    long size;
    struct hd_geometry geo;
    switch(cmd) {
	/*
	 * The only command we need to interpret is HDIO_GETGEO, since
	 * we can't partition the drive otherwise.  We have no real
	 * geometry, of course, so make something up.
	 */
    case HDIO_GETGEO:
	size = Device.size*(hardsect_size/KERNEL_SECTOR_SIZE);
	geo.cylinders = (size & ~0x3f) >> 6;
	geo.heads = 4;
	geo.sectors = 16;
	geo.start = 4;
	if (copy_to_user((void *) arg, &geo, sizeof(geo)))
	    return -EFAULT;
	return 0;
    }

    return -ENOTTY; /* unknown command */
}




/*
 * The device operations structure.
 */
static struct block_device_operations igms_ops = {
    .owner           = THIS_MODULE,
    .ioctl	     = igms_ioctl
};

static int __init igms_init(void)
{
/*
 * Set up our internal device.
 */
    Device.size = nsectors*hardsect_size;
    spin_lock_init(&Device.lock);
/*
 * Get a request queue.
 */
    Queue = blk_init_queue(igms_request, &Device.lock);
    if (Queue == NULL)
	    goto out;
    blk_queue_hardsect_size(Queue, hardsect_size);
/*
 * Get registered.
 */
    major_num = register_blkdev(major_num, "igms");

    if (major_num <= 0) {
	printk(KERN_WARNING "igms: unable to get major number\n");
	goto out;
    }
/*
 * And the gendisk structure.
 */
    Device.gd = alloc_disk(16);
    if (! Device.gd)
	goto out_unregister;
    Device.gd->major = major_num;
    Device.gd->first_minor = 0;
    Device.gd->random = 0;
    Device.gd->fops = &igms_ops;
    Device.gd->private_data = &Device;
    strcpy (Device.gd->disk_name, "igms0");
    set_capacity(Device.gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
    Device.gd->queue = Queue;
    add_disk(Device.gd);

    printk("Iguana ramdisk driver initialized\n");

    return 0;

  out_unregister:
    unregister_blkdev(major_num, "igms");
  out:
    return -ENOMEM;
}

static void __exit
igms_exit(void)
{
    del_gendisk(Device.gd);
    put_disk(Device.gd);
    unregister_blkdev(major_num, "igms");
    blk_cleanup_queue(Queue);
}
	
module_init(igms_init);
module_exit(igms_exit);
