/*
 * Linux driver for Iguana virtual MTD device
 * (c) 2007 Open Kernel Labs, Inc.
 *
 * Author: Carl van Schaik
 */

#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/irq.h>

#include <linux/mtd/mtd.h>
#include <asm/driver/ig_mtd.h>

#include <interfaces/devicecore_client.h>
#include <interfaces/vmtd_client.h>

#include <iguana/types.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/env.h>
#include <vmtd/mtd.h>

#define IG_MTD_MAX_DEVICES      2

//#define DEBUG_MSG(fmt, args...) printk(KERN_DEBUG fmt, ## args)
//#define DEBUG_MSG(fmt, args...) printk(fmt, ## args)
#define DEBUG_MSG(fmt, args...)
#define ERROR_MSG(fmt, args...) printk(KERN_NOTICE fmt, ## args)

static struct mtd_info *ig_mtdlist = NULL;

static int ig_erase(struct mtd_info *mtd, struct erase_info *instr);
static int ig_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
static int ig_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
static int ig_read_ecc(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf,
		u_char *eccbuf, struct nand_oobinfo *oobsel);
static int ig_write_ecc(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf,
		u_char *eccbuf, struct nand_oobinfo *oobsel);
static int ig_read_oob(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
static int ig_write_oob(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
static int ig_block_isbad(struct mtd_info *mtd, loff_t ofs);
static int ig_block_markbad(struct mtd_info *mtd, loff_t ofs);
static int ig_writev (struct mtd_info *mtd, const struct kvec *vecs,
			unsigned long count, loff_t to, size_t * retlen);
static int ig_writev_ecc (struct mtd_info *mtd, const struct kvec *vecs, unsigned long count,
			loff_t to, size_t * retlen, u_char *eccbuf, struct nand_oobinfo *oobsel);

/*
 * allocate packet from free list and insert to pending list
 * packet is marked as not-ready.
 * returns allocated packet.
 */
static struct client_cmd_packet *
alloc_cmd_packet(struct ig_mtd *vmtd)
{
	struct client_cmd_packet *packet;

	packet = vmtd->free_list;

	if (packet) {
		struct control_block *control = vmtd->control;
		// dequeue packet
		vmtd->free_list = packet->next;
		
		// setup packet to not-ready
		packet->status = PACKET_SETUP;

		mutex_lock(&(control->queue_lock));

		// add packet to pending queue

		if (control->pend_list) {
			packet->next = control->pend_list;
			packet->prev = control->pend_list->prev;
			control->pend_list->prev = packet;
		} else {
			packet->next = packet;
			packet->prev = packet;
		}

		control->pend_list = packet;

		mutex_unlock(&(control->queue_lock));
	}

	return packet;
}

/*
 * move a packet from done list and insert to free list
 */
static void
free_cmd_packet(struct ig_mtd *vmtd, struct client_cmd_packet *packet)
{
	struct control_block *control = vmtd->control;
	struct client_cmd_packet * walk;
	DEBUG_MSG("%s: %p\n", __func__, packet);

	mutex_lock(&(control->queue_lock));
	
	// find packet - XXX not most efficient way
	walk = control->done_list;

	if (walk == packet) {
		control->done_list = packet->next;
	} else {
		while (walk && walk->next != packet) {
			walk = walk->next;
		}

		if (!walk || walk->next != packet) {
			mutex_unlock(&(control->queue_lock));
			printk(" - error packet not in done list?\n");
			return;
		}

		walk->next = packet->next;
	}
	mutex_unlock(&(control->queue_lock));

	// add packet to free list
	packet->next = vmtd->free_list;
	vmtd->free_list = packet;
}

/* 8-bit NAND Flash */
static struct nand_oobinfo nand_oob_8 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 3,
	.eccpos = {6, 7, 8},
	.oobfree = { {0, 5}, {11, 5} }
};


static int
ig_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	return ig_read_ecc(mtd, from, len, retlen, buf, NULL, NULL);
}

