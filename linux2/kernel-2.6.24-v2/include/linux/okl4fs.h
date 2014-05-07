#ifndef _LINUX_OKL4FS_H
#define _LINUX_OKL4FS_H

struct inode *okl4fs_get_inode(struct super_block *sb, int mode, dev_t dev);
extern int okl4fs_get_sb(struct file_system_type *fs_tpye, 
        int flags, const char *dev_name, void *data, struct vfsmount *mnt);

extern const struct file_operations okl4fs_file_operations;
extern int __init init_okl4fs(void);

#endif
