/* 
 * arch/l4/kernel/syscalls.c
 *
 * Copyright 2004 National ICT Australia.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */


#include <linux/types.h>
#include <linux/kernel.h>
#include <l4.h>
#include <asm/syscalls.h>
#include <asm/macros.h>
#include INC_SYSTEM2(syscall_defs.h)
#include INC_SYSTEM2(unistd.h)

extern l4_syscall_handler_t	sys_exit;
extern l4_syscall_handler_t	sys_fork;
extern l4_syscall_handler_t	sys_vfork;
extern l4_syscall_handler_t	sys_execve;
extern l4_syscall_handler_t	sys_clone;
extern l4_syscall_handler_t	sys_kill;
extern l4_syscall_handler_t	sys_tkill;
extern l4_syscall_handler_t	sys_tgkill;
extern l4_syscall_handler_t	sys_getpid;

extern l4_syscall_handler_t	sys_read;
extern l4_syscall_handler_t	sys_write;
extern l4_syscall_handler_t	sys_open;
extern l4_syscall_handler_t	sys_close;
extern l4_syscall_handler_t	sys_stat;
extern l4_syscall_handler_t	sys_newstat;
extern l4_syscall_handler_t	sys_fstat;
extern l4_syscall_handler_t	sys_newfstat;
extern l4_syscall_handler_t	sys_lstat;
extern l4_syscall_handler_t	sys_newlstat;
extern l4_syscall_handler_t	sys_lseek;
extern l4_syscall_handler_t	sys_mprotect;
extern l4_syscall_handler_t	sys_munmap;
extern l4_syscall_handler_t	sys_mremap;
extern l4_syscall_handler_t	sys_msync;
extern l4_syscall_handler_t	sys_mlock;
extern l4_syscall_handler_t	sys_munlock;
extern l4_syscall_handler_t	sys_mlockall;
extern l4_syscall_handler_t	sys_munlockall;

extern l4_syscall_handler_t	sys_poll;
extern l4_syscall_handler_t	sys_pread64;
extern l4_syscall_handler_t	sys_pwrite64;
extern l4_syscall_handler_t	sys_readv;
extern l4_syscall_handler_t	sys_writev;
extern l4_syscall_handler_t	sys_ioctl;
extern l4_syscall_handler_t	sys_access;
extern l4_syscall_handler_t	sys_pipe;	    // Seems to be arch dependant
extern l4_syscall_handler_t	sys_select;

extern l4_syscall_handler_t	sys_creat;
extern l4_syscall_handler_t	sys_link;
extern l4_syscall_handler_t	sys_unlink;
extern l4_syscall_handler_t	sys_chdir;
extern l4_syscall_handler_t	sys_time;
extern l4_syscall_handler_t	sys_mknod;
extern l4_syscall_handler_t	sys_chmod;
extern l4_syscall_handler_t	sys_chown;
extern l4_syscall_handler_t	sys_fchmod;
extern l4_syscall_handler_t	sys_fchown;
extern l4_syscall_handler_t	sys_lchown;
extern l4_syscall_handler_t	sys_symlink;
extern l4_syscall_handler_t	sys_readlink;
extern l4_syscall_handler_t	sys_umask;
extern l4_syscall_handler_t	sys_fchdir;
extern l4_syscall_handler_t	sys_rename;
extern l4_syscall_handler_t	sys_mkdir;
extern l4_syscall_handler_t	sys_rmdir;
extern l4_syscall_handler_t	sys_utime;
extern l4_syscall_handler_t	sys_utimes;

extern l4_syscall_handler_t	sys_wait4;
extern l4_syscall_handler_t	sys_newuname;

extern l4_syscall_handler_t	sys_restart_syscall;

extern l4_syscall_handler_t	sys_brk;
extern l4_syscall_handler_t	sys_rt_sigaction;
extern l4_syscall_handler_t	sys_rt_sigprocmask;
extern l4_syscall_handler_t	sys_rt_sigreturn;
extern l4_syscall_handler_t	sys_rt_sigpending;
extern l4_syscall_handler_t	sys_rt_sigtimedwait;
extern l4_syscall_handler_t	sys_rt_sigqueueinfo;
extern l4_syscall_handler_t	sys_sigaltstack;
extern l4_syscall_handler_t	sys_sigpending;
extern l4_syscall_handler_t	sys_sigprocmask;

extern l4_syscall_handler_t	sys_fcntl;
extern l4_syscall_handler_t	sys_flock;
extern l4_syscall_handler_t	sys_fsync;
extern l4_syscall_handler_t	sys_fdatasync;
extern l4_syscall_handler_t	sys_truncate;
extern l4_syscall_handler_t	sys_ftruncate;
extern l4_syscall_handler_t	sys_getdents;
extern l4_syscall_handler_t	sys_getcwd;

extern l4_syscall_handler_t	sys_sysinfo;
extern l4_syscall_handler_t	sys_times;
extern l4_syscall_handler_t	sys_syslog;

extern l4_syscall_handler_t	sys_gettimeofday;
extern l4_syscall_handler_t	sys_settimeofday;
extern l4_syscall_handler_t	sys_setrlimit;
extern l4_syscall_handler_t	sys_getrlimit;
extern l4_syscall_handler_t	sys_getrusage;

extern l4_syscall_handler_t	sys_socket;
extern l4_syscall_handler_t	sys_socketcall;

