#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include "internal.h"

#include <l4/kdebug.h>
#include <stdarg.h>

extern STAILQ_HEAD(lrqhead, blocked_linux_thread) okl4fs_read_queue;
extern STAILQ_HEAD(lwqhead, blocked_linux_thread) okl4fs_write_queue;

const struct address_space_operations okl4fs_aops = { };

const struct file_operations okl4fs_file_operations = {
	.read = okl4fs_read,
	.write = okl4fs_write,
	.fsync = okl4fs_fsync,
	.llseek = okl4fs_llseek,
	.flush= okl4fs_close,
	.open = okl4fs_open,
};

const struct inode_operations okl4fs_file_inode_operations = { };

int notified = 0;

ssize_t
okl4fs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	char *kbuf;
	int fd;
	int nbytes = 0;
    struct blocked_linux_thread *blt;

	if ((kbuf = kmalloc(count, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	fd = (int)file->f_dentry->d_inode->i_private;

tryagain:
	nbytes = okl4_read(fd, kbuf, count);
    
    if (nbytes == -EAGAIN) {
        /* No data to read - add to queue and block */
        blt = kmalloc(sizeof(struct blocked_linux_thread), GFP_KERNEL);
        if (blt == NULL)
            return -ENOMEM;
        blt->task = current;
        STAILQ_INSERT_HEAD(&okl4fs_read_queue, blt, links);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        if (!notified)
            return -EINTR;
        notified = 0;
        kfree(blt);
        goto tryagain;
    }

	*pos += nbytes;
	if (copy_to_user(buf, kbuf, count)) {
		kfree(kbuf);
		return -EFAULT;
	}

	kfree(kbuf);

	return nbytes;
}

ssize_t
okl4fs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
	char *kbuf;
	int fd;
	int nbytes = 0;
    struct blocked_linux_thread *blt;

	if ((kbuf = kmalloc(count, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	fd = (int)file->f_dentry->d_inode->i_private;
	if (copy_from_user(kbuf, buf, count)) {
		kfree(kbuf);
		return -EFAULT;
	}

tryagain:
	nbytes = okl4_write(fd, kbuf, count);

    if (nbytes == -EAGAIN) {
        blt = kmalloc(sizeof(struct blocked_linux_thread), GFP_KERNEL);
        if (blt == NULL)
            return -ENOMEM;
        blt->task = current;
        STAILQ_INSERT_HEAD(&okl4fs_write_queue, blt, links);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        if (!notified)
            return -EINTR;
        notified = 0;
        goto tryagain;
    }

	*pos += nbytes;
	kfree(kbuf);
	return nbytes;
}

loff_t
okl4fs_llseek(struct file *file, loff_t offset, int origin)
{
	int fd;
	loff_t new_pos;

	fd = (int)file->f_dentry->d_inode->i_private;

	new_pos = okl4_lseek(fd, offset, origin);
	
	file->f_pos = new_pos;

	return new_pos;
}

int
okl4fs_fsync(struct file *file, struct dentry *dentry, int datasync)
{
	int fd;
	
	fd = (int)file->f_dentry->d_inode->i_private;
    if (atomic_dec_and_test(&file->f_count)) {
        atomic_inc(&file->f_count);
       return okl4_close(fd);
    }

    atomic_inc(&file->f_count);
	return 0;
}

int 
okl4fs_close(struct file *file, fl_owner_t id)
{
	int fd;

	fd = (int)file->f_dentry->d_inode->i_private;

	return okl4_close(fd);
}

static int
okl4fs_fcntl(int fd, int cmd, ... )
{
    int ret;
    va_list vl;

    va_start(vl, cmd);
    ret = okl4_fcntl(fd, cmd, vl);
    va_end(vl);

    return ret;
}

int
okl4fs_open(struct inode *inode, struct file *file)
{
	int fd;

    /* Get libfs to open the file */
	fd = okl4_open(file->f_dentry->d_name.name, O_RDONLY);
    if (fd < 0)
        return fd;
	file->f_dentry->d_inode->i_private = (void *)fd;

    /* Set non-blocking mode to avoid blocking the whole kernel */
    fd = okl4fs_fcntl(fd, F_SETFL, O_NONBLOCK);
    if (fd == -EINVAL)
        return -1;

	return 0;
}
