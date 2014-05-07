
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <mutex/mutex.h>

#include <asm/system.h>
#include <asm/io.h>

#include <interfaces/devicecore_client.h>
#include <interfaces/vnet_client.h>

#include <iguana/types.h>
#include <net/net.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/env.h>

#define TERMINATED 1
#define COMPLETED 2

#define nprintk(x...) { if (ig_net->debug) printk(x); }

extern int iguana_alloc_irq(void);

struct control_block {
	unsigned long status;

	//struct ig_mtd_info info;

	struct mutex queue_lock;

	/* list of pending packets (doubly linked) */
	struct client_cmd_packet *tx_pend_head;
	struct client_cmd_packet *tx_pend_last;

	struct client_cmd_packet *rx_pend_head;
	struct client_cmd_packet *rx_pend_last;
};


/* Information that need to be kept for each board. */
struct ig_net {
    objref_t dev;
    L4_ThreadId_t server;

    struct semaphore sem;

    struct net_device_stats stats;

    memsection_ref_t control_ms;
    uintptr_t cmd_base;
    struct control_block *control;

    struct client_cmd_packet *free_list;
    struct client_cmd_packet *last_free;

    int available;
    int stopped;

    volatile int debug;
};

static struct client_cmd_packet *
alloc_cmd_packet(struct ig_net *vnet)
{
	struct client_cmd_packet *packet;

	packet = vnet->free_list;
	if (!packet){
		return NULL;
	}

	vnet->free_list = packet->next;
	if (vnet->free_list == NULL)
		vnet->last_free = NULL;

	return packet;
}

static void dequeue_cmd_packet (struct ig_net *vnet, struct client_cmd_packet *packet){
		struct control_block *control = vnet->control;
        if (packet != control->tx_pend_head){
            printk("WARN: Try to free non-head!\n");
        }
       // down(&vnet->sem);
        
        control->tx_pend_head = packet->next;

        packet->next = NULL;

        if (vnet->free_list == NULL){
            vnet->free_list = packet;
        } else {
            vnet->last_free->next = packet;
        }
        vnet->last_free = packet;
        //printk("DQ: %p is -2\n", packet);
        packet->status = -2;

      //  up(&vnet->sem);
}
static void queue_cmd_packet (struct ig_net *vnet, struct client_cmd_packet *packet){
	if (packet) {
		struct control_block *control = vnet->control;
		
		// setup packet to not-ready
        //printk("EQ: %p is PACKET_SETUP\n", packet);
		packet->status = PACKET_SETUP;

//        down(&vnet->sem);

		// add packet to pending queue

		if (control->tx_pend_head) {
			packet->next = NULL;
            packet->prev = control->tx_pend_last;
			control->tx_pend_last->next = packet;
            control->tx_pend_last = packet;
		} else {
			packet->next = NULL;
			packet->prev = NULL;
            control->tx_pend_head = packet;
            control->tx_pend_last = packet;
		}

    // up(&vnet->sem);
	}
}


static inline uintptr_t
vaddr_to_memsect(struct ig_net *ig_net, void *vaddr)
{
    return ((uintptr_t) vaddr - ig_net->cmd_base) << 4;
}

static inline void *
memsect_to_vaddr(struct ig_net *ig_net, uintptr_t memsect)
{
	return ((void *) ig_net->cmd_base + (memsect >> 4));
}


static int
ig_net_open(struct net_device *dev)
{
	printk("Opening network device\n");
	return 0;
}

static int
ig_net_stop(struct net_device *dev)
{
	printk("Stopping network device\n");
	return 0;
}

static void net_timeout(struct net_device *dev)
{
	struct ig_net *ig_net = netdev_priv(dev);
	struct client_cmd_packet *packet;
    struct control_block *control = ig_net->control;

    // this happens sometimes when the head is stick in 'sending' and the rest pend
    // up behind it.
    // Most likely indicative of a bug somewhere, but for now we can remove the offender
    // and let the rest of them out.

    packet = control->tx_pend_head;

    if (packet && packet->status == PACKET_SENDING)
    {
        printk("\n\t\tTHAT FUNNY STATE HAPPENED?!? ");
        packet->status = 0;
        dev_kfree_skb_irq( (struct sk_buff *)packet->args[0]);
        packet->args[0] =0; // just in case it does someday make it out
        dequeue_cmd_packet(ig_net, packet);
        netif_stop_queue(dev);
        printk("but recovered\n");
        return;
    }

    printk("freelist:\n");
    for (packet = ig_net->free_list;
         packet;
         packet = packet->next){
        printk("%p(%d)->", packet, packet->status);
    }
    printk("NULL\n");

    printk("rx_pend:\n");
    for (packet = control->rx_pend_head;
         packet;
         packet = packet->next){
        printk("%p(%d)->", packet, packet->status);
    }
    printk("NULL\n");

    printk("tx_pend:\n");
    for (packet = control->tx_pend_head;
         packet;
         packet = packet->next){
        printk("%p(%d)->", packet, packet->status);
    }
    printk("NULL\n");


	printk("net timeout\n");
    if (ig_net->stopped){
        printk("is stopped, restartng\n");
        netif_wake_queue(dev);
    } else {
        printk("attempting a kick in the pants\n");
        L4_Notify(ig_net->server, 0x1);
    }
}