static int
ig_read_ecc(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf,
		u_char *eccbuf, struct nand_oobinfo *oobsel)
{
	uint32_t page, offset, length;
	struct ig_mtd *this = (struct ig_mtd*)mtd->priv;
	struct client_cmd_packet *packet;

	DEBUG_MSG("%s: from %llx : len %x (eccbuf %p) called\n", __func__, from, len, eccbuf);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG_MSG("%s: attempt to read past end device\n", __func__);
		*retlen = 0;
	}

	page = from / mtd->oobblock;
	offset = from % mtd->oobblock;

	packet = alloc_cmd_packet(this);

	if (packet) {
		int ecc;
		// Setup packet
		packet->cmd = MTD_CMD_READ_ECC;
		packet->args[0] = page;
		packet->args[1] = offset;
		packet->args[2] = (unsigned long)eccbuf;
		packet->length = len;
		packet->data = buf;

		packet->ref = mtd;

		// lastly - set packet to pending
		packet->status = PACKET_PEND;

		DEBUG_MSG("%s: notify server\n", __func__);
		// Notify server
		L4_Notify(this->server, 0x1);

		// Handle result
		wait_event(this->wq, packet->status == PACKET_DONE);
		DEBUG_MSG("%s: pkt done\n", __func__);

		length = packet->length;
		ecc = packet->args[2];

		/* By now packet should be on done_list */
		free_cmd_packet(this, packet);

		if (length == -1ul) {
			*retlen = 0;
			DEBUG (MTD_DEBUG_LEVEL0, "%s: error\n", __func__);
			return -EIO;
		}

		*retlen = packet->length;

		return ecc ? -EBADMSG : 0;
	}
	// XXX should we block and try again later?
	*retlen = 0;
	DEBUG (MTD_DEBUG_LEVEL0, "%s: no packets free\n", __func__);
	return -ENOMEM;
}

static int
ig_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	return ig_write_ecc(mtd, to, len, retlen, buf, NULL, NULL);
}

