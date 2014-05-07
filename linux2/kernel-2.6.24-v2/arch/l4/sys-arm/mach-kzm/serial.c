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

#include <asm/io.h>

#if 1
/* XXX ripped from l4 kernel -gl */
struct serial_imx31 {
    word_t rxr;
    word_t pad1[15];
    word_t txr;
    word_t pad2[15];
    word_t cr1;
    word_t cr2;
    word_t cr3;
    word_t cr4;
    word_t fifocr;
    word_t sr1;
    word_t sr2;
    /* there are more but they are not needed here */
};

#define SR1_TRDY        0x2000    /* Xmitter ready */
#define SR1_RRDY        0x200     /* receiver ready */

static volatile struct serial_imx31 * serial_regs = NULL;

#include <okl4/env.h>
#include <okl4/types.h>
#endif

#define DRIVER_VERSION "v1.0"
#define DRIVER_AUTHOR "Open Kernel Labs Inc."
#define DRIVER_DESC "Iguana virtual serial driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define IG_SERIAL_MINORS 1

struct ig_serial;
static irqreturn_t ig_serial_interrupt(int irq, void *dev_id);

/* Iguana virtual tty driver */

struct ig_serial {
	struct tty_struct *tty; /**< The linux tty structure */
	struct semaphore sem;   /**< Semaphore to ensure consistency */
	int open_count;         /**< Number of times opened */\
	int available;
};

static struct ig_serial *ig_serial_table[IG_SERIAL_MINORS];

extern int iguana_alloc_irq(void);

#define DEBUG(s)		{} // printk("ig_serial [%s] :" s "\n", __FUNCTION__)

static int __init
kzm_cons_setup(struct console *co, char *options)
{
	word_t seg;
	okl4_env_segments_t *segs;
	int i;

	seg = *(word_t *)okl4_env_get("MAIN_SERIAL_MEM0");
	segs = OKL4_ENV_GET_SEGMENTS("SEGMENTS");
	for (i = 0; i < segs->segments; i++) {
		if (segs->segments[i].segment == seg) {
			serial_regs = (void *)segs->segments[i].virt_addr;
			break;
		}
	}
	return 0;
}

void
kzm_serial_putchar(const char c)
{
	/* Locking? */
	if (serial_regs) {
		while ((serial_regs->sr1 & SR1_TRDY) == 0);
			serial_regs->txr = c;
		if (c == '\n')
			kzm_serial_putchar('\r');
	}
}

static int
ig_serial_open(struct tty_struct *tty, struct file *file)
{
	struct ig_serial *ig_serial;
	int index;
	int irq;

	DEBUG("start");

	tty->driver_data = NULL;

	if (tty->index > 0) {
		return -ENODEV;
	}

	index = tty->index;
	ig_serial = ig_serial_table[index];
	if (ig_serial == NULL) {
		ig_serial = kmalloc(sizeof(*ig_serial), GFP_KERNEL);
		if (!ig_serial)
			return -ENOMEM;
		memset(ig_serial, 0, sizeof(*ig_serial));

		/* Initialize the lock */
		init_MUTEX(&ig_serial->sem);
		ig_serial->open_count = 0;
		ig_serial_table[index] = ig_serial;
		ig_serial->available = 16;	/* XXX */
	}

	down(&ig_serial->sem);

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

static irqreturn_t 
ig_serial_interrupt(int irq, void *dev_id)
{
	struct ig_serial *ig_serial = ig_serial_table[0];
	int data;

	if (serial_regs) {
		serial_regs->cr1 &= ~(1 << 9);	/* RRDY disable */
		data = serial_regs->rxr & 0xff;
		serial_regs->cr1 |= (1 << 9);	/* RRDY enable */
		if (data == 0xb)	/* ctrl - k */
			L4_KDB_Enter("breakin");
		if (ig_serial) {
			tty_insert_flip_char(ig_serial->tty, data, TTY_NORMAL);
			tty_flip_buffer_push(ig_serial->tty);
		}
	} else {
		printk("%s: spurious\n", __func__);
	}

	return IRQ_HANDLED;
#if 0
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
#endif

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

	if (!ig_serial) {
		return -ENODEV;
	}

	down(&ig_serial->sem);

	if (ig_serial->open_count == 0) {
		xmitted = -EINVAL;
		goto exit;
	}

	while (xmitted < count) {
		if (serial_regs) 
			kzm_serial_putchar(buffer[xmitted]);
		xmitted++;
	}
exit:
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

extern L4_ThreadId_t timer_thread;

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

	request_irq(45, ig_serial_interrupt, 0, "igserial", ig_serial_driver);
	/* enable the device */
	serial_regs->cr1 |= (1 << 9);	/* RRDY enable */

	DEBUG("end");

	return retval;
}

void __exit ig_serial_exit(void)
{
	tty_unregister_driver(ig_serial_driver);
}

module_init(ig_serial_init);
module_exit(ig_serial_exit);

static void
kzm_cons_write(struct console *console, const char *p, unsigned count)
{
	while (count-- > 0)
		kzm_serial_putchar(*p++);
}

struct tty_driver *
kzm_cons_device(struct console *co, int *index)
{
	*index = 0;
	return ig_serial_driver;
}
	
static struct console kzm_cons = {
	.name	= "ttyS",
	.write	= kzm_cons_write,
	.flags	= CON_PRINTBUFFER,
	.device = kzm_cons_device,
	.setup = kzm_cons_setup,
	.index = -1
};

static int __init
kzm_console_init(void)
{
	register_console(&kzm_cons);
}

console_initcall(kzm_console_init);