static int iguana_net_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    return 0;
}

static int ig_net_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct ig_net *ig_net = netdev_priv(dev);


    if (1){
        struct client_cmd_packet *packet;
        unsigned long flags;

        local_irq_save(flags);
        packet = alloc_cmd_packet(ig_net);
        if (!packet){
            local_irq_restore(flags);
            netif_stop_queue(dev);
            ig_net->stopped = 1;
            return 1;
        }
        queue_cmd_packet(ig_net, packet);
        local_irq_restore(flags);

        packet->cmd = NET_CMD_TX;
        packet->args[0] = (unsigned long)skb;
        packet->args[1] = 0;
        packet->args[2] = 0;

        packet->length = skb->len;
        packet->data   = skb->data;


        //printk("TX: Alloced %p\n", packet);
        //printk("TX:  %p PACKET_PEND\n", packet);
        packet->status = PACKET_PEND;

        L4_Notify(ig_net->server, 1);
    }

    return 0;
}

/* The typical workload of the driver:
   Handle the network interface interrupts. */
   
static irqreturn_t 
ig_net_interrupt(int irq, void *dev_id)
{

	struct net_device *dev = dev_id;
	struct ig_net *ig_net = netdev_priv(dev);
    struct client_cmd_packet *packet, *next;

    for (packet = ig_net->control->tx_pend_head ;
         packet && packet->status == PACKET_DONE;
         packet = next){

        next = packet->next;
        packet->status = 0;
        if (packet->args[0] == 0){
            printk("should never get here \n");
            while (1);
        }
        dev_kfree_skb_irq( (struct sk_buff *)packet->args[0]);
        //printk("TX: Done %p\n", packet);
        dequeue_cmd_packet(ig_net, packet);
        if (ig_net->stopped){
            netif_wake_queue(dev);
            ig_net->stopped = 0;
        }
    }

    for (packet = ig_net->control->rx_pend_head ;
         packet && packet->status == PACKET_FULL;
         packet = next){

        struct sk_buff *skb;
        struct sk_buff *new_skb = dev_alloc_skb(1536+2);
        next = packet->next;
        skb_reserve(new_skb, 2);

        skb = (struct sk_buff*)packet->args[0];
        skb->len = packet->length;
        packet->length = 1536;

        packet->args[0] = (unsigned long)new_skb;
        packet->data = new_skb->data;

        skb->protocol = eth_type_trans(skb,dev);
        skb->ip_summed = CHECKSUM_UNNECESSARY;

        skb->dev = dev;
        if (netif_rx(skb) == NET_RX_DROP){
            printk("SKB RX dropped\n");
            ig_net->stats.rx_errors++;
            ig_net->stats.rx_dropped++;
        }
        dev->last_rx = jiffies;

        ig_net->control->rx_pend_last->next = packet;
        ig_net->control->rx_pend_last = packet;
        ig_net->control->rx_pend_head = packet->next;
        packet->status = -1;

        packet->next = NULL;
        packet->status = PACKET_EMPTY;
    }
    return IRQ_HANDLED;
}

