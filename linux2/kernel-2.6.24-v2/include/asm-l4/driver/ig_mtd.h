/* 
 * Linux driver for Iguana virtual MTD devices
 *
 * Copyright (C) 2007 Open Kernel Labs, Inc.
 *
 * Author: Carl van Schaik
 *
 * Released under GPL
 */

#ifndef __MTD_IG_H__
#define __MTD_IG_H__

#include <linux/mtd/mtd.h>
#include <asm/semaphore.h>
#include <mutex/mutex.h>

struct ig_mtd_info {
	uint32_t blocks;
	uint32_t block_size;
	uint32_t erase_size;
	uint32_t page_size;
	uint32_t oob_size;
};

struct control_block {
	unsigned long status;

	struct ig_mtd_info info;

	struct mutex queue_lock;

	/* list of pending packets (doubly linked) */
	struct client_cmd_packet *pend_list;
	/* list of completed packets (singley linked) */
	struct client_cmd_packet *done_list;
};

struct ig_mtd {
	objref_t dev;
	L4_ThreadId_t server;
	memsection_ref_t control_ms;
	unsigned long cmd_base;
	struct control_block *control;
	int irq;

	/* list of available packets (singley linked) */
	struct client_cmd_packet *free_list;

	struct mtd_info *next;

	wait_queue_head_t wq; /* Wait on here when we're waiting for the mtd */
};

#endif /* __MTD_IG_H__ */
