/*
 * This is the virtual tty driver for Iguana virtual devices.
 *
 * The skeleton implementation was taken from the Linux Device Drivers
 * example file.
 *
 */

#include <linux/init.h>
#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/tty_driver.h>
#include <linux/interrupt.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>

/*
 * XXX namespace clash.  :-(
 */
#define device_create okl4_device_create

#include <interfaces/vserial_client.h>
#include <iguana/thread.h>
#include <iguana/hardware.h>
#include <iguana/object.h>
#include <iguana/memsection.h>
#include <iguana/env.h>

#include <driver/types.h>

#define DRIVER_VERSION "v1.0"
#define DRIVER_AUTHOR "Open Kernel Labs Inc."
#define DRIVER_DESC "Iguana virtual serial driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define IG_SERIAL_MINORS 1

struct ig_serial;
static irqreturn_t ig_serial_interrupt(int irq, void *dev_id);

#define BUFFER_SIZE 0x2000
#define PACKET_SIZE 16

#define TERMINATED 1
#define COMPLETED 2

struct stream_packet {
	uintptr_t next;
	uintptr_t data_ptr;
	size_t size;
	size_t xferred;
	volatile int status;
	char data[PACKET_SIZE];
};

/* Iguana virtual tty driver */
struct control_block {
	volatile uintptr_t tx;
	volatile uintptr_t rx;
};

struct ig_serial {
	struct tty_struct *tty; /**< The linux tty structure */
	struct semaphore sem;   /**< Semaphore to ensure consistency */
	int open_count;         /**< Number of times opened */
	server_t server;        /**< The Iguana serial server */
	device_t dev;           /**< The Iguana serial device */
	uintptr_t memsect_base; /**< The base of the shared memory section */
	/** Point to the control block */
	struct control_block *control;
	/** Free list of packets */
	struct stream_packet *free_list;
	/** List of completed transmit packets */
	struct stream_packet *tx_complete;
	/** The last packet transmitted */
	struct stream_packet *tx_last;

	struct stream_packet *rx_complete;
	struct stream_packet *rx_last;
	int available;
};

/**
 * Convert a virtual address into a memory section reference for
 * sharing pointers with the serial server.
 */
static inline uintptr_t
vaddr_to_memsect(struct ig_serial *ig_serial, void *vaddr)
{
	return ((uintptr_t) vaddr - ig_serial->memsect_base) << 4;
}

/**
 * Convert a memory section reference back into a virtual address.
 */
static inline void *
memsect_to_vaddr(struct ig_serial *ig_serial, uintptr_t memsect)
{
	return ((void *) ig_serial->memsect_base + (memsect >> 4));
}

static struct ig_serial *ig_serial_table[IG_SERIAL_MINORS];

extern L4_ThreadId_t timer_thread;
extern int iguana_alloc_irq(void);

#define DEBUG(s)		{}  //printk("ig_serial [%s] :" s "\n", __FUNCTION__)

