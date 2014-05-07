/*
 * OK Linux interface to the OKL4 Simple File System
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/backing-dev.h>
#include <linux/okl4fs.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <unistd.h>
#include "internal.h"

#include <iguana/env.h>
#include <iguana/memsection.h>

#include <l4/kdebug.h>

#define OKL4FS_MAGIC 0x94521626

static char *intervm_test = "intervm_test";

static const struct super_operations okl4fs_ops;
static const struct inode_operations okl4fs_dir_inode_operations;

static struct backing_dev_info okl4fs_backing_dev_info = {
	.ra_pages = 0,
	.capabilities = BDI_CAP_NO_ACCT_DIRTY | BDI_CAP_NO_WRITEBACK |
			BDI_CAP_MAP_DIRECT | BDI_CAP_MAP_COPY |
			BDI_CAP_READ_MAP | BDI_CAP_WRITE_MAP,
};

STAILQ_HEAD(lrqhead, blocked_linux_thread) okl4fs_read_queue;
STAILQ_HEAD(lwqhead, blocked_linux_thread) okl4fs_write_queue;

struct inode *
okl4fs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
	struct inode *inode = new_inode(sb);

	if (inode) {
		inode->i_mode = mode;
		inode->i_uid = current->fsuid;
		inode->i_gid = current->fsgid;
		inode->i_blocks = 0;
		inode->i_mapping->a_ops = &okl4fs_aops;
		inode->i_mapping->backing_dev_info = &okl4fs_backing_dev_info;
		mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
		switch (mode & S_IFMT) {
			default:
				init_special_inode(inode, mode, dev);
				break;
			case S_IFREG:
				inode->i_op = &okl4fs_file_inode_operations;
				inode->i_fop = &okl4fs_file_operations;
				break;
			case S_IFIFO:
				inode->i_op = &okl4fs_file_inode_operations;
				inode->i_fop = &okl4fs_file_operations;
				break;
			case S_IFDIR:
				inode->i_op = &okl4fs_dir_inode_operations;
				inode->i_fop = &simple_dir_operations;
				break;
		}
	}
	
	return inode;
}

int
okl4fs_mknod(struct inode *dir, struct dentry *dentry, int mode, dev_t dev)
{
	struct inode *inode;

	switch (mode & S_IFMT) {
		case S_IFIFO:
			inode = okl4fs_get_inode(dir->i_sb, mode, dev);
            atomic_inc(&dentry->d_count);
			d_add(dentry, inode);
			dir->i_mtime = dir->i_ctime = CURRENT_TIME;
			break;
		default:
			return -EROFS;
	}

	return 0;
}

static int
okl4fs_create(struct inode *dir, struct dentry *dentry, int mode, struct nameidata *nd)
{
	return -EROFS;
}

static struct dentry *
okl4fs_lookup(struct inode *dir, struct dentry *dentry, struct nameidata *nd)
{
	struct inode *inode = NULL;
	int fd = -1;

	dentry->d_op = dir->i_sb->s_root->d_op;
	if (dentry->d_name.len > 255) {
		return ERR_PTR(-ENAMETOOLONG);
	}
	
	if (strcmp(dentry->d_name.name, intervm_test) != 0) {
		fd = okl4_open(dentry->d_name.name, O_RDONLY);
    }

	if (fd >=0) {
		inode = okl4fs_get_inode(dir->i_sb, 0555 | S_IFREG, 0);
		if (!inode)
			return ERR_PTR(-EACCES);
		inode->i_private = (void *)fd;
		okl4_close(fd);
	} else
		return NULL;
    atomic_inc(&dentry->d_count);
	d_add(dentry, inode);

	return NULL;
}

static const struct inode_operations okl4fs_dir_inode_operations = {
	.create = okl4fs_create,
	.lookup = okl4fs_lookup,
	.mknod = okl4fs_mknod,
};

static const struct super_operations okl4fs_ops = {
	.statfs = simple_statfs,
	.drop_inode = generic_delete_inode,
};

static int
okl4fs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	struct dentry *root;

	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = OKL4FS_MAGIC;
	sb->s_op = &okl4fs_ops;
	sb->s_time_gran = 1;
	inode = okl4fs_get_inode(sb, S_IFDIR | 0755, 0);
	if (!inode)
		return -ENOMEM;
	
	root = d_alloc_root(inode);
	if (!root) {
		iput(inode);
		return -ENOMEM;
	}
	sb->s_root = root;

	return 0;
}

int
okl4fs_get_sb(struct file_system_type *fs_type, int flags, 
		const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_nodev(fs_type, flags, data, okl4fs_fill_super, mnt);
}

static struct file_system_type okl4fs_fs_type = {
	.name = "okl4fs",
	.get_sb = okl4fs_get_sb,
	.kill_sb = kill_litter_super,
};

static int __init
init_okl4fs_fs(void)
{
    STAILQ_INIT(&okl4fs_read_queue);
    STAILQ_INIT(&okl4fs_write_queue);
	return register_filesystem(&okl4fs_fs_type);
}

static void __exit
exit_okl4fs_fs(void)
{
	unregister_filesystem(&okl4fs_fs_type);
}

module_init(init_okl4fs_fs)
module_exit(exit_okl4fs_fs)