static int
ig_write_ecc(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf,
		u_char *eccbuf, struct nand_oobinfo *oobsel)
{
	uint32_t page, length;
	struct ig_mtd *this = (struct ig_mtd*)mtd->priv;
	struct client_cmd_packet *packet;

	DEBUG_MSG("%s: to %llx : len %x (eccbuf %p) called\n", __func__, to, len, eccbuf);

	/* Start address/len must align on page boundary */
	if ((to|len) % mtd->oobblock) {
		DEBUG (MTD_DEBUG_LEVEL0, "%s: Unaligned address\n", __func__);
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((to + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: Erase past end of device\n");
		return -EINVAL;
	}

	page = to / mtd->oobblock;

	packet = alloc_cmd_packet(this);

	if (packet) {
		// Setup packet
		packet->cmd = MTD_CMD_WRITE_ECC;
		packet->args[0] = page;
		packet->args[1] = 0;
		packet->args[2] = (unsigned long)eccbuf;
		packet->length = len;
		packet->data = (void*)buf;

		packet->ref = mtd;

		// lastly - set packet to pending
		packet->status = PACKET_PEND;

		DEBUG_MSG("%s: notify server\n", __func__);
		// Notify server
		L4_Notify(this->server, 0x1);

		// Handle result
		wait_event(this->wq, packet->status == PACKET_DONE);
		DEBUG_MSG("%s: pkt done\n", __func__);

		length = packet->length;

		/* By now packet should be on done_list */
		free_cmd_packet(this, packet);

		if (length == -1ul) {
			*retlen = 0;
			DEBUG (MTD_DEBUG_LEVEL0, "%s: error\n", __func__);
			return -EIO;
		}

		*retlen = packet->length;

		return 0;
	}
	// XXX should we block and try again later?
	*retlen = 0;
	DEBUG (MTD_DEBUG_LEVEL0, "%s: no packets free\n", __func__);
	return -ENOMEM;
}

static int
ig_writev (struct mtd_info *mtd, const struct kvec *vecs,
			unsigned long count, loff_t to, size_t * retlen)
{
	return (ig_writev_ecc (mtd, vecs, count, to, retlen, NULL, NULL));
}

static int
ig_writev_ecc (struct mtd_info *mtd, const struct kvec *vecs, unsigned long count,
			loff_t to, size_t * retlen, u_char *eccbuf, struct nand_oobinfo *oobsel)
{
	uint32_t page, length, i, totlen, copied;
	struct ig_mtd *this = (struct ig_mtd*)mtd->priv;
	struct client_cmd_packet *packet;
	char *tbuf;
	size_t iov_offset;

	*retlen = 0;

	totlen = 0;
	/* Calculate data length */
	for (i = 0; i < count; i++) {
		totlen += vecs[i].iov_len;
	}

	DEBUG_MSG("%s: to %llx : len %x [count=%d] (eccbuf %p) called\n", __func__, to, totlen, count, eccbuf);

	/* Start address/len must align on page boundary */
	if ((to|totlen) % mtd->oobblock) {
		DEBUG (MTD_DEBUG_LEVEL0, "%s: Unaligned address\n", __func__);
		return -EINVAL;
	}

	/* Do not allow write past end of device */
	if ((to + totlen) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_writev: Write past end of device\n");
		return -EINVAL;
	}

	page = to / mtd->oobblock;
	copied = 0;

	tbuf = kmalloc(mtd->oobblock, GFP_KERNEL);
	if (!tbuf) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_writev: Cannot alloc mem\n");
		return -ENOMEM;
	}

	iov_offset = 0;
	while (copied < totlen) {
		uintptr_t data_size;

		packet = alloc_cmd_packet(this);

		if (!packet) {
			// XXX should we block and try again later?
			*retlen = 0;
			kfree(tbuf);
			DEBUG (MTD_DEBUG_LEVEL0, "%s: no packets free\n", __func__);
			return -ENOMEM;
		}
		// Setup packet
		packet->cmd = MTD_CMD_WRITE_ECC;
		packet->args[0] = page;
		packet->args[1] = 0;
		packet->args[2] = 0;//(unsigned long)eccbuf;	-seems not used here?

		/* If the given tuple is >= pagesize then
		 * write it out from the iov
		 */
		if ((vecs->iov_len - iov_offset) >= mtd->oobblock) {
			/* Calc number of pages we can write
			 * out of this iov in one go */
			uintptr_t bufstart, numpages;

			numpages = (vecs->iov_len - iov_offset) / mtd->oobblock;
			bufstart = (uintptr_t)vecs->iov_base;
			bufstart += iov_offset;
			data_size = mtd->oobblock * numpages;

			packet->data = (void*)bufstart;
			packet->length = data_size;

			iov_offset += data_size;
			page += numpages;

			/* Check, if we have to switch to the next tuple */
			if (iov_offset >= vecs->iov_len) {
				vecs++;
				iov_offset = 0;
				count--;
			}
		} else {
			/* We must use the internal buffer, read data out of each 
			 * tuple until we have a full page to write
			 */
			int cnt = 0;
			while (cnt < mtd->oobblock) {
				if (vecs->iov_base != NULL && vecs->iov_len) 
					tbuf[cnt++] = ((u_char *) vecs->iov_base)[iov_offset++];
				/* Check, if we have to switch to the next tuple */
				if (iov_offset >= vecs->iov_len) {
					vecs++;
					iov_offset = 0;
					count--;
				}
			}
			data_size = mtd->oobblock;
			packet->length = mtd->oobblock;
			packet->data = (void*)tbuf;

			page++;
		}

		packet->ref = mtd;

		// lastly - set packet to pending
		packet->status = PACKET_PEND;

		DEBUG_MSG("%s: notify server\n", __func__);
		// Notify server
		L4_Notify(this->server, 0x1);

		// Handle result
		wait_event(this->wq, packet->status == PACKET_DONE);
		DEBUG_MSG("%s: pkt done\n", __func__);

		length = packet->length;

		/* By now packet should be on done_list */
		free_cmd_packet(this, packet);

		if (length == -1ul) {
			*retlen = 0;
			DEBUG (MTD_DEBUG_LEVEL0, "%s: error\n", __func__);
			kfree(tbuf);
			return -EIO;
		}

		copied += length;

		// did not copy all data?
		if (length != data_size)
		    break;
		// done all iovecs?
		if (count == 0)
		    break;
	}
	*retlen = copied;

	kfree(tbuf);
	return 0;
}

static int
ig_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	uint32_t page, error;
	struct ig_mtd *this = (struct ig_mtd*)mtd->priv;
	struct client_cmd_packet *packet;

	/* Start address must align on block boundary */
	if (instr->addr % mtd->erasesize) {
		DEBUG (MTD_DEBUG_LEVEL0, "%s: Unaligned address\n", __func__);
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (instr->len % mtd->erasesize) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: Length not block aligned\n");
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->len + instr->addr) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: Erase past end of device\n");
		return -EINVAL;
	}

	DEBUG_MSG("%s: block %lx, len %lx\n", __func__, instr->addr, instr->len);

	instr->fail_addr = 0xffffffff;

	page = instr->addr / mtd->oobblock;

	packet = alloc_cmd_packet(this);

	if (packet) {
		// Setup packet
		packet->cmd = MTD_CMD_ERASE_BLOCK;
		packet->args[0] = page;
		packet->args[1] = 0;
		packet->length = instr->len;
		packet->data = NULL;

		packet->ref = mtd;

		instr->state = MTD_ERASING;

		// lastly - set packet to pending
		packet->status = PACKET_PEND;

		DEBUG_MSG("%s: notify server\n", __func__);
		// Notify server
		L4_Notify(this->server, 0x1);

		// Handle result
		wait_event(this->wq, packet->status == PACKET_DONE);
		DEBUG_MSG("%s: pkt done\n", __func__);

		error = packet->length;

		/* By now packet should be on done_list */
		free_cmd_packet(this, packet);

		if (error == -1ul) {
			DEBUG (MTD_DEBUG_LEVEL0, "%s: error\n", __func__);
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = packet->args[0] * mtd->oobblock;
			return -EIO;
		}

		instr->state = MTD_ERASE_DONE;
		/* Do call back function */
		mtd_erase_callback(instr);

		return 0;
	}

	DEBUG (MTD_DEBUG_LEVEL0, "%s: no packets free\n", __func__);
	// XXX should we block and try again later?
	instr->state = MTD_ERASE_FAILED;
	return -ENOMEM;
}