extern l4_syscall_handler_t	sys_gettid;
extern l4_syscall_handler_t	sys_getuid;
extern l4_syscall_handler_t	sys_getgid;
extern l4_syscall_handler_t	sys_setuid;
extern l4_syscall_handler_t	sys_setgid;
extern l4_syscall_handler_t	sys_geteuid;
extern l4_syscall_handler_t	sys_getegid;
extern l4_syscall_handler_t	sys_setpgid;
extern l4_syscall_handler_t	sys_getppid;
extern l4_syscall_handler_t	sys_getpgrp;
extern l4_syscall_handler_t	sys_setsid;
extern l4_syscall_handler_t	sys_setreuid;
extern l4_syscall_handler_t	sys_setregid;
extern l4_syscall_handler_t	sys_getgroups;
extern l4_syscall_handler_t	sys_setgroups;
extern l4_syscall_handler_t	sys_setresuid;
extern l4_syscall_handler_t	sys_getresuid;
extern l4_syscall_handler_t	sys_setresgid;
extern l4_syscall_handler_t	sys_getresgid;
extern l4_syscall_handler_t	sys_getpgid;
extern l4_syscall_handler_t	sys_setfsuid;
extern l4_syscall_handler_t	sys_setfsgid;
extern l4_syscall_handler_t	sys_getsid;
extern l4_syscall_handler_t	sys_capget;
extern l4_syscall_handler_t	sys_capset;

extern l4_syscall_handler_t	sys_sysctl;

extern l4_syscall_handler_t	sys_timer_create;
extern l4_syscall_handler_t	sys_timer_settime;
extern l4_syscall_handler_t	sys_timer_gettime;
extern l4_syscall_handler_t	sys_timer_getoverrun;
extern l4_syscall_handler_t	sys_timer_delete;
extern l4_syscall_handler_t	sys_clock_settime;
extern l4_syscall_handler_t	sys_clock_gettime;
extern l4_syscall_handler_t	sys_clock_getres;
extern l4_syscall_handler_t	sys_clock_nanosleep;

extern l4_syscall_handler_t	sys_ustat;
extern l4_syscall_handler_t	sys_statfs;
extern l4_syscall_handler_t	sys_fstatfs;
extern l4_syscall_handler_t	sys_statfs64;
extern l4_syscall_handler_t	sys_fstatfs64;
extern l4_syscall_handler_t	sys_sysfs;

extern l4_syscall_handler_t	sys_chroot;
extern l4_syscall_handler_t	sys_sync;
extern l4_syscall_handler_t	sys_mount;
extern l4_syscall_handler_t	sys_umount;

extern l4_syscall_handler_t	sys_swapon;
extern l4_syscall_handler_t	sys_swapoff;
extern l4_syscall_handler_t	sys_reboot;
extern l4_syscall_handler_t	sys_sethostname;
extern l4_syscall_handler_t	sys_setdomainname;
extern l4_syscall_handler_t	sys_exit_group;

extern l4_syscall_handler_t	sys_dup;
extern l4_syscall_handler_t	sys_dup2;
extern l4_syscall_handler_t	sys_pause;
extern l4_syscall_handler_t	sys_nanosleep;
extern l4_syscall_handler_t	sys_getitimer;
extern l4_syscall_handler_t	sys_alarm;
extern l4_syscall_handler_t	sys_setitimer;

extern l4_syscall_handler_t	sys_acct;
extern l4_syscall_handler_t	sys_getpriority;
extern l4_syscall_handler_t	sys_setpriority;
extern l4_syscall_handler_t	sys_sched_setparam;
extern l4_syscall_handler_t	sys_sched_getparam;
extern l4_syscall_handler_t	sys_sched_setscheduler;
extern l4_syscall_handler_t	sys_sched_getscheduler;
extern l4_syscall_handler_t	sys_sched_yield;
extern l4_syscall_handler_t	sys_sched_get_priority_max;
extern l4_syscall_handler_t	sys_sched_get_priority_min;
extern l4_syscall_handler_t	sys_sched_rr_get_interval;

extern l4_syscall_handler_t	sys_adjtimex;
extern l4_syscall_handler_t	sys_nice;

extern l4_syscall_handler_t	sys_uselib;

extern l4_syscall_handler_t	sys_create_module;
extern l4_syscall_handler_t	sys_init_module;
extern l4_syscall_handler_t	sys_delete_module;

extern l4_syscall_handler_t	sys_setxattr;
extern l4_syscall_handler_t	sys_lsetxattr;
extern l4_syscall_handler_t	sys_fsetxattr;
extern l4_syscall_handler_t	sys_getxattr;
extern l4_syscall_handler_t	sys_lgetxattr;
extern l4_syscall_handler_t	sys_fgetxattr;
extern l4_syscall_handler_t	sys_listxattr;
extern l4_syscall_handler_t	sys_llistxattr;
extern l4_syscall_handler_t	sys_flistxattr;
extern l4_syscall_handler_t	sys_removexattr;
extern l4_syscall_handler_t	sys_lremovexattr;
extern l4_syscall_handler_t	sys_fremovexattr;

extern l4_syscall_handler_t	sys_mq_open;
extern l4_syscall_handler_t	sys_mq_unlink;
extern l4_syscall_handler_t	sys_mq_timedsend;
extern l4_syscall_handler_t	sys_mq_timedreceive;
extern l4_syscall_handler_t	sys_mq_notify;
extern l4_syscall_handler_t	sys_mq_getsetattr;

