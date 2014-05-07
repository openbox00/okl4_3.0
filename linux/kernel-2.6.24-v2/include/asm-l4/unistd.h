/* 
 * Copyright (C) 2000, 2001  Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#ifndef _L4_UNISTD_H_
#define _L4_UNISTD_H_

#include "linux/resource.h"
#include <asm/uaccess.h>

#include <asm/macros.h>
#include INC_SYSTEM2(unistd.h)

/*
 * "Conditional" syscalls
 *
 * What we want is __attribute__((weak,alias("sys_ni_syscall"))),
 * but it doesn't work on all toolchains, so we just do it by hand
 */
#ifndef cond_syscall
#ifdef __mips
#define cond_syscall(x) asm(".weak\t" #x "\n" #x "\t=\tsys_ni_syscall");
#else
#define cond_syscall(x) asm(".weak\t" #x "\n\t.set\t" #x ",sys_ni_syscall");
#endif
#endif

#ifdef __KERNEL_SYSCALLS__

asmlinkage long sys_wait4(pid_t pid, int __user *stat_addr,
				int options, struct rusage __user *ru);
extern long sys_open(const char *filename, int flags, int mode);
extern long sys_dup(unsigned int fildes);
extern long sys_close(unsigned int fd);
extern int __execve(char *file, char **argv, char **env);
extern int sys_execve(char *file, char **argv, char **env);
extern long sys_setsid(void);
extern long sys_mount(char *dev_name, char *dir_name, char *type, 
		      unsigned long flags, void *data);
extern long sys_select(int n, fd_set *inp, fd_set *outp, fd_set *exp, 
		       struct timeval *tvp);
extern off_t sys_lseek(unsigned int fd, off_t offset, unsigned int origin);
extern ssize_t sys_read(unsigned int fd, char __user *buf, size_t count);
extern ssize_t sys_write(unsigned int fd, const char __user *buf, size_t count);

#define KERNEL_CALL(ret_t, sys, args...)           \
          mm_segment_t fs = get_fs();             \
          ret_t ret;                              \
          set_fs(KERNEL_DS);                      \
          ret = sys(args);                        \
          set_fs(fs);                             \
          return ret;


static inline long open(const char *pathname, int flags, int mode) 
{
	KERNEL_CALL(int, sys_open, pathname, flags, mode)
}

static inline long dup(unsigned int fd)
{
	KERNEL_CALL(int, sys_dup, fd);
}

static inline long close(unsigned int fd)
{
	KERNEL_CALL(int, sys_close, fd);
}

static inline int execve(const char *filename, char *const argv[], 
			 char *const envp[])
{
	//printk("execve called\n");
	/* Because we need to get back to the syscall loop
	   we bounch through __execve */
	KERNEL_CALL(int, __execve, (char*) filename, (char **) argv, 
		    (char **) envp);
}

static inline long waitpid(pid_t pid, int *status, int options)
{
	KERNEL_CALL(pid_t, sys_wait4, pid, status, options, NULL)
}

static inline pid_t setsid(void)
{
	KERNEL_CALL(pid_t, sys_setsid)
}

static inline long lseek(unsigned int fd, off_t offset, unsigned int whence)
{
	KERNEL_CALL(long, sys_lseek, fd, offset, whence)
}

static inline int read(unsigned int fd, char * buf, int len)
{
	KERNEL_CALL(int, sys_read, fd, buf, len)
}

static inline int write(unsigned int fd, char * buf, int len)
{
	KERNEL_CALL(int, sys_write, fd, buf, len)
}
#endif

#endif