static int
ig_read_oob(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	uint32_t page, offset, length;
	struct ig_mtd *this = (struct ig_mtd*)mtd->priv;
	struct client_cmd_packet *packet;

	DEBUG_MSG("%s: from %llx : len %x called\n", __func__, from, len);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG_MSG("%s: attempt to read past end device\n", __func__);
		*retlen = 0;
	}

	page = from / mtd->oobblock;
	offset = from % mtd->oobsize;

	packet = alloc_cmd_packet(this);

	if (packet) {
		int ecc;
		// Setup packet
		packet->cmd = MTD_CMD_READ_OOB;
		packet->args[0] = page;
		packet->args[1] = offset;
		packet->length = len;
		packet->data = buf;

		packet->ref = mtd;

		// lastly - set packet to pending
		packet->status = PACKET_PEND;

		DEBUG_MSG("%s: notify server\n", __func__);
		// Notify server
		L4_Notify(this->server, 0x1);

		// Handle result
		wait_event(this->wq, packet->status == PACKET_DONE);
		DEBUG_MSG("%s: pkt done\n", __func__);

		length = packet->length;
		ecc = packet->args[2];

		/* By now packet should be on done_list */
		free_cmd_packet(this, packet);

		if (length == -1ul) {
			DEBUG (MTD_DEBUG_LEVEL0, "%s: error\n", __func__);
			*retlen = 0;
			return -EIO;
		}

		*retlen = packet->length;

		return ecc ? -EBADMSG : 0;
	}
	// XXX should we block and try again later?
	*retlen = 0;
	DEBUG (MTD_DEBUG_LEVEL0, "%s: no packets free\n", __func__);
	return -ENOMEM;
}