extern l4_syscall_handler_t	sys_vhangup;
extern l4_syscall_handler_t	sys_quotactl;
extern l4_syscall_handler_t	sys_prctl;

extern l4_syscall_handler_t	sys_sendfile;
extern l4_syscall_handler_t	sys_sendfile64;

extern l4_syscall_handler_t	sys_pivot_root;
extern l4_syscall_handler_t	sys_mincore;
extern l4_syscall_handler_t	sys_madvise;
extern l4_syscall_handler_t	sys_readahead;

extern l4_syscall_handler_t	sys_futex;
extern l4_syscall_handler_t	sys_sched_setaffinity;
extern l4_syscall_handler_t	sys_sched_getaffinity;
extern l4_syscall_handler_t	sys_io_setup;
extern l4_syscall_handler_t	sys_io_destroy;
extern l4_syscall_handler_t	sys_io_getevents;
extern l4_syscall_handler_t	sys_io_submit;
extern l4_syscall_handler_t	sys_io_cancel;

extern l4_syscall_handler_t	sys_lookup_dcookie;
extern l4_syscall_handler_t	sys_epoll_create;
extern l4_syscall_handler_t	sys_epoll_ctl;
extern l4_syscall_handler_t	sys_epoll_wait;
extern l4_syscall_handler_t	sys_remap_file_pages;

#ifdef __ARCH_WANT_SYS_FADVISE64
extern l4_syscall_handler_t	sys_fadvise64;
#endif
extern l4_syscall_handler_t	sys_fadvise64_64;
extern l4_syscall_handler_t	sys_pciconfig_read;
extern l4_syscall_handler_t	sys_pciconfig_write;

extern l4_syscall_handler_t	sys_faccessat;
extern l4_syscall_handler_t	sys_fchmodat;
extern l4_syscall_handler_t	sys_fchownat;
#ifdef __ARCH_WANT_STAT64
extern l4_syscall_handler_t	sys_fstatat64;
#endif
#ifdef __ARCH_WANT_NEWFSTATAT
extern l4_syscall_handler_t	sys_newfstat;
#endif
extern l4_syscall_handler_t	sys_futimesat;
extern l4_syscall_handler_t	sys_linkat;
extern l4_syscall_handler_t	sys_openat;
extern l4_syscall_handler_t	sys_readlinkat;
extern l4_syscall_handler_t	sys_renameat;
extern l4_syscall_handler_t	sys_symlinkat;
extern l4_syscall_handler_t	sys_unlinkat;

extern l4_syscall_handler_t	sys_tee;
extern l4_syscall_handler_t	sys_splice;
extern l4_syscall_handler_t	sys_vmsplice;


extern l4_syscall_handler_t	sys_timerfd;
extern l4_syscall_handler_t	sys_add_key;
extern l4_syscall_handler_t	sys_epoll_pwait;
extern l4_syscall_handler_t	sys_fallocate;
extern l4_syscall_handler_t	sys_getcpu;
extern l4_syscall_handler_t	sys_get_mempolicy;
extern l4_syscall_handler_t	sys_set_mempolicy;
extern l4_syscall_handler_t	sys_migrate_pages;
extern l4_syscall_handler_t	sys_ioprio_set;
extern l4_syscall_handler_t	sys_ioprio_get;
extern l4_syscall_handler_t	sys_keyctl;
extern l4_syscall_handler_t	sys_mknodat;
extern l4_syscall_handler_t	sys_mkdirat;
extern l4_syscall_handler_t	sys_move_pages;
extern l4_syscall_handler_t	sys_ppoll;
extern l4_syscall_handler_t	sys_pselect6;
extern l4_syscall_handler_t	sys_signalfd;
extern l4_syscall_handler_t	sys_unshare;

extern l4_syscall_handler_t	sys_inotify_init;
extern l4_syscall_handler_t	sys_inotify_add_watch;
extern l4_syscall_handler_t	sys_inotify_rm_watch;

extern l4_syscall_handler_t	sys_waitid;

extern l4_syscall_handler_t	sys_get_robust_list;
extern l4_syscall_handler_t	sys_set_robust_list;

extern l4_syscall_handler_t	sys_utimensat;

extern l4_syscall_handler_t	sys_ni_syscall;

#if BITS_PER_LONG == 32
extern l4_syscall_handler_t	sys_stat64;
extern l4_syscall_handler_t	sys_fstat64;
extern l4_syscall_handler_t	sys_lstat64;
extern l4_syscall_handler_t	sys_llseek;

extern l4_syscall_handler_t	sys_fcntl64;
extern l4_syscall_handler_t	fixup_sys_truncate64;
extern l4_syscall_handler_t	fixup_sys_ftruncate64;
extern l4_syscall_handler_t	sys_getdents64;

extern l4_syscall_handler_t	fixup_sys_pread64;
extern l4_syscall_handler_t	fixup_sys_pwrite64;
extern l4_syscall_handler_t	fixup_sys_readahead;
extern l4_syscall_handler_t	fixup_sys_fadvise64_64;
#endif