static int
ig_serial_open(struct tty_struct *tty, struct file *file)
{
	DEBUG("start");

	struct ig_serial *ig_serial;
	int index;
	int irq;
	CORBA_Environment env;
	L4_Word_t tid;

	tty->driver_data = NULL;

	if (tty->index > 0) {
		return -ENODEV;
	}

	index = tty->index;
	ig_serial = ig_serial_table[index];
	if (ig_serial == NULL) {
		/* Called when we are initialising the device */
		memsection_ref_t memsect;
		int i;

		/* Allocate space for the serial structure */
		ig_serial = kmalloc(sizeof(*ig_serial), GFP_KERNEL);
		if (!ig_serial) {
			kfree(ig_serial);
			return -ENOMEM;
		}
		memset(ig_serial, 0, sizeof(*ig_serial));

		/* Initialise the lock */
		init_MUTEX(&ig_serial->sem);

		ig_serial->open_count = 0;
		ig_serial_table[index] = ig_serial;

		/* Take the lock so that we operate correctly */
		down(&ig_serial->sem);

        irq = iguana_alloc_irq();
		request_irq(irq, ig_serial_interrupt, 0, tty->name, tty);

		/* First, we find the serial device */
		ig_serial->server = env_thread_id(iguana_getenv("VSERIAL_TID"));
		ig_serial->dev    = env_const(iguana_getenv("VSERIAL_HANDLE"));

        /* and retrieve the statically allocated shared memsection */
		memsect = env_memsection(iguana_getenv("VSERIAL_MS"));
		ig_serial->memsect_base = (uintptr_t)memsection_base(memsect);

		/* Initialise the control block */
		ig_serial->control = (struct control_block*)ig_serial->memsect_base;
		ig_serial->control->tx = ~0;
		ig_serial->control->rx = ~0;

		/* Register the control block with the server */
		virtual_serial_register_control_block(ig_serial->server,
                        ig_serial->dev, 
                        vaddr_to_memsect(ig_serial, ig_serial->control), &env);

		/* Initialise the free list */
		ig_serial->tx_complete = NULL;
		ig_serial->tx_last = NULL;

		ig_serial->rx_complete = NULL;
		ig_serial->rx_last = NULL;

		ig_serial->free_list = (struct stream_packet*)(ig_serial->memsect_base + sizeof(struct control_block));

		for (i=0; i < (BUFFER_SIZE-sizeof(struct control_block)) / sizeof(struct stream_packet); i++) {
			ig_serial->free_list[i].next = (uintptr_t) &ig_serial->free_list[i+1];
			ig_serial->free_list[i].data_ptr = vaddr_to_memsect(ig_serial, &ig_serial->free_list[i].data);
			ig_serial->available += PACKET_SIZE;
		}
		ig_serial->free_list[i-1].next = 0;

		for (i = 0; i < 10; i++) {
			struct stream_packet *packet;
			int new_q = 0;
			packet = ig_serial->free_list;
			if (packet == NULL) {
				panic("Iguana serial driver corrupted\n");
			}

			if (ig_serial->rx_complete == NULL) {
				ig_serial->rx_complete = packet;
			}

			ig_serial->free_list = (void*) packet->next;

			packet->next = ~0;
			packet->size = PACKET_SIZE;
			packet->xferred = 0;
			packet->status = 0;
			ig_serial->available -= PACKET_SIZE;

			if (ig_serial->rx_last != NULL) {
				ig_serial->rx_last->next = vaddr_to_memsect(ig_serial, packet);
				barrier();
				if (ig_serial->rx_last->status & TERMINATED) {
					new_q = 1;
				}
			} else {
				new_q = 1;
			}
			if (new_q) {
				ig_serial->control->rx = vaddr_to_memsect(ig_serial, packet);
			}
			ig_serial->rx_last = packet;
		}

		tid = timer_thread.raw;
		virtual_serial_init(ig_serial->server, ig_serial->dev, tid, IGUANA_IRQ_NOTIFY_MASK(irq), &env);

	} else {
		down(&ig_serial->sem);
	}

    L4_Notify(ig_serial->server, 0x1);

	tty->driver_data = ig_serial;
	ig_serial->tty = tty;
	ig_serial->open_count++;

	up(&ig_serial->sem);
	
	DEBUG("end");
	
	return 0;
}

static void do_close(struct ig_serial *ig_serial)
{
	down(&ig_serial->sem);

	if (!ig_serial->open_count) {
		goto exit;
	}

	ig_serial->open_count--;

	if (ig_serial->open_count <= 0) {
		/* Shut down anything. Currently, we don't have to do anything */
	}

exit:
	up(&ig_serial->sem);
}

static void ig_serial_close(struct tty_struct *tty, struct file *file)
{
	struct ig_serial *ig_serial = tty->driver_data;

	if (ig_serial) {
		do_close(ig_serial);
	}
}