static int
ig_write_oob(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	uint32_t page, offset, length;
	struct ig_mtd *this = (struct ig_mtd*)mtd->priv;
	struct client_cmd_packet *packet;

	DEBUG_MSG("%s: to %llx : len %x called\n", __func__, to, len);

	/* Do not allow writes past end of device */
	if ((to + len) > mtd->size) {
		DEBUG_MSG("%s: attempt to write past end device\n", __func__);
		*retlen = 0;
	}

	page = to / mtd->oobblock;
	offset = to % mtd->oobsize;

	packet = alloc_cmd_packet(this);

	if (packet) {
		// Setup packet
		packet->cmd = MTD_CMD_WRITE_OOB;
		packet->args[0] = page;
		packet->args[1] = offset;
		packet->length = len;
		packet->data = (void*)buf;

		packet->ref = mtd;

		// lastly - set packet to pending
		packet->status = PACKET_PEND;

		DEBUG_MSG("%s: notify server\n", __func__);
		// Notify server
		L4_Notify(this->server, 0x1);

		// Handle result
		wait_event(this->wq, packet->status == PACKET_DONE);
		DEBUG_MSG("%s: pkt done\n", __func__);

		length = packet->length;

		/* By now packet should be on done_list */
		free_cmd_packet(this, packet);

		if (length == -1ul) {
			DEBUG (MTD_DEBUG_LEVEL0, "%s: error\n", __func__);
			*retlen = 0;
			return -EIO;
		}

		*retlen = packet->length;

		return 0;
	}
	// XXX should we block and try again later?
	*retlen = 0;
	DEBUG (MTD_DEBUG_LEVEL0, "%s: no packets free\n", __func__);
	return -ENOMEM;
}

static int
ig_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	char *buf = kmalloc(mtd->oobblock, GFP_KERNEL);
	int i, fail = 0;

	for (i = 0; i < mtd->erasesize; i += mtd->oobblock)
	{
		int ret;
		size_t len;
		ret = ig_read(mtd, ofs, mtd->oobblock, &len, buf);

		if (ret == -EBADMSG) {
			fail = 1;
			break;
		}
	}
	kfree(buf);
	return fail;
}

static int ig_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	printk("%s: called\n", __func__);
	return -EINVAL;
}

/* Handle interrupts */
static irqreturn_t 
ig_mtd_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
	struct ig_mtd *this;
	struct mtd_info *mtd;

	DEBUG_MSG("%s: called\n", __func__);

	if (ig_mtdlist == NULL) {
		return 0;
	}

	mtd = ig_mtdlist;
	
	while (mtd) {
		this = (struct ig_mtd *)(mtd->priv);
		wake_up(&this->wq);
		mtd = this->next;
	}
	return 0;
}

/* === MODULE INIT === */

static int register_device(char *name, struct ig_mtd *vmtd)
{
	struct ig_mtd *this;
	struct mtd_info *mtd, *mtd_prev, *new;

	new = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!new) {
		ERROR_MSG("ig_mtd: Cannot allocate new MTD device.\n");
		return -ENOMEM;
	}
	memset(new, 0, sizeof(struct mtd_info));

	if (ig_mtdlist == NULL) {
		ig_mtdlist = new;
	}

	mtd = ig_mtdlist;
	mtd_prev = NULL;
	
	while (mtd->priv) {
		this = (struct ig_mtd *)(mtd->priv);
		if (!this->next) {
			this->next = new;
		}
		mtd = this->next;
		mtd_prev = mtd;	    // to handle errors: list removal
	}

	this = vmtd;

	mtd->priv = this;

	// XXX Iguana Init


	mtd->name = name;

	// XXX get these from server
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->ecctype = MTD_ECC_SW; //MTD_ECC_HW;
	mtd->size = vmtd->control->info.blocks * vmtd->control->info.block_size;
	mtd->erasesize = vmtd->control->info.erase_size;
	mtd->oobblock = vmtd->control->info.page_size;
	mtd->oobsize = vmtd->control->info.oob_size;

	mtd->erase = ig_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = ig_read;
	mtd->write = ig_write;
	mtd->read_ecc = ig_read_ecc;
	mtd->write_ecc = ig_write_ecc;
	mtd->read_oob = ig_read_oob;
	mtd->write_oob = ig_write_oob;
	mtd->readv = NULL;
	mtd->writev = ig_writev;
	mtd->writev_ecc = ig_writev_ecc;
	mtd->sync = NULL;
	mtd->block_isbad = ig_block_isbad;
	mtd->block_markbad = ig_block_markbad;

	// XXX
	memcpy(&mtd->oobinfo, &nand_oob_8, sizeof(mtd->oobinfo));

	mtd->owner = THIS_MODULE;

	if (add_mtd_device(mtd)) {
		struct ig_mtd *prev = (struct ig_mtd *)(mtd_prev->priv);
		ERROR_MSG("ig_mtd: Failed to register new device\n");
		prev->next = NULL;
		kfree(mtd);
		kfree(this);
		return -EAGAIN;
	}
	DEBUG_MSG("ig_mtd: registed device %lx on server %lx\n",
			this->dev, this->server.raw);
	return 0;
}

