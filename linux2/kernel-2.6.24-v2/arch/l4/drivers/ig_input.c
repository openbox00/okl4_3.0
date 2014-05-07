#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/input.h>

#include <interfaces/devicecore_client.h>
#include <interfaces/vkpp_client.h>
#include <iguana/thread.h>
#include <iguana/hardware.h>
#include <iguana/object.h>
#include <iguana/env.h>
#include <iguana/memsection.h>

#define DRIVER_VERSION "v1.0"
#define DRIVER_AUTHOR "Open Kernel Labs Inc."
#define DRIVER_DESC "OKL4 virtual input driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define BUFFER_SIZE 0x2000
#define PACKET_SIZE 16

#define TERMINATED 1
#define COMPLETED 2

extern L4_ThreadId_t timer_thread;
extern int iguana_alloc_irq(void);

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
	volatile uintptr_t rx;
};


struct ig_input {
	struct input_dev *input_dev; /**< The linux tty structure */
	L4_ThreadId_t server;   /**< The Iguana serial server */
	objref_t dev;           /**< The Iguana serial device */
	uintptr_t memsect_base; /**< The base of the shared memory section */
	/** Point to the control block */
	struct control_block *control;
	/** Free list of packets */
	struct stream_packet *free_list;
	struct stream_packet *rx_last;
};

/**
 * Convert a virtual address into a memory section reference for
 * sharing pointers with the serial server.
 */
static inline uintptr_t
vaddr_to_memsect(struct ig_input *ig_input, void *vaddr)
{
	return ((uintptr_t) vaddr - ig_input->memsect_base) << 4;
}

/**
 * Convert a memory section reference back into a virtual address.
 */
static inline void *
memsect_to_vaddr(struct ig_input *ig_input, uintptr_t memsect)
{
	return ((void *) ig_input->memsect_base + (memsect >> 4));
}

struct ig_input *ig_input;

static irqreturn_t 
ig_input_interrupt(int irq, void *dev_id)
{
	struct stream_packet *packet;
	uintptr_t next;
	int i;
	int data = 0;
	int kick = 0;

//	packet = ig_input->rx_complete;
        packet = memsect_to_vaddr(ig_input, ig_input->control->rx);	
	while(packet) {
		int new_q = 0;
		uint16_t *keys;
		next = packet->next;
		if ((packet->status & COMPLETED) == 0) {
			break;
		}
		packet->status = 0;
                ig_input->control->rx = next;


		/* Append to free list */
		data = 1;
		keys = (uint16_t*)packet->data;
		for (i=0; i < (packet->xferred/2); i++) {
			if (keys[i] & 0x4000) {
				//printk("Mouse %x, %d\n", keys[i] & 0xff, (keys[i] & 0x100) ? -1 : 1);
				if (keys[i] & 0x8000) {
					input_report_rel(ig_input->input_dev, keys[i] & 0x0ff,
							(keys[i] & 0x100) ? -1 : 1);
				}
			} else {
				//printk("Key %s: %x\n", (keys[i] & 0x8000) ? "down" : "up", keys[i] & 0x01ff);
				input_report_key(ig_input->input_dev, keys[i] & 0x01ff, ((keys[i] & 0x8000) != 0));
			}
		}
		
		packet->next = ~0;
		packet->size = PACKET_SIZE;
		packet->xferred = 0;
		
		if (ig_input->rx_last != NULL) {
			ig_input->rx_last->next = vaddr_to_memsect(ig_input, packet);
			if (ig_input->rx_last->status & TERMINATED) {
				new_q = 1;
			}
		} else {
			new_q = 1;
		}
		if (new_q) {
			ig_input->control->rx = vaddr_to_memsect(ig_input, packet);
			kick = 1;
		}
		ig_input->rx_last = packet;

		if (next == ~0) {
			packet = NULL;
		} else {
			packet = memsect_to_vaddr(ig_input, next);
		}
	}

	if (data) {
		input_sync(ig_input->input_dev);
	}

	if (kick) {
		L4_Notify(ig_input->server, 0x1);
	}

    return IRQ_HANDLED;
}

