/*
 *  Copyright (c) 2007 Open Kernel Labs
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>

/* 
 * XXX hack 
 *
 * OKL4 device driver v2 has a function called device_create(),
 * but so does Linux!
 */
#define device_create okl4_device_create

#include <interfaces/devicecore_client.h>
#include <interfaces/vtouch_client.h>
#include <iguana/thread.h>
#include <iguana/hardware.h>
#include <iguana/object.h>
#include <iguana/memsection.h>
#include <iguana/env.h>

#define DRIVER_DESC     "Iguana virtual touchscreen driver"

MODULE_AUTHOR("Carl van Schaik <carl@ok-labs.com>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define BUFFER_SIZE (0x1000-1)
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

struct control_block {
	volatile uintptr_t rx;
};

struct ig_touch {
	struct input_dev *input;
	L4_ThreadId_t server;   /* The Iguana serial server */
	objref_t dev;           /* The Iguana serial device */

	memsection_ref_t control_ms;
	uintptr_t control_base;
	int irq;

	/** Point to the control block */
	struct control_block *control;
	/** Free list of packets */
	struct stream_packet *free_list;
	struct stream_packet *rx_complete;
	struct stream_packet *rx_last;
};

struct press_info {
	int x;
	int y;
	int pressure;
};

/**
 * Convert a virtual address into a memory section reference for
 * sharing pointers with the serial server.
 */
static inline uintptr_t
vaddr_to_memsect(struct ig_touch *touch, void *vaddr)
{
	return ((uintptr_t)vaddr - touch->control_base) << 4;
}

/**
 * Convert a memory section reference back into a virtual address.
 */
static inline void *
memsect_to_vaddr(struct ig_touch *touch, uintptr_t memsect)
{
	return ((void *)touch->control_base + (memsect >> 4));
}

static irqreturn_t
ig_touch_interrupt(int irq, void *dev_id)
{
	struct ig_touch *touch = (struct ig_touch*)dev_id;
	struct stream_packet *packet;
	uintptr_t next;
	int data = 0;
	int kick = 0;

	packet = touch->rx_complete;
	while (packet) {
		struct press_info *pi = (struct press_info*)packet->data;
		int new_q = 0;

		next = packet->next;
		if ((packet->status & COMPLETED) == 0) {
			break;
		}

		data = 1;
		if (pi->x) {
			input_report_abs(touch->input, ABS_X, pi->x);
			input_report_abs(touch->input, ABS_Y, pi->y);
			input_report_key(touch->input, BTN_TOUCH, 1);
			//input_report_abs(&touch->input, ABS_PRESSURE, 0xffff);
		} else {
			input_report_key(touch->input, BTN_TOUCH, 0);
			//input_report_abs(&touch->input, ABS_PRESSURE, 0);
		}

		packet->next = ~0;
		packet->size = PACKET_SIZE;
		packet->xferred = 0;
		packet->status = 0;
		
		if (touch->rx_last != NULL) {
			touch->rx_last->next = vaddr_to_memsect(touch, packet);
			if (touch->rx_last->status & TERMINATED) {
				new_q = 1;
			}
		} else {
			new_q = 1;
		}
		if (new_q) {
			touch->control->rx = vaddr_to_memsect(touch, packet);
			kick = 1;
		}
		touch->rx_last = packet;

		if (next == ~0) {
			packet = NULL;
		} else {
			packet = memsect_to_vaddr(touch, next);
		}
	}

	if (data) {
		input_sync(touch->input);
	}

	if (kick) {
		L4_Notify(touch->server, 0x1);
	}

	touch->rx_complete = packet;

	return IRQ_HANDLED;
}

// XXX put in header
extern int iguana_alloc_irq(void);
extern L4_ThreadId_t timer_thread;

static int __init touch_init(void)
{
	struct ig_touch *touch;
	thread_ref_t server_;
	L4_ThreadId_t server;
	CORBA_Environment env;
	int i;

	touch = kmalloc(sizeof(struct ig_touch), GFP_KERNEL);
	if (!touch) {
		kfree(touch);
		return -ENOMEM;
	}

	memset(touch, 0, sizeof(struct ig_touch));

	touch->irq = iguana_alloc_irq();

	/* First, we find the input device */
	memsection_lookup(env_memsection_base(iguana_getenv("OKL4_CORE_DEVICE_SERVER")),
                          &server_);
	server = thread_l4tid(server_);
	touch->dev = device_core_get_vdevice(server, &touch->server,
		&timer_thread, IGUANA_IRQ_NOTIFY_MASK(touch->irq), "touchscreen",  &env);

	/* Now, we add a memory section to the server */
	touch->control_ms = memsection_create(4096, &(touch->control_base));
	virtual_touch_add_memsection(touch->server, touch->dev, touch->control_ms, 0,0, &env);

	/* Initialise the control block */
	touch->control = (void*)touch->control_base;
	touch->control->rx = ~0;

	/* Register the control block with the server */
	virtual_touch_register_control_block(touch->server,
			touch->dev,
			vaddr_to_memsect(touch, touch->control), &env);

	touch->rx_complete = NULL;
	touch->rx_last = NULL;

	touch->free_list = (void*)(touch->control_base+ sizeof(struct control_block));

	for (i=0; i < BUFFER_SIZE / sizeof(struct stream_packet); i++) {
		touch->free_list[i].next = (uintptr_t) &touch->free_list[i+1];
		touch->free_list[i].data_ptr = vaddr_to_memsect(touch, &touch->free_list[i].data);
	}
	touch->free_list[i-1].next = 0;

	for (i = 0; i < BUFFER_SIZE / sizeof(struct stream_packet); i++) {
		struct stream_packet *packet;
		int new_q = 0;
		packet = touch->free_list;
		if (packet == NULL) {
			panic("Iguana touch driver corrupted\n");
		}

		if (touch->rx_complete == NULL) {
			touch->rx_complete = packet;
		}
		
		touch->free_list = (void*) packet->next;
		
		packet->next = ~0;
		packet->size = PACKET_SIZE;
		packet->xferred = 0;
		packet->status = 0;
		
		if (touch->rx_last != NULL) {
			touch->rx_last->next = vaddr_to_memsect(touch, packet);
			if (touch->rx_last->status & TERMINATED) {
				new_q = 1;
			}
		} else {
			new_q = 1;
		}
		if (new_q) {
			touch->control->rx = vaddr_to_memsect(touch, packet);
		}
		touch->rx_last = packet;
	}
	L4_Notify(touch->server, 0x1);

	/* register and setup input driver */
	touch->input = input_allocate_device();
	/*
	 * XXX How to undo Iguana initalization?
	 */
	if (!touch->input) 
		return -ENOMEM;
	touch->input->evbit[0] = BIT(EV_KEY) | BIT(EV_ABS);
	touch->input->keybit[LONG(BTN_TOUCH)] = BIT(BTN_TOUCH);

	input_set_abs_params(touch->input, ABS_X, 250, 3900, 0, 0);
	input_set_abs_params(touch->input, ABS_Y, 250, 3900, 0, 0);
	//input_set_abs_params(&touch->input, ABS_PRESSURE, 0, 0xffff, 0, 0);

	touch->input->private = touch;
	touch->input->name = DRIVER_DESC;

	input_register_device(touch->input);

	request_irq(touch->irq, ig_touch_interrupt, 0, touch->input->name, touch);

	return 0;
}

static void __exit touch_cleanup(void)
{
}

MODULE_LICENSE("GPL");

module_init(touch_init);
module_exit(touch_cleanup);