static struct net_device_stats *
net_get_stats(struct net_device *dev)
{
	struct ig_net *ig_net = netdev_priv(dev);
	return &ig_net->stats;
}
static void set_multicast_list(struct net_device *dev)
{
    struct dev_mc_list *mc_ptr;

    if (dev->flags & IFF_PROMISC){
        printk("Looking for Promiscious mode\n");
        return;
    }
    if (dev->flags & IFF_ALLMULTI){
        printk("Want All multicast \n");
    }

    for (mc_ptr = dev->mc_list; mc_ptr; mc_ptr = mc_ptr->next){
#if 0
        printk("Want multi: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mc_ptr->dmi_addr[0],
                mc_ptr->dmi_addr[1],
                mc_ptr->dmi_addr[2],
                mc_ptr->dmi_addr[3],
                mc_ptr->dmi_addr[4],
                mc_ptr->dmi_addr[5]);
#endif
    }
}
static int set_mac_address(struct net_device *dev, void *p)
{
	int i;
	struct sockaddr *addr = p;
    printk("set_mac_address\n");

	if (netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	if (1) {
		printk("%s: Setting MAC address to ", dev->name);
		for (i = 0; i < dev->addr_len; i++)
			printk(" %2.2x", dev->dev_addr[i]);
		printk(".\n");
	}
	return 0;
}




static int
ig_net_init(struct net_device *dev)
{
	struct ig_net* ig_net = netdev_priv(dev);
    uint32_t mac1, mac2, mac3;

	dev->open = ig_net_open;
	dev->stop = ig_net_stop;
	dev->hard_start_xmit = ig_net_hard_start_xmit;
    dev->flags &= ~IFF_MULTICAST;
//    dev->features |= NETIF_F_NO_CSUM;

	dev->tx_timeout		= net_timeout;
	dev->watchdog_timeo	= HZ;
	dev->get_stats		= net_get_stats;
	dev->set_multicast_list = set_multicast_list;
	dev->set_mac_address 	= set_mac_address;
    dev->do_ioctl       = iguana_net_ioctl;

    virtual_net_read_mac_address(ig_net->server, ig_net->dev, 
            &mac1, &mac2, &mac3, NULL);

    dev->dev_addr[0] = (mac1 >> 0)  & 0xff;
    dev->dev_addr[1] = (mac1 >> 8)  & 0xff;

    dev->dev_addr[2] = (mac2 >> 0)  & 0xff;
    dev->dev_addr[3] = (mac2 >> 8)  & 0xff;

    dev->dev_addr[4] = (mac3 >> 0)  & 0xff;
    dev->dev_addr[5] = (mac3 >> 8)  & 0xff;

    ig_net->debug = 0;


    return 0;
}
/* The inverse routine to net_open(). */
extern L4_ThreadId_t timer_thread;

static int __init ig_net_init_module(void)
{
    thread_ref_t server_;
    L4_ThreadId_t server;
    CORBA_Environment env;

    int i = 0;
    struct client_cmd_packet *packet, *prev_packet = NULL;

	struct net_device *dev = alloc_etherdev(sizeof(struct ig_net));
	struct ig_net* ig_net = netdev_priv(dev);
    memset(ig_net, 0, sizeof(struct ig_net));

    ig_net->control_ms = memsection_create(4096, &(ig_net->cmd_base));
    ig_net->stopped = 0;

    memsection_lookup(env_memsection_base(iguana_getenv("OKL4_CORE_DEVICE_SERVER")),
                      &server_);
    server = thread_l4tid(server_);

    /* Allocate an IRQ number */
    dev->irq = iguana_alloc_irq();
    printk("IG_NET on IRQ %d\n", dev->irq);
    request_irq(dev->irq, ig_net_interrupt, 0, dev->name, dev);

    /* Attach to the MTD server */
    ig_net->dev = device_core_get_net(server, &(ig_net->server), &timer_thread, 
            IGUANA_IRQ_NOTIFY_MASK(dev->irq), &env);



    ig_net->control = (struct control_block*)ig_net->cmd_base;
    ig_net->control->tx_pend_head = NULL;
    ig_net->control->tx_pend_head = NULL;
    ig_net->control->status = 0x4299;

    virtual_net_add_memsection(ig_net->server, ig_net->dev, ig_net->control_ms, 0, &env);

    ig_net->free_list =
        (struct client_cmd_packet *)(ig_net->cmd_base +
                                 sizeof(struct control_block));

    for (packet = ig_net->free_list;
         (packet+sizeof(struct client_cmd_packet)) < (struct client_cmd_packet *)(ig_net->cmd_base + 4096); packet++) {
        ig_net->free_list[i].next =  & ig_net->free_list[i + 1];
        ig_net->available++;
        i++;
    }
    ig_net->free_list[i - 1].next = 0;
    ig_net->last_free = & ig_net->free_list[i - 1];

    /* Now reserve some to give to the driver for it's RX pool */
    for (i = 0; i < 10; i++) {
        struct sk_buff *skb;

        packet = alloc_cmd_packet(ig_net);
        packet->prev = prev_packet;
        if (prev_packet)
            prev_packet->next = packet;

        if (packet == NULL) {
            panic("Iguana net driver corrupted\n");
        }

        skb = dev_alloc_skb(1536+2);
        skb_reserve(skb, 2);
        skb->dev = dev;

        packet->length = 1536;
        packet->status = PACKET_EMPTY;
        packet->next = NULL;
        packet->data= skb->data;
        packet->args[0] = (unsigned long)skb;
        ig_net->available--;


        if (ig_net->control->rx_pend_head == NULL){
            ig_net->control->rx_pend_head = packet;
            ig_net->control->rx_pend_last = packet;
            packet->prev = NULL;
        }
        else{
            packet->prev = ig_net->control->rx_pend_last;
            ig_net->control->rx_pend_last = packet;
        }
        prev_packet = packet;
    }
    prev_packet->next = NULL;
    virtual_net_register_control_block(ig_net->server, ig_net->dev, 0, &env);

	ether_setup(dev);
	sprintf(dev->name, "eth%d", 0);
	dev->init = ig_net_init;
	register_netdev(dev);
	return 0;
}

static void __exit ig_net_exit(void)
{
	//input_unregister_device(ig_input->input_dev);
}

module_init(ig_net_init_module);
module_exit(ig_net_exit);