static int __init ig_input_init(void)
{
	/* Called when we are initialising the device */
	void *buffer_area, *control_area, *input_dev;
	thread_ref_t server_;
	L4_ThreadId_t server;
	thread_ref_t dummy;
	CORBA_Environment env;
	memsection_ref_t memsect;
	uintptr_t base;
	int i, irq;
	
	/* Allocate space for the input structure */
	ig_input = kmalloc(sizeof(*ig_input), GFP_KERNEL);
	input_dev = kmalloc(sizeof(struct input_dev), GFP_KERNEL);
	buffer_area = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	control_area = kmalloc(sizeof(struct control_block), GFP_KERNEL);
	if (!ig_input || !buffer_area || !control_area || !input_dev) {
		kfree(ig_input);
		kfree(buffer_area);
		kfree(control_area);
		kfree(input_dev);
		return -ENOMEM;
	}
	memset(ig_input, 0, sizeof(*ig_input));
	memset(input_dev, 0, sizeof(struct input_dev));
	memset(buffer_area, 0, BUFFER_SIZE);
	memset(control_area, 0, sizeof(struct control_block));

	irq = iguana_alloc_irq();
	printk("IG_KPP got IRQ %d\n", irq);

	/* First, we find the input device */
	memsection_lookup(env_memsection_base(iguana_getenv("OKL4_CORE_DEVICE_SERVER")),
                          &server_);
	server = thread_l4tid(server_);
	ig_input->dev = device_core_get_kpp(server, &ig_input->server, 
            &timer_thread, IGUANA_IRQ_NOTIFY_MASK(irq), &env);

	/* Now, we add the kernel memory section to the server */
	memsect = memsection_lookup((objref_t) buffer_area, &dummy);
	ig_input->memsect_base = base = (uintptr_t) memsection_base(memsect);
	virtual_kpp_add_memsection(ig_input->server, ig_input->dev, memsect, 0, &env);
	
	/* Initialise the control block */
	ig_input->control = control_area;
	ig_input->control->rx = ~0;
	
	ig_input->rx_last = NULL;

	ig_input->free_list = buffer_area;

	for (i=0; i < BUFFER_SIZE / sizeof(struct stream_packet); i++) {
		ig_input->free_list[i].next = (uintptr_t) &ig_input->free_list[i+1];
		ig_input->free_list[i].data_ptr = &ig_input->free_list[i].data;
                    //vaddr_to_memsect(ig_input, &ig_input->free_list[i].data);
	}
	ig_input->free_list[i-1].next = 0;

	for (i = 0; i < BUFFER_SIZE / sizeof(struct stream_packet); i++) {
		struct stream_packet *packet;
		int new_q = 0;
		packet = ig_input->free_list;
		if (packet == NULL) {
			panic("Iguana input driver corrupted\n");
		}

		ig_input->free_list = (void*) packet->next;
		
		packet->next = ~0;
		packet->size = PACKET_SIZE;
		packet->xferred = 0;
		packet->status = 0;
		
		if (ig_input->rx_last != NULL) {
			ig_input->rx_last->next = vaddr_to_memsect(ig_input, packet);
			if (ig_input->rx_last->status & TERMINATED) {
				new_q = 1;
			}
		} else {
			new_q = 1;
		}
		if (new_q) {
			ig_input->control->rx = vaddr_to_memsect(ig_input, packet);
		}
		ig_input->rx_last = packet;
	}
	/* Register the control block with the server */
	virtual_kpp_register_control_block(ig_input->server,
                ig_input->dev, 
                vaddr_to_memsect(ig_input, ig_input->control), &env);


	ig_input->input_dev = input_dev;
	ig_input->input_dev->evbit[0] = BIT(EV_KEY) | BIT(EV_REL);
	set_bit(KEY_0, ig_input->input_dev->keybit);
	set_bit(KEY_1, ig_input->input_dev->keybit);
	set_bit(KEY_2, ig_input->input_dev->keybit);
	set_bit(KEY_3, ig_input->input_dev->keybit);
	set_bit(KEY_4, ig_input->input_dev->keybit);
	set_bit(KEY_5, ig_input->input_dev->keybit);
	set_bit(KEY_6, ig_input->input_dev->keybit);
	set_bit(KEY_7, ig_input->input_dev->keybit);
	set_bit(KEY_8, ig_input->input_dev->keybit);
	set_bit(KEY_9, ig_input->input_dev->keybit);

	set_bit(KEY_KPASTERISK, ig_input->input_dev->keybit);
	set_bit(KEY_CHAT, ig_input->input_dev->keybit);
	set_bit(KEY_F17, ig_input->input_dev->keybit);
	set_bit(KEY_F18, ig_input->input_dev->keybit);
	set_bit(KEY_F19, ig_input->input_dev->keybit);
	set_bit(KEY_F20, ig_input->input_dev->keybit);
	set_bit(KEY_F21, ig_input->input_dev->keybit);
	set_bit(KEY_END, ig_input->input_dev->keybit);
	set_bit(/*KEY_F22*/KEY_ENTER, ig_input->input_dev->keybit);
	set_bit(KEY_F23, ig_input->input_dev->keybit);
	set_bit(KEY_BACK, ig_input->input_dev->keybit);
	set_bit(KEY_UP, ig_input->input_dev->keybit);
	set_bit(KEY_DOWN, ig_input->input_dev->keybit);
	set_bit(KEY_LEFT, ig_input->input_dev->keybit);
	set_bit(KEY_RIGHT, ig_input->input_dev->keybit);
	set_bit(BTN_LEFT, ig_input->input_dev->keybit);
	set_bit(BTN_RIGHT, ig_input->input_dev->keybit);
	set_bit(REL_X, ig_input->input_dev->relbit);
	set_bit(REL_Y, ig_input->input_dev->relbit);

	//set_bit(KEY_CONNECT, ig_input->input_dev->keybit);
	//set_bit(KEY_FINANCE, ig_input->input_dev->keybit);

	set_bit(/*KEY_F13*/KEY_F1, ig_input->input_dev->keybit);
	set_bit(/*KEY_F14*/KEY_F2, ig_input->input_dev->keybit);
	set_bit(/*KEY_F15*/KEY_F3, ig_input->input_dev->keybit);
	set_bit(/*KEY_F16*/KEY_F4, ig_input->input_dev->keybit);

	set_bit(KEY_HOME, ig_input->input_dev->keybit);
	set_bit(KEY_POWER, ig_input->input_dev->keybit);

	set_bit(KEY_VOLUMEDOWN, ig_input->input_dev->keybit);
	set_bit(KEY_VOLUMEUP, ig_input->input_dev->keybit);
	set_bit(KEY_RECORD, ig_input->input_dev->keybit);
	ig_input->input_dev->name = "Iguana virtual input device";

	request_irq(irq, ig_input_interrupt, 0, 
            ig_input->input_dev->name, ig_input->input_dev);

	input_register_device(ig_input->input_dev);
	return 0;
}

static void __exit ig_input_exit(void)
{
	input_unregister_device(ig_input->input_dev);
}

module_init(ig_input_init);
module_exit(ig_input_exit);