static int try_free(struct ig_serial *ig_serial)
{
	int count = 0;
	struct stream_packet *packet;
	uintptr_t next;
	packet = ig_serial->tx_complete;
	while(packet) {
		next = packet->next;
		if ((packet->status & COMPLETED) == 0) {
			break;
		}
		count++;
		/* Append to free list */
		packet->next = (uintptr_t) ig_serial->free_list;
		ig_serial->free_list = packet;
		ig_serial->available += PACKET_SIZE;
		if (ig_serial->tx_last == packet) {
			ig_serial->tx_last = NULL;
		}
		if (next == ~0) {
			packet = NULL;
		} else {
			packet = memsect_to_vaddr(ig_serial, next);
		}

	}

	ig_serial->tx_complete = packet;

	if (count)
		tty_wakeup(ig_serial->tty);

	return count;
}

static irqreturn_t 
ig_serial_interrupt(int irq, void *dev_id)
{
	DEBUG("start");
	
	struct stream_packet *packet;
	uintptr_t next;
	int i;
	int data = 0;
	int kick = 0;
	struct ig_serial *ig_serial = ig_serial_table[0];

	packet = ig_serial->rx_complete;
	while(packet) {
		int new_q = 0;
		next = packet->next;
		if ((packet->status & COMPLETED) == 0) {
			break;
		}
		/* Append to free list */
		data = 1;
		for (i=0; i < packet->xferred; i++) {
#if 0	/* original oklinux code -gl */
			if (ig_serial->tty->flip.count >= TTY_FLIPBUF_SIZE)
				tty_flip_buffer_push(ig_serial->tty);
			tty_insert_flip_char(ig_serial->tty, packet->data[i], TTY_NORMAL);
#else
			if (tty_insert_flip_char(ig_serial->tty, 
			    packet->data[i],
			    TTY_NORMAL))
				tty_flip_buffer_push(ig_serial->tty);
#endif
		}
		
		packet->next = ~0;
		packet->size = PACKET_SIZE;
		packet->xferred = 0;
		packet->status = 0;
		
		if (ig_serial->rx_last != NULL) {
			ig_serial->rx_last->next = vaddr_to_memsect(ig_serial, packet);
			barrier();
			if (ig_serial->rx_last->status & TERMINATED) {
				new_q = 1;
			}
		} else {
			new_q = 1;
		}
		if (new_q) {
			ig_serial->control->rx = vaddr_to_memsect(ig_serial, packet);
			kick = 1;
		}
		ig_serial->rx_last = packet;

		if (next == ~0) {
			packet = NULL;
		} else {
			packet = memsect_to_vaddr(ig_serial, next);
		}
	}
	if (data) {
		tty_flip_buffer_push(ig_serial->tty);
	}

	if (kick) {
		L4_Notify(ig_serial->server, 0x1);
	}

	ig_serial->rx_complete = packet;

	try_free(ig_serial);

	return IRQ_HANDLED;
}


static int ig_serial_write(struct tty_struct *tty,
						   const unsigned char *buffer, int count)
{
	DEBUG("start");
	
	struct ig_serial *ig_serial = tty->driver_data;
	struct stream_packet *packet;
	int xmitted = 0;
	int remaining = count;
	int kick = 0;

	if (!ig_serial) {
		return -ENODEV;
	}

	down(&ig_serial->sem);

	if (ig_serial->open_count == 0) {
		xmitted = -EINVAL;
		goto exit;
	}

	while (xmitted < count) {
		int this_xfer;
		int new_q = 0;
		unsigned long flags;

		local_irq_save(flags);

		packet = ig_serial->free_list;

		if (packet == NULL) {
			local_irq_restore(flags);
			goto exit;
		}

		ig_serial->free_list = (void*) packet->next;
		local_irq_restore(flags);

		if (remaining > PACKET_SIZE) {
			this_xfer = PACKET_SIZE;
		} else {
			this_xfer = remaining;
		}

		memcpy(packet->data, buffer, this_xfer);
		buffer += this_xfer;
		xmitted += this_xfer;
		remaining -= this_xfer;
		ig_serial->available -= PACKET_SIZE;

		/* There is a race here, between checking tx, and setting
		   last. We take care of this later when we are freeing 
		   processed packets */

		packet->next = ~0;
		packet->size = this_xfer;
		packet->status = 0;

		local_irq_save(flags);

		if (ig_serial->tx_complete == NULL) {
			ig_serial->tx_complete = packet;
		}

		/* Append packet to the end of the list, in an atomic way! */
		if (ig_serial->tx_last != NULL) {
			ig_serial->tx_last->next = vaddr_to_memsect(ig_serial, packet);
			barrier();
			if (ig_serial->tx_last->status & TERMINATED) {
				new_q = 1;
			}
		} else {
			new_q = 1;
		}

		if (new_q) {
			kick = 1;
			ig_serial->control->tx = vaddr_to_memsect(ig_serial, packet);
		}
		ig_serial->tx_last = packet;

		local_irq_restore(flags);
	}

exit:
	if (kick) {
		L4_Notify(ig_serial->server, 0x1);
	}
	up(&ig_serial->sem);
	
	return xmitted;
}