static void unregister_devices(void)
{
	struct ig_mtd *this;
	struct mtd_info *mtd, *prev_mtd;

	if (ig_mtdlist == NULL) {
		return;
	}

	mtd = ig_mtdlist;
	
	while (mtd) {
		this = (struct ig_mtd *)(mtd->priv);
		del_mtd_device(mtd);
		prev_mtd = mtd;
		mtd = this->next;

		kfree(this);
		kfree(prev_mtd);
	}
}

extern L4_ThreadId_t timer_thread;

// XXX put in header
extern int iguana_alloc_irq(void);

static void init_packets(struct ig_mtd* vmtd)
{
    struct client_cmd_packet *packet;
    int i = 0;

    /* Initialise the free list */
    vmtd->control->pend_list = NULL;
    vmtd->control->done_list = NULL;

    vmtd->free_list =
        (struct client_cmd_packet*)(vmtd->cmd_base +
                                 sizeof(struct control_block));

    for (packet = vmtd->free_list;
         (packet+1) < (struct client_cmd_packet *)(vmtd->cmd_base + 4096); packet++)
    {
        vmtd->free_list[i].next = &vmtd->free_list[i + 1];
        i++;
    }
    vmtd->free_list[i-1].next = NULL;

    mutex_init(&(vmtd->control->queue_lock));

    printk("igmtd: inited cmd %d packets\n", i);
}

static int __init mtd_ig_init(void)
{
	int ret;
	struct ig_mtd* vmtd;

	printk("Iguana virtual MTD driver v1\n");

	// Scan Iguana for devices
	{
		thread_ref_t server_;
		L4_ThreadId_t server;
		CORBA_Environment env;

		memsection_lookup(env_memsection_base(iguana_getenv("OKL4_CORE_DEVICE_SERVER")),
                                  &server_);

		vmtd = kmalloc(sizeof(struct ig_mtd), GFP_KERNEL);
		if (!vmtd) {
			ERROR_MSG("ig_mtd: Cannot allocate new MTD device.\n");
			return -ENOMEM;
		}
		memset(vmtd, 0, sizeof(struct ig_mtd));

		server = thread_l4tid(server_);

		/* Allocate an IRQ number */
		vmtd->irq = iguana_alloc_irq();
		printk("IG_vMTD on IRQ %d\n", vmtd->irq);
		request_irq(vmtd->irq, ig_mtd_interrupt, 0, "IG_vMTD", vmtd);

		/* Attach to the MTD server */
		vmtd->dev = device_core_get_mtd(server, &vmtd->server,
				&timer_thread, IGUANA_IRQ_NOTIFY_MASK(vmtd->irq), &env);

		vmtd->control_ms = memsection_create(4096, &(vmtd->cmd_base));
		virtual_mtd_add_memsection(vmtd->server, vmtd->dev, vmtd->control_ms, 0, &env);
		virtual_mtd_register_control_block(vmtd->server, vmtd->dev, 0, 0, &env);

		vmtd->control = (struct control_block*)vmtd->cmd_base;
		init_waitqueue_head(&vmtd->wq);

		init_packets(vmtd);

		if ((ret = register_device("Iguana MTD", vmtd))){
			unregister_devices();
			return ret;
		}
	}

	return 0;
}

static void __exit mtd_ig_cleanup(void)
{
	unregister_devices();
}

module_init(mtd_ig_init);
module_exit(mtd_ig_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carl van Schaik <carl@ok-labs.com>");
MODULE_DESCRIPTION("MTD driver for Iguana virtual MTD devices");
