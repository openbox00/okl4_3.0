#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/seq_file.h>

/* Used for cross space copy */
//#include <asm/segment.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#include <okl4/env.h>
#include <l4/ipc.h>
#include <l4/misc.h>

#define DECRYPT_MASK  0x34

#define CROSS_CELL_MAJOR 220

/**
 * The Bare device is a variable length
 * region of memory. So, we use a linked list of 
 * indirect blocks
 */

#define CROSS_CELL_NR_DEVICES 1



int cross_cell_major = CROSS_CELL_MAJOR;
int cross_cell_minor = 0;

L4_CapId_t decrypt_cell;
char *shmem_base;
extern L4_Word_t decrypt_notify;
L4_Word_t written_data = 0;
extern L4_Word_t irq_mask;

module_param(cross_cell_major, int, S_IRUGO);
module_param(cross_cell_minor, int, S_IRUGO);

struct cross_cell_dev {
	unsigned long size;
	struct semaphore sem;
	struct cdev cdev;
};

struct cross_cell_dev *cross_cell_devices;

struct task *user_task;

/* prototypes */

int cross_cell_open (struct inode *inode, struct file *filp);
void cross_cell_release (struct inode *inode, struct file *filp);
ssize_t  cross_cell_read (struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos);
ssize_t  cross_cell_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos);
static irqreturn_t cross_cell_interrupt(int irq, void *dev_id);


/* The different file operations */

static const struct file_operations cross_cell_fops = {
	.owner = THIS_MODULE,
	.read = cross_cell_read,
	.write = cross_cell_write,
	.open = cross_cell_open,
	.release = cross_cell_release,
};

int
cross_cell_open (struct inode *inode, struct file *filp)
{
	okl4_env_segment_t *shmem;

	/* Device Information */
	struct cross_cell_dev *dev = container_of(inode->i_cdev,
			struct cross_cell_dev, cdev);
	filp->private_data = dev;

	/* Now trim to 0 the length of the device if open was write-only */
	if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		up(&dev->sem);
	}

	decrypt_cell = *(L4_CapId_t *)okl4_env_get("DECRYPT_CELL_CAP");

	shmem = okl4_env_get_segment("MAIN_SHMEM");
	assert(shmem != NULL);

	shmem_base = (char *)shmem->virt_addr;

	return 0;
}

static irqreturn_t
cross_cell_interrupt(int irq, void *dev_id)
{
	decrypt_notify = 1;

	if (user_task != NULL)
		wake_up_process(user_task);

	return IRQ_HANDLED;
}

void
cross_cell_release(struct inode *inode, struct file *filp)
{
	return ;
}

ssize_t
cross_cell_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct cross_cell_dev *dev = filp->private_data; /* the first listitem */
	ssize_t retval = 0;
	L4_Word_t mask = 0;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (strlen(shmem_base) == 0 && written_data == 1) {
		written_data = 0;
		goto out;
	}

	if (!decrypt_notify) {
		user_task = current;
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}
	decrypt_notify = 0;

	if (copy_to_user(buf, shmem_base, strlen(shmem_base) + 1)) {
		retval = -EFAULT;
		goto out;
	}

	retval = strlen(shmem_base);
	*shmem_base = '\0';
	*f_pos += retval;

out:
	up(&dev->sem);
	return retval;
}

ssize_t  cross_cell_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct cross_cell_dev *dev = filp->private_data;
	ssize_t retval = -ENOMEM;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (copy_from_user(shmem_base, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*(shmem_base + count) = '\0';
	L4_Notify(decrypt_cell, DECRYPT_MASK);

	*f_pos += count;
	retval = count;
	written_data = 1;

out:
	up(&dev->sem);
	return retval;
}

/* The module stuff */

static void cross_cell_setup_cdev(struct cross_cell_dev *dev, int index)
{
	int err, devno = MKDEV(cross_cell_major, cross_cell_minor + index);
	cdev_init(&dev->cdev, &cross_cell_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &cross_cell_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err)
		printk("<1>, Error %d adding cross_cell%d", err, index);
}

int cross_cell_init_module(void)
{
	int result;
	dev_t dev = 0;
	/* Register your major number */

	if (cross_cell_major) {
		dev = MKDEV(cross_cell_major, 0);
		result = register_chrdev_region(dev, 1, "cell");
	} else {
		result = alloc_chrdev_region(&dev, 0, 1, "cell");
		cross_cell_major = MAJOR(dev);
	}

	if (result < 0) {
		return result;
	}

	cross_cell_devices = kmalloc(sizeof(struct cross_cell_dev), GFP_KERNEL);
	if (!cross_cell_devices) {
		result = -ENOMEM;
		unregister_chrdev(cross_cell_major, "cell");
		return result;
	}
	memset(cross_cell_devices, 0, sizeof(struct cross_cell_dev));
	init_MUTEX(&cross_cell_devices->sem);
	cross_cell_setup_cdev(cross_cell_devices, 0);

	int irq = 6, i;
	if (!(irq_mask & (1UL << irq))) {
		irq_mask |= (1UL << irq);
		irq += 46;
	}
	i = request_irq(irq, cross_cell_interrupt, 0, "cell", cross_cell_devices);

	/* Success */
	return 0;
}

void cross_cell_cleanup_module(void)
{
	dev_t devno = MKDEV(cross_cell_major, 0);

	if(cross_cell_devices) {
		cdev_del(&cross_cell_devices->cdev);
		kfree(cross_cell_devices);
	}
	unregister_chrdev_region(devno, 1);
}

module_init(cross_cell_init_module);
module_exit(cross_cell_cleanup_module);