static int ig_serial_write_room(struct tty_struct *tty)
{
	struct ig_serial *ig_serial = tty->driver_data;
	int room = -EINVAL;

	if (!ig_serial) {
		return -ENODEV;
	}

	down(&ig_serial->sem);

	if (ig_serial->open_count == 0) {
		goto exit;
	}

	room = ig_serial->available;

	if (room == 0) {
		//printk("No room left???!?!\n");
	}
exit:
	up(&ig_serial->sem);
	
	return room;

}

static int ig_chars_in_buffer(struct tty_struct *tty){
    return 0;
}


static struct tty_operations ig_serial_ops = {
	.open = ig_serial_open,
	.close = ig_serial_close,
	.write = ig_serial_write,
	.write_room = ig_serial_write_room,
	.chars_in_buffer = ig_chars_in_buffer,
};

static struct tty_driver *ig_serial_driver;

int __init ig_serial_init(void)
{
	int retval;
	int i;
	
	DEBUG("start");

	ig_serial_driver = alloc_tty_driver(IG_SERIAL_MINORS);
	if (!ig_serial_driver){
		return -ENOMEM;
        }

	/* intiailise the tty driver */
	ig_serial_driver->owner = THIS_MODULE;
	ig_serial_driver->driver_name = "ig_serial";
	ig_serial_driver->name = "ttyS";
	ig_serial_driver->major = TTY_MAJOR;
	ig_serial_driver->minor_start = 64;
	ig_serial_driver->type = TTY_DRIVER_TYPE_SERIAL;
	ig_serial_driver->subtype = SERIAL_TYPE_NORMAL;
	ig_serial_driver->flags = TTY_DRIVER_REAL_RAW;
	ig_serial_driver->init_termios = tty_std_termios;
	ig_serial_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	tty_set_operations(ig_serial_driver, &ig_serial_ops);

	retval = tty_register_driver(ig_serial_driver);

	if (retval) {
		printk(KERN_ERR "failed to register ig serial driver");
		put_tty_driver(ig_serial_driver);
		return retval;
	}

	/* FIXME */
	for(i = 0; i < IG_SERIAL_MINORS; i++) {
		tty_register_device(ig_serial_driver, i, NULL);
	}
	
	printk(KERN_INFO DRIVER_DESC " " DRIVER_VERSION "\n");

	DEBUG("end");

	return retval;
}

void __exit ig_serial_exit(void)
{
	tty_unregister_driver(ig_serial_driver);
}

static int __init
ig_cons_setup(struct console *co, char *options)
{
	return 0;
}

static void
ig_cons_write(struct console *console, const char *p, unsigned count)
{
	/* should switch to proper driver */
	while (count-- > 0)
		L4_KDB_PrintChar(*p++);
}

struct tty_driver *
ig_cons_device(struct console *co, int *index)
{
	*index = 0;
	return ig_serial_driver;
}

static struct console ig_serial_console = {
	.name	= "ttyS",
	.write	= ig_cons_write,
	.flags	= CON_PRINTBUFFER,
	.device	= ig_cons_device,
	.setup	= ig_cons_setup,
	.index	= -1,
};

static int __init
ig_console_init(void)
{
	register_console(&ig_serial_console);
	return 0;
}

module_init(ig_serial_init);
module_exit(ig_serial_exit);

console_initcall(ig_console_init);