#ifdef CONFIG_UID16
extern l4_syscall_handler_t	sys_chown16;
extern l4_syscall_handler_t	sys_lchown16;
extern l4_syscall_handler_t	sys_fchown16;
extern l4_syscall_handler_t	sys_setregid16;
extern l4_syscall_handler_t	sys_setgid16;
extern l4_syscall_handler_t	sys_setreuid16;
extern l4_syscall_handler_t	sys_setuid16;
extern l4_syscall_handler_t	sys_setresuid16;
extern l4_syscall_handler_t	sys_getresuid16;
extern l4_syscall_handler_t	sys_setresgid16;
extern l4_syscall_handler_t	sys_getresgid16;
extern l4_syscall_handler_t	sys_setfsuid16;
extern l4_syscall_handler_t	sys_setfsgid16;
extern l4_syscall_handler_t	sys_getgroups16;
extern l4_syscall_handler_t	sys_setgroups16;
extern l4_syscall_handler_t	sys_getuid16;
extern l4_syscall_handler_t	sys_geteuid16;
extern l4_syscall_handler_t	sys_getgid16;
extern l4_syscall_handler_t	sys_getegid16;
#endif

extern l4_syscall_handler_t	sys_stime;
extern l4_syscall_handler_t	sys32_ptrace;
extern l4_syscall_handler_t	compat_sys_utime;
extern l4_syscall_handler_t	compat_sys_times;
extern l4_syscall_handler_t	compat_sys_ioctl;
extern l4_syscall_handler_t	compat_sys_fcntl;
extern l4_syscall_handler_t	sys_olduname;
extern l4_syscall_handler_t	sys32_sigaction;
extern l4_syscall_handler_t	sys_sgetmask;
extern l4_syscall_handler_t	sys_ssetmask;
extern l4_syscall_handler_t	sys32_sigsuspend;
extern l4_syscall_handler_t	compat_sys_sigpending;
extern l4_syscall_handler_t	compat_sys_setrlimit;
extern l4_syscall_handler_t	compat_sys_getrlimit;
extern l4_syscall_handler_t	compat_sys_getrusage;
extern l4_syscall_handler_t	sys32_gettimeofday;
extern l4_syscall_handler_t	sys32_settimeofday;
extern l4_syscall_handler_t	sys32_readdir;
extern l4_syscall_handler_t	old_mmap;
extern l4_syscall_handler_t	compat_sys_statfs;
extern l4_syscall_handler_t	compat_sys_fstatfs;
extern l4_syscall_handler_t	compat_sys_setitimer;
extern l4_syscall_handler_t	compat_sys_getitimer;
extern l4_syscall_handler_t	compat_sys_newstat;
extern l4_syscall_handler_t	compat_sys_newlstat;
extern l4_syscall_handler_t	compat_sys_newfstat;
extern l4_syscall_handler_t	sys32_wait4;
extern l4_syscall_handler_t	sys32_sysinfo;
extern l4_syscall_handler_t	sys32_ipc;
extern l4_syscall_handler_t	sys32_sigreturn;
extern l4_syscall_handler_t	sys32_newuname;
extern l4_syscall_handler_t	sys32_adjtimex;
extern l4_syscall_handler_t	compat_sys_sigprocmask;
extern l4_syscall_handler_t	sys_bdflush;
extern l4_syscall_handler_t	sys32_personality;
extern l4_syscall_handler_t	sys32_llseek;
extern l4_syscall_handler_t	sys32_getdents;
extern l4_syscall_handler_t	sys32_select;
extern l4_syscall_handler_t	sys32_readv;
extern l4_syscall_handler_t	sys32_writev;
extern l4_syscall_handler_t	sys_cacheflush;
extern l4_syscall_handler_t	sys_cachectl;
extern l4_syscall_handler_t	sys_sysmips;
extern l4_syscall_handler_t	sys32_sysctl;
extern l4_syscall_handler_t	sys32_sched_rr_get_interval;
extern l4_syscall_handler_t	compat_sys_nanosleep;
extern l4_syscall_handler_t	sys_accept;
extern l4_syscall_handler_t	sys_bind;
extern l4_syscall_handler_t	sys_connect;
extern l4_syscall_handler_t	sys_getpeername;
extern l4_syscall_handler_t	sys_getsockname;
extern l4_syscall_handler_t	sys_getsockopt;
extern l4_syscall_handler_t	sys_listen;
extern l4_syscall_handler_t	sys_recv;
extern l4_syscall_handler_t	sys_recvfrom;
extern l4_syscall_handler_t	compat_sys_recvmsg;
extern l4_syscall_handler_t	sys_send;
extern l4_syscall_handler_t	compat_sys_sendmsg;
extern l4_syscall_handler_t	sys_sendto;
extern l4_syscall_handler_t	compat_sys_setsockopt;
extern l4_syscall_handler_t	sys_shutdown;
extern l4_syscall_handler_t	sys_socketpair;
extern l4_syscall_handler_t	sys_nfsservctl;
extern l4_syscall_handler_t	sys32_rt_sigreturn;
extern l4_syscall_handler_t	sys32_rt_sigaction;
extern l4_syscall_handler_t	sys32_rt_sigprocmask;
extern l4_syscall_handler_t	sys32_rt_sigpending;
extern l4_syscall_handler_t	sys32_rt_sigtimedwait;
extern l4_syscall_handler_t	sys32_rt_sigqueueinfo;
extern l4_syscall_handler_t	sys32_rt_sigsuspend;
extern l4_syscall_handler_t	sys32_pread;
extern l4_syscall_handler_t	sys32_pwrite;
extern l4_syscall_handler_t	sys32_sigaltstack;
extern l4_syscall_handler_t	sys32_sendfile;
extern l4_syscall_handler_t	sys32_mmap2;
extern l4_syscall_handler_t	sys32_truncate64;
extern l4_syscall_handler_t	sys32_ftruncate64;
extern l4_syscall_handler_t	compat_sys_fcntl64;
extern l4_syscall_handler_t	sys32_readahead;
extern l4_syscall_handler_t	compat_sys_futex;
extern l4_syscall_handler_t	sys32_sched_setaffinity;
extern l4_syscall_handler_t	sys32_sched_getaffinity;
extern l4_syscall_handler_t	sys_set_tid_address;
extern l4_syscall_handler_t	sys_fadvise64;
extern l4_syscall_handler_t	sys_statfs64;
extern l4_syscall_handler_t	sys_fstatfs64;
extern l4_syscall_handler_t	compat_sys_utimes;
extern l4_syscall_handler_t	sys_setsockopt;
extern l4_syscall_handler_t	sys_recvmsg;
extern l4_syscall_handler_t	sys_sendmsg;
extern l4_syscall_handler_t	sys_msgsnd;
extern l4_syscall_handler_t	sys_msgrcv;
extern l4_syscall_handler_t	sys_msgget;
extern l4_syscall_handler_t	sys_msgctl;
extern l4_syscall_handler_t	sys_shmat;
extern l4_syscall_handler_t	sys_shmdt;
extern l4_syscall_handler_t	sys_shmget;
extern l4_syscall_handler_t	sys_shmctl;
extern l4_syscall_handler_t	sys_semop;
extern l4_syscall_handler_t	sys_semget;
extern l4_syscall_handler_t	sys_semctl;



