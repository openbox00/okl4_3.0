/* internal.h: okl4fs internal definitions
 */

#ifndef _OKL4FS_INTERNAL_H
#define _OKL4FS_INTERNAL_H

#include <linux/fs.h>
#include <linux/sched.h>
#include <fs/fs.h>

struct blocked_linux_thread {
    struct task_struct *task;
    STAILQ_ENTRY(blocked_linux_thread) links;
};

extern const struct address_space_operations okl4fs_aops;            
extern const struct file_operations okl4fs_file_operations;          
extern const struct inode_operations okl4fs_file_inode_operations;   

ssize_t okl4fs_read(struct file *, char __user *, size_t, loff_t *);
ssize_t okl4fs_write(struct file *, const char __user *, size_t, loff_t *);
loff_t okl4fs_llseek(struct file *, loff_t, int);
int okl4fs_fsync(struct file *, struct dentry *, int);
int okl4fs_close(struct file *, fl_owner_t id);
int okl4fs_open(struct inode *, struct file *);

#endif	/*_OKL4FS_INTERNAL_H*/