//32 bit
extern l4_syscall_handler_t	sys32_execve;

// mips
#if 0
extern l4_syscall_handler_t	mips_sys32_syscall;
[__L4_mips_sys32_syscall]	= { mips_sys32_syscall,	{{ .args = 0, .ret_type = L4_RET_TYPE_INT32 }}	},
#endif

#if !defined(__ia32__)
#define sys_stat	sys_newstat
#define sys_fstat	sys_newfstat
#define sys_lstat	sys_newlstat
#endif

l4_syscall_entry_t l4_syscall_table[] = {
	[__L4_sys_exit	    ]	= { sys_exit,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fork	    ]	= { sys_fork,		{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_FORK  }} },
	[__L4_sys_vfork	    ]	= { sys_vfork,		{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_FORK  }} },
	[__L4_sys_execve    ]	= { sys_execve,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_EXEC  }} },
	[__L4_sys_clone	    ]	= { sys_clone,		{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_FORK  }} },
	[__L4_sys_kill	    ]	= { sys_kill,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_tkill	    ]	= { sys_tkill,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_tgkill    ]	= { sys_tgkill,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getpid    ]	= { sys_getpid,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_read	    ]	= { sys_read,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_write	    ]	= { sys_write,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_open	    ]	= { sys_open,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_close	    ]	= { sys_close,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_stat	    ]	= { sys_stat,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fstat	    ]	= { sys_fstat,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lstat	    ]	= { sys_lstat,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lseek	    ]	= { sys_lseek,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mprotect  ]	= { sys_mprotect,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_munmap    ]	= { sys_munmap,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mremap    ]	= { sys_mremap,		{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_msync	    ]   = { sys_msync,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mlock	    ]   = { sys_mlock,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_munlock   ]   = { sys_munlock,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mlockall  ]   = { sys_mlockall,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_munlockall]   = { sys_munlockall,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_poll	    ]	= { sys_poll,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_readv	    ]	= { sys_readv,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_writev    ]	= { sys_writev,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_ioctl	    ]	= { sys_ioctl,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_access    ]	= { sys_access,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_select    ]	= { sys_select,		{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_creat	    ]	= { sys_creat,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_link	    ]	= { sys_link,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_unlink    ]	= { sys_unlink,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_chdir	    ]	= { sys_chdir,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_time	    ]	= { sys_time,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mknod	    ]	= { sys_mknod,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_chmod	    ]	= { sys_chmod,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_chown	    ]	= { sys_chown,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fchmod    ]	= { sys_fchmod,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fchown    ]	= { sys_fchown,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lchown    ]	= { sys_lchown,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_symlink   ]	= { sys_symlink,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_readlink  ]	= { sys_readlink,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_umask	    ]	= { sys_umask,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fchdir    ]	= { sys_fchdir,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_rename    ]	= { sys_rename,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mkdir	    ]	= { sys_mkdir,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_rmdir	    ]	= { sys_rmdir,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_utime	    ]	= { sys_utime,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_utimes    ]	= { sys_utimes,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_wait4	    ]	= { sys_wait4,		{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_uname	    ]	= { sys_newuname,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_restart_syscall ]	= { sys_restart_syscall,    {{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_brk	    ]	= { sys_brk,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_rt_sigaction]	= { sys_rt_sigaction,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_rt_sigprocmask ]  = { sys_rt_sigprocmask,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_rt_sigpending  ]  = { sys_rt_sigpending,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_rt_sigtimedwait]  = { sys_rt_sigtimedwait,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_rt_sigqueueinfo]  = { sys_rt_sigqueueinfo,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sigpending]	= { sys_sigpending,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sigprocmask]	= { sys_sigprocmask,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_fcntl	    ]	= { sys_fcntl,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_flock	    ]	= { sys_flock,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fsync	    ]	= { sys_fsync,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fdatasync ]	= { sys_fdatasync,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_truncate  ]	= { sys_truncate,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_ftruncate ]	= { sys_ftruncate,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getdents  ]	= { sys_getdents,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getcwd    ]	= { sys_getcwd,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_sysinfo   ]	= { sys_sysinfo,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_times	    ]	= { sys_times,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_syslog    ]	= { sys_syslog,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_gettimeofday]	= { sys_gettimeofday,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_settimeofday]	= { sys_settimeofday,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setrlimit ]	= { sys_setrlimit,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getrlimit ]	= { sys_getrlimit,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getrusage ]	= { sys_getrusage,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },

	[__L4_sys_socket    ]	= { sys_socket,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_socketcall]	= { sys_socketcall,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_getuid    ]	= { sys_getuid,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getgid    ]	= { sys_getgid,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_gettid    ]	= { sys_gettid,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setuid    ]	= { sys_setuid,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setgid    ]	= { sys_setgid,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_geteuid   ]	= { sys_geteuid,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getegid   ]	= { sys_getegid,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setpgid   ]	= { sys_setpgid,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getppid   ]	= { sys_getppid,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getpgrp   ]	= { sys_getpgrp,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setsid    ]	= { sys_setsid,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setreuid  ]	= { sys_setreuid,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setregid  ]	= { sys_setregid,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getgroups ]	= { sys_getgroups,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setgroups ]	= { sys_setgroups,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setresuid ]	= { sys_setresuid,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getresuid ]	= { sys_getresuid,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setresgid ]	= { sys_setresgid,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getresgid ]	= { sys_getresgid,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getpgid   ]	= { sys_getpgid,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setfsuid  ]	= { sys_setfsuid,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setfsgid  ]	= { sys_setfsgid,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getsid    ]	= { sys_getsid,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_capget    ]	= { sys_capget,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_capset    ]	= { sys_capset,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_sysctl    ]	= { sys_sysctl,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_timer_create	]   = { sys_timer_create,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_timer_settime	]   = { sys_timer_settime,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_timer_gettime	]   = { sys_timer_gettime,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_timer_getoverrun] = { sys_timer_getoverrun,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_timer_delete	]   = { sys_timer_delete,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_clock_settime	]   = { sys_clock_settime,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_clock_gettime	]   = { sys_clock_gettime,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_clock_getres	]   = { sys_clock_getres,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_clock_nanosleep]  = { sys_clock_nanosleep,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_ustat	    ]	= { sys_ustat,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_statfs    ]	= { sys_statfs,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fstatfs   ]	= { sys_fstatfs,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_statfs64 ]	= { sys_statfs64,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fstatfs64 ]	= { sys_fstatfs64,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sysfs	    ]	= { sys_sysfs,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_chroot    ]	= { sys_chroot,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sync	    ]	= { sys_sync,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },
	[__L4_sys_mount	    ]	= { sys_mount,		{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_umount    ]	= { sys_umount,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_swapon    ]	= { sys_swapon,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_swapoff   ]	= { sys_swapoff,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_reboot    ]	= { sys_reboot,		{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sethostname]	= { sys_sethostname,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setdomainname]= { sys_setdomainname,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_exit_group]	= { sys_exit_group,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_dup	    ]	= { sys_dup,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_dup2	    ]	= { sys_dup2,		{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_pause	    ]	= { sys_pause,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_nanosleep ]	= { sys_nanosleep,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getitimer ]	= { sys_getitimer,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_alarm	    ]	= { sys_alarm,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setitimer ]	= { sys_setitimer,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_acct		]   = { sys_acct,		{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getpriority	]   = { sys_getpriority,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setpriority	]   = { sys_setpriority,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_setparam]   = { sys_sched_setparam,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_getparam]   = { sys_sched_getparam,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_setscheduler]	= { sys_sched_setscheduler,   {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_getscheduler]	= { sys_sched_getscheduler,   {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_yield]	= { sys_sched_yield,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_get_priority_max]   = { sys_sched_get_priority_max,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_get_priority_min]   = { sys_sched_get_priority_min,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_rr_get_interval	]   = { sys_sched_rr_get_interval,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_adjtimex	]   = { sys_adjtimex,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_nice	]   = { sys_nice,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_uselib	]   = { sys_uselib,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_init_module	]   = { sys_init_module,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_delete_module	]   = { sys_delete_module,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setxattr	]   = { sys_setxattr,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lsetxattr	]   = { sys_lsetxattr,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fsetxattr	]   = { sys_fsetxattr,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getxattr	]   = { sys_getxattr,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lgetxattr	]   = { sys_lgetxattr,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fgetxattr	]   = { sys_fgetxattr,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_listxattr	]   = { sys_listxattr,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
        [__L4_sys_llistxattr	]   = { sys_llistxattr,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_flistxattr	]   = { sys_flistxattr,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
        [__L4_sys_removexattr	]   = { sys_removexattr,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lremovexattr	]   = { sys_lremovexattr,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fremovexattr	]   = { sys_fremovexattr,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_mq_open	]   = { sys_mq_open,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
        [__L4_sys_mq_unlink	]   = { sys_mq_unlink,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mq_timedsend	]   = { sys_mq_timedsend,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
        [__L4_sys_mq_timedreceive]  = { sys_mq_timedreceive,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mq_notify	]   = { sys_mq_notify,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mq_getsetattr	]   = { sys_mq_getsetattr,  {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_vhangup   ]	= { sys_vhangup,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_quotactl  ]	= { sys_quotactl,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_prctl	]	= { sys_prctl,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_sendfile  ]	= { sys_sendfile,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sendfile64]	= { sys_sendfile64,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_pivot_root]	= { sys_pivot_root,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mincore   ]	= { sys_mincore ,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_madvise   ]	= { sys_madvise,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_futex		]   = { sys_futex,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_setaffinity ]   = { sys_sched_setaffinity,  {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_sched_getaffinity ]   = { sys_sched_getaffinity,  {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
        [__L4_sys_io_setup	]   = { sys_io_setup,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_io_destroy	]   = { sys_io_destroy,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
        [__L4_sys_io_getevents	]   = { sys_io_getevents,   {{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_io_submit	]   = { sys_io_submit,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_io_cancel	]   = { sys_io_cancel,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

        [__L4_sys_lookup_dcookie    ]   = { sys_lookup_dcookie,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_epoll_create	    ]   = { sys_epoll_create,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
        [__L4_sys_epoll_ctl	]   = { sys_epoll_ctl,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_epoll_wait	]   = { sys_epoll_wait,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_remap_file_pages  ]   = { sys_remap_file_pages,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_pciconfig_read    ]   = { sys_pciconfig_read,	    {{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_pciconfig_write   ]   = { sys_pciconfig_write,    {{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_faccessat	] = { sys_faccessat,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fchmodat	] = { sys_fchmodat,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fchownat	] = { sys_fchownat,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
/* XXX different system call nr? */
#ifdef __ARCH_WANT_STAT64
	[__L4_sys_fstatat	] = { sys_fstatat64,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#endif
#ifdef __ARCH_WANT_NEWFSTATAT
	[__L4_sys_fstatat	] = { sys_newfstatat,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#endif
#ifdef __ARCH_WANT_SYS_FADVISE64
#if BITS_PER_LONG == 64
	[__L4_sys_fadvise64	] = { sys_fadvise64,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#elif BITS_PER_LONG == 32
	[__L4_sys_fadvise64	] = { sys_fadvise64,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#else
#error "unknown architecture wordsize"
#endif
#endif
	[__L4_sys_futimesat	] = { sys_futimesat,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_linkat	] = { sys_linkat,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_openat	] = { sys_openat,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_readlinkat	] = { sys_readlinkat,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_renameat	] = { sys_renameat,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_symlinkat	] = { sys_symlinkat,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_unlinkat	] = { sys_unlinkat,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_tee		] = { sys_tee,		{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_splice	] = { sys_splice,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_vmsplice	] = { sys_vmsplice,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_inotify_init	] = { sys_inotify_init,		{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_inotify_add_watch	] = { sys_inotify_add_watch,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_inotify_rm_watch	] = { sys_inotify_rm_watch,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_waitid	] = { sys_waitid,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_get_robust_list	] = { sys_get_robust_list,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_set_robust_list	] = { sys_set_robust_list,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_utimensat	] = { sys_utimensat,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_create_module	] = { sys_ni_syscall,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_init_module	] = { sys_init_module,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_delete_module	] = { sys_delete_module,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_get_kernel_syms ] = { sys_ni_syscall, {{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_timerfd ] = { sys_timerfd,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_add_key ] = { sys_add_key,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#if 0	/*notyet*/
	[__L4_sys_epoll_pwait ] = { sys_epoll_pwait,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fallocate ] = { sys_fallocate,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#endif
	[__L4_sys_getcpu ] = { sys_getcpu,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_get_mempolicy ] = { sys_get_mempolicy,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_get_mempolicy ] = { sys_set_mempolicy,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_ioprio_get ] = { sys_ioprio_get,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_ioprio_set ] = { sys_ioprio_set,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getcpu ] = { sys_getcpu,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_keyctl ] = { sys_keyctl,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#if 0	/*notyet*/
	[__L4_sys_migrate_pages ] = { sys_migrate_pages,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#endif
	[__L4_sys_mkdirat ] = { sys_mkdirat,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_mknodat ] = { sys_mknodat,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_move_pages ] = { sys_move_pages,	{{ .args = 7/*XXX*/, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#if 0	/*notyet*/
	[__L4_sys_ppoll ] = { sys_ppoll,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_pselect6 ] = { sys_pselect6,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#endif
	[__L4_sys_signalfd ] = { sys_signalfd,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_unshare ] = { sys_unshare,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

#if BITS_PER_LONG == 64
	[__L4_sys_pread64   ]	= { sys_pread64,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_pwrite64  ]	= { sys_pwrite64,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_readahead ]	= { sys_readahead,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fadvise64_64] = { sys_fadvise64_64,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#endif

#if BITS_PER_LONG == 32
	[__L4_sys_stat64]	= { sys_stat64,		{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fstat64]	= { sys_fstat64,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lstat64]	= { sys_lstat64,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys__llseek]	= { sys_llseek,		{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_fcntl64]	= { sys_fcntl64,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_truncate64]	= { fixup_sys_truncate64,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_ftruncate64]	= { fixup_sys_ftruncate64,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getdents64]	= { sys_getdents64,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },

	[__L4_sys_pread64   ]	= { fixup_sys_pread64,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_pwrite64  ]	= { fixup_sys_pwrite64,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_readahead ]	= { fixup_sys_readahead,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fadvise64_64] = { fixup_sys_fadvise64_64,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },
#endif

#ifdef CONFIG_UID16
	[__L4_sys_chown16	]	= { sys_chown16,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_lchown16	]	= { sys_lchown16,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_fchown16	]	= { sys_fchown16,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setregid16	]	= { sys_setregid16,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setgid16	]	= { sys_setgid16,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setreuid16	]	= { sys_setreuid16,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setuid16	]	= { sys_setuid16,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setresuid16	]	= { sys_setresuid16,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getresuid16	]	= { sys_getresuid16,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setresgid16	]	= { sys_setresgid16,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getresgid16	]	= { sys_getresgid16,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setfsuid16	]	= { sys_setfsuid16,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setfsgid16	]	= { sys_setfsgid16,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getgroups16	]	= { sys_getgroups16,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_setgroups16	]	= { sys_setgroups16,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getuid16	]	= { sys_getuid16,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_geteuid16	]	= { sys_geteuid16,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getgid16	]	= { sys_getgid16,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
	[__L4_sys_getegid16	]	= { sys_getegid16,	{{ .args = 0, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },
#endif

	ARCH_SPECIFIC_SYSCALL_LINKAGE
#if 0
	sys_waitpid	3
	sys32_execve	0
	sys_lseek	3
	sys_stime	1
	sys32_ptrace	4
	compat_sys_utime	2			/* 4030 */
	compat_sys_times	1
	compat_sys_ioctl	3
	compat_sys_fcntl	3		/* 4055 */
	sys_olduname	1
	sys_ustat	2
	sys32_sigaction	3
	sys_sgetmask	0
	sys_ssetmask	1
	sys32_sigsuspend	0
	compat_sys_sigpending	1
	compat_sys_setrlimit	2		/* 4075 */
	compat_sys_getrlimit	2
	compat_sys_getrusage	2
	sys32_gettimeofday 2
	sys32_settimeofday 2
	sys_swapon	2
	sys32_readdir	3
	old_mmap	6			/* 4090 */
	sys_fchmod	2
	compat_sys_statfs	2
	compat_sys_fstatfs	2		/* 4100 */
	compat_sys_setitimer	3
	compat_sys_getitimer	2	/* 4105 */
	compat_sys_newlstat	2
	compat_sys_newfstat	2
	sys32_wait4		4
	sys32_sysinfo		1
	sys32_ipc		6
	sys32_sigreturn	0
	sys32_newuname	1
	sys32_adjtimex	1
	sys_mprotect	3			/* 4125 */
	compat_sys_sigprocmask	3
	sys_bdflush	2
	sys32_personality	1
	sys32_getdents	3
	sys32_select	5
	sys32_readv	3			/* 4145 */
	sys32_writev	3
	sys_cacheflush	3
	sys_cachectl	3
	sys_sysmips	4
	sys32_sysctl	1
	sys_mlock	2
	sys_munlock	2			/* 4155 */
	sys_mlockall	1
	sys_munlockall	0
	sys32_sched_rr_get_interval 2		/* 4165 */
	compat_sys_nanosleep	2
	sys_accept	3
	sys_bind	3
	sys_connect	3			/* 4170 */
	sys_getpeername	3
	sys_getsockname	3
	sys_getsockopt	5
	sys_listen	2
	sys_recv	4			/* 4175 */
	sys_recvfrom	6
	compat_sys_recvmsg	3
	sys_send	4
	compat_sys_sendmsg	3
	sys_sendto	6			/* 4180 */
	compat_sys_setsockopt	5
	sys_shutdown	2
	sys_socket	3
	sys_socketpair	4
	sys_nfsservctl	3
	sys32_rt_sigreturn 0
	sys32_rt_sigaction 4
	sys32_rt_sigprocmask 4			/* 4195 */
	sys32_rt_sigpending 2
	sys32_rt_sigtimedwait 4
	sys32_rt_sigqueueinfo 3
	sys32_rt_sigsuspend 0
	sys32_pread	6			/* 4200 */
	sys32_pwrite	6
	sys32_sigaltstack	0
	sys32_sendfile	4
	sys32_mmap2	6			/* 4210 */
	sys32_truncate64	4
	sys32_ftruncate64	4
	compat_sys_fcntl64	3		/* 4220 */
	sys32_readahead	5
	compat_sys_futex	5
	sys32_sched_setaffinity	3
	sys32_sched_getaffinity	3		/* 4240 */
	sys_set_tid_address	1
	sys_fadvise64		6
	sys_statfs64		3		/* 4255 */
	sys_fstatfs64		2
	sys_timer_create	3
	sys_timer_settime	4
	sys_timer_gettime	2
	sys_timer_getoverrun	1		/* 4260 */
	sys_timer_delete	1
	sys_clock_settime	2
	sys_clock_gettime	2
	sys_clock_getres	2
	sys_clock_nanosleep	4		/* 4265 */
	compat_sys_utimes	2
#endif
//	ARCH_SPECIFIC_SYSCALLS
};

