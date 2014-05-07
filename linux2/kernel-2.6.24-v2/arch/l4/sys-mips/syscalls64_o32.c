#include <l4.h>

#include <linux/kernel.h>

#include <asm/syscalls.h>

#ifndef CONFIG_MIPS32_O32

l4_mips_abi_t l4_mips_abi_o32 = {
	0, 0, 0, 0
};

#else /* CONFIG_MIPS32_O32 */

#include INC_SYSTEM(sgidefs.h)

#undef _MIPS_SIM
#define _MIPS_SIM	_MIPS_SIM_ABI32

#include INC_SYSTEM(unistd.h)

#if __NR_Linux != __NR_O32_Linux
#error mips include broken
#endif

l4_mips_abi_syscalls_t	l4_mips_abi_o32_map[__NR_O32_Linux_syscalls+1] = {
	[__NR_exit	- __NR_Linux] = { __L4_sys_exit },
	[__NR_fork	- __NR_Linux] = { __L4_sys_fork },

	[__NR_fcntl64	- __NR_Linux] = { __L4_compat_sys_fcntl64 },
	[__NR_getdents64 - __NR_Linux] = { __L4_sys_getdents64 },

//	[__NR_vfork	- __NR_Linux] = {__L4_sys_vfork },	    -- mips does not have
	[__NR_execve	- __NR_Linux] = { __L4_sys32_execve },
	[__NR_clone	- __NR_Linux] = { __L4_sys_clone },
	[__NR_kill	- __NR_Linux] = { __L4_sys_kill },
	[__NR_tkill	- __NR_Linux] = { __L4_sys_tkill },
	[__NR_tgkill	- __NR_Linux] = { __L4_sys_tgkill },
	[__NR_getpid	- __NR_Linux] = { __L4_sys_getpid },

	[__NR_read	- __NR_Linux] = { __L4_sys_read },
	[__NR_write	- __NR_Linux] = { __L4_sys_write },
	[__NR_open	- __NR_Linux] = { __L4_sys_open },
	[__NR_close	- __NR_Linux] = { __L4_sys_close },

	[__NR_stat	- __NR_Linux] = { __L4_compat_sys_newstat },
	[__NR_fstat	- __NR_Linux] = { __L4_compat_sys_newfstat },
	[__NR_lstat	- __NR_Linux] = { __L4_compat_sys_newlstat },
	[__NR_stat64	- __NR_Linux] = { __L4_sys_newstat },
	[__NR_fstat64	- __NR_Linux] = { __L4_sys_newfstat },
	[__NR_lstat64	- __NR_Linux] = { __L4_sys_newlstat },
	[__NR_statfs   - __NR_Linux] = { __L4_compat_sys_statfs},
	[__NR_fstatfs  - __NR_Linux] = { __L4_compat_sys_fstatfs},
	[__NR_statfs64	- __NR_Linux] = { __L4_sys_statfs64 },
	[__NR_fstatfs64	- __NR_Linux] = { __L4_sys_fstatfs64 },
	[__NR_lseek	- __NR_Linux] = { __L4_sys_lseek },
	[__NR__llseek  - __NR_Linux] = { __L4_sys32__llseek },
	[__NR_mmap	- __NR_Linux] = { __L4_old_mmap },
	[__NR_mmap2	- __NR_Linux] = { __L4_sys32_mmap2 },
	[__NR_mprotect	- __NR_Linux] = { __L4_sys_mprotect },
	[__NR_munmap	- __NR_Linux] = { __L4_sys_munmap },
	[__NR_mremap	- __NR_Linux] = { __L4_sys_mremap },
	[__NR_msync	- __NR_Linux] = { __L4_sys_msync },
	[__NR_mlock	- __NR_Linux] = { __L4_sys_mlock },
	[__NR_munlock	- __NR_Linux] = { __L4_sys_munlock },
	[__NR_mlockall	- __NR_Linux] = { __L4_sys_mlockall },
	[__NR_munlockall	- __NR_Linux] = { __L4_sys_munlockall },

	[__NR_poll	- __NR_Linux] = { __L4_sys_poll },
//	[__NR_pread64	- __NR_Linux] = { __L4_sys32_pread64 },
//	[__NR_pwrite64	- __NR_Linux] = { __L4_sys32_pwrite64 },
	[__NR_readv	- __NR_Linux] = { __L4_compat_sys_readv },
	[__NR_writev	- __NR_Linux] = { __L4_compat_sys_writev },
	[__NR_ioctl	- __NR_Linux] = { __L4_compat_sys_ioctl },
	[__NR_access	- __NR_Linux] = { __L4_sys_access },
	[__NR_pipe	- __NR_Linux] = { __L4_sys_pipe },
//	[__NR__newselect- __NR_Linux] = { __L4_sys_select },

	[__NR_creat	- __NR_Linux] = { __L4_sys_creat },
	[__NR_link	- __NR_Linux] = { __L4_sys_link },
	[__NR_unlink	- __NR_Linux] = { __L4_sys_unlink },
	[__NR_chdir	- __NR_Linux] = { __L4_sys_chdir },
	[__NR_time	- __NR_Linux] = { __L4_sys_time },
	[__NR_mknod	- __NR_Linux] = { __L4_sys_mknod },
	[__NR_chmod	- __NR_Linux] = { __L4_sys_chmod },
	[__NR_chown	- __NR_Linux] = { __L4_sys_chown },
	[__NR_fchmod	- __NR_Linux] = { __L4_sys_fchmod },
	[__NR_fchown	- __NR_Linux] = { __L4_sys_fchown },
	[__NR_lchown	- __NR_Linux] = { __L4_sys_lchown },
	[__NR_symlink	- __NR_Linux] = { __L4_sys_symlink },
	[__NR_readlink	- __NR_Linux] = { __L4_sys_readlink },
	[__NR_umask	- __NR_Linux] = { __L4_sys_umask },
	[__NR_fchdir	- __NR_Linux] = { __L4_sys_fchdir },
	[__NR_rename	- __NR_Linux] = { __L4_sys_rename },
	[__NR_mkdir	- __NR_Linux] = { __L4_sys_mkdir },
	[__NR_rmdir	- __NR_Linux] = { __L4_sys_rmdir },
//	[__NR_readdir	- __NR_Linux] = { __L4_sys32_readdir },
	[__NR_utime	- __NR_Linux] = { __L4_sys_utime },
//	[__NR_utimes	- __NR_Linux] = { __L4_sys_utimes },

	[__NR_waitpid	- __NR_Linux] = { __L4_sys32_waitpid },
	[__NR_wait4	- __NR_Linux] = { __L4_compat_sys_wait4 },
	[__NR_uname	- __NR_Linux] = { __L4_sys32_newuname },

	[__NR_restart_syscall - __NR_Linux] = { __L4_sys_restart_syscall },

	[__NR_brk	- __NR_Linux] = { __L4_sys_brk },
	[__NR_rt_sigaction	- __NR_Linux] = { __L4_sys32_rt_sigaction },
	[__NR_rt_sigprocmask	- __NR_Linux] = { __L4_sys32_rt_sigprocmask },
	[__NR_rt_sigreturn	- __NR_Linux] = { __L4_sys32_rt_sigreturn },
	[__NR_rt_sigpending	- __NR_Linux] = { __L4_sys32_rt_sigpending },
	[__NR_rt_sigtimedwait	- __NR_Linux] = { __L4_sys32_rt_sigtimedwait },
	[__NR_rt_sigqueueinfo	- __NR_Linux] = { __L4_sys32_rt_sigqueueinfo },
	[__NR_rt_sigsuspend	- __NR_Linux] = { __L4_sys32_rt_sigsuspend },
//	[__NR_sigaltstack	- __NR_Linux] = { __L4_sys_sigaltstack },
//	[__NR_signal		- __NR_Linux] = { __L4_sys_signal },
//	[__NR_sigaction		- __NR_Linux] = { __L4_sys_sigaction },
	[__NR_sigprocmask	- __NR_Linux] = { __L4_sys32_sigprocmask },
	[__NR_sigreturn		- __NR_Linux] = { __L4_sys32_sigreturn },
	[__NR_sigpending	- __NR_Linux] = { __L4_sys32_sigpending },
	[__NR_sigsuspend	- __NR_Linux] = { __L4_sys32_sigsuspend },

	[__NR_fcntl	- __NR_Linux] = { __L4_compat_sys_fcntl },
	[__NR_flock	- __NR_Linux] = { __L4_sys_flock },
	[__NR_fsync	- __NR_Linux] = { __L4_sys_fsync },
	[__NR_fdatasync	- __NR_Linux] = { __L4_sys_fdatasync },
	[__NR_truncate	- __NR_Linux] = { __L4_sys_truncate },
	[__NR_ftruncate	- __NR_Linux] = { __L4_sys_ftruncate },
	[__NR_getpriority - __NR_Linux] = { __L4_sys_getpriority },
	[__NR_setpriority - __NR_Linux] = { __L4_sys_setpriority },
	[__NR_getdents	- __NR_Linux] = { __L4_sys32_getdents },
	[__NR_getcwd	- __NR_Linux] = { __L4_sys_getcwd },

//	[__NR_ipc	- __NR_Linux] = { __L4_sys32_ipc },
	[__NR_sysinfo	- __NR_Linux] = { __L4_sys32_sysinfo },
	[__NR_times	- __NR_Linux] = { __L4_compat_sys_times },
	[__NR_syslog	- __NR_Linux] = { __L4_sys_syslog },

	[__NR_gettimeofday	- __NR_Linux] = { __L4_sys32_gettimeofday },
//	[__NR_settimeofday	- __NR_Linux] = { __L4_sys32_settimeofday },
	[__NR_setrlimit	- __NR_Linux] = { __L4_compat_sys_setrlimit },
	[__NR_getrlimit	- __NR_Linux] = { __L4_compat_sys_getrlimit },
	[__NR_getrusage	- __NR_Linux] = { __L4_compat_sys_getrusage },

	[__NR_socket	- __NR_Linux] = { __L4_sys_socket },
//	[__NR_socketcall- __NR_Linux] = { __L4_sys_socketcall },	-- mips64 does not have

	[__NR_getuid 	- __NR_Linux] = { __L4_sys_getuid },
	[__NR_getgid   	- __NR_Linux] = { __L4_sys_getgid },
	[__NR_gettid 	- __NR_Linux] = { __L4_sys_gettid },
	[__NR_setuid   	- __NR_Linux] = { __L4_sys_setuid },
	[__NR_setgid   	- __NR_Linux] = { __L4_sys_setgid },
	[__NR_geteuid  	- __NR_Linux] = { __L4_sys_geteuid },
	[__NR_getegid  	- __NR_Linux] = { __L4_sys_getegid },
	[__NR_setpgid  	- __NR_Linux] = { __L4_sys_setpgid },
	[__NR_getppid  	- __NR_Linux] = { __L4_sys_getppid },
	[__NR_getpgrp  	- __NR_Linux] = { __L4_sys_getpgrp },
	[__NR_setsid   	- __NR_Linux] = { __L4_sys_setsid },
	[__NR_setreuid 	- __NR_Linux] = { __L4_sys_setreuid },
	[__NR_setregid 	- __NR_Linux] = { __L4_sys_setregid },
	[__NR_getgroups	- __NR_Linux] = { __L4_sys_getgroups },
	[__NR_setgroups	- __NR_Linux] = { __L4_sys_setgroups },
	[__NR_setresuid	- __NR_Linux] = { __L4_sys_setresuid },
	[__NR_getresuid	- __NR_Linux] = { __L4_sys_getresuid },
	[__NR_setresgid	- __NR_Linux] = { __L4_sys_setresgid },
	[__NR_getresgid	- __NR_Linux] = { __L4_sys_getresgid },
	[__NR_getpgid  	- __NR_Linux] = { __L4_sys_getpgid },
	[__NR_setfsuid 	- __NR_Linux] = { __L4_sys_setfsuid },
	[__NR_setfsgid 	- __NR_Linux] = { __L4_sys_setfsgid },
	[__NR_getsid   	- __NR_Linux] = { __L4_sys_getsid },
	[__NR_capget   	- __NR_Linux] = { __L4_sys_capget },
	[__NR_capset   	- __NR_Linux] = { __L4_sys_capset },

//	[__NR__sysctl	- __NR_Linux] = { __L4_sys_sysctl },

	[__NR_chroot	- __NR_Linux] = { __L4_sys_chroot },
	[__NR_sync	- __NR_Linux] = { __L4_sys_sync },
	[__NR_mount	- __NR_Linux] = { __L4_sys_mount },
	[__NR_umount2	- __NR_Linux] = { __L4_sys_umount },

	[__NR_swapon	- __NR_Linux] = { __L4_sys_swapon },
	[__NR_swapoff	- __NR_Linux] = { __L4_sys_swapoff },
	[__NR_reboot	- __NR_Linux] = { __L4_sys_reboot },
	[__NR_sethostname	- __NR_Linux] = { __L4_sys_sethostname },
	[__NR_setdomainname	- __NR_Linux] = { __L4_sys_setdomainname },
	[__NR_exit_group- __NR_Linux] = { __L4_sys_exit_group },

	[__NR_acct	- __NR_Linux] = { __L4_sys_acct },

	[__NR_sched_setparam	- __NR_Linux] = { __L4_sys_sched_setparam },
	[__NR_sched_getparam	- __NR_Linux] = { __L4_sys_sched_getparam },
	[__NR_sched_setscheduler- __NR_Linux] = { __L4_sys_sched_setscheduler },
	[__NR_sched_getscheduler- __NR_Linux] = { __L4_sys_sched_getscheduler },
	[__NR_sched_yield	- __NR_Linux] = { __L4_sys_sched_yield },
	[__NR_sched_get_priority_max	- __NR_Linux] = { __L4_sys_sched_get_priority_max },
	[__NR_sched_get_priority_min	- __NR_Linux] = { __L4_sys_sched_get_priority_min },
	[__NR_sched_rr_get_interval	- __NR_Linux] = { __L4_sys_sched_rr_get_interval },

	[__NR_adjtimex	- __NR_Linux] = { __L4_sys32_adjtimex },
	[__NR_nice	- __NR_Linux] = { __L4_sys_nice },

	[__NR_uselib	- __NR_Linux] = { __L4_sys_uselib },

	[__NR_dup	- __NR_Linux] = { __L4_sys_dup },
	[__NR_dup2	- __NR_Linux] = { __L4_sys_dup2 },
	[__NR_pause	- __NR_Linux] = { __L4_sys_pause },
	[__NR_nanosleep	- __NR_Linux] = { __L4_compat_sys_nanosleep },
//	[__NR_getitimer	- __NR_Linux] = { __L4_sys_getitimer },
	[__NR_alarm	- __NR_Linux] = { __L4_sys_alarm },
//	[__NR_setitimer	- __NR_Linux] = { __L4_sys_setitimer },

	[__NR_init_module	- __NR_Linux] = { __L4_sys_init_module },
	[__NR_delete_module	- __NR_Linux] = { __L4_sys_delete_module  },

	[__NR_stime	- __NR_Linux] = { __L4_sys_stime },

	[__NR_setxattr	- __NR_Linux] = { __L4_sys_setxattr },
	[__NR_lsetxattr	- __NR_Linux] = { __L4_sys_lsetxattr },
	[__NR_fsetxattr	- __NR_Linux] = { __L4_sys_fsetxattr },
	[__NR_getxattr	- __NR_Linux] = { __L4_sys_getxattr },
	[__NR_lgetxattr	- __NR_Linux] = { __L4_sys_lgetxattr },
	[__NR_fgetxattr	- __NR_Linux] = { __L4_sys_fgetxattr },
	[__NR_listxattr	- __NR_Linux] = { __L4_sys_listxattr },
	[__NR_llistxattr	- __NR_Linux] = { __L4_sys_llistxattr },
	[__NR_flistxattr	- __NR_Linux] = { __L4_sys_flistxattr },
	[__NR_removexattr	- __NR_Linux] = { __L4_sys_removexattr },
	[__NR_lremovexattr	- __NR_Linux] = { __L4_sys_lremovexattr },
	[__NR_fremovexattr	- __NR_Linux] = { __L4_sys_fremovexattr	},

	[__NR_bind		- __NR_Linux] = { __L4_sys_bind	 },
	[__NR_connect		- __NR_Linux] = { __L4_sys_connect },
	[__NR_listen		- __NR_Linux] = { __L4_sys_listen },
	[__NR_accept		- __NR_Linux] = { __L4_sys_accept },
	[__NR_getsockname	- __NR_Linux] = { __L4_sys_getsockname },
	[__NR_getpeername	- __NR_Linux] = { __L4_sys_getpeername },
	[__NR_socketpair	- __NR_Linux] = { __L4_sys_socketpair },
	[__NR_send		- __NR_Linux] = { __L4_sys_send	 },
	[__NR_sendto		- __NR_Linux] = { __L4_sys_sendto },
	[__NR_recv		- __NR_Linux] = { __L4_sys_recv },
	[__NR_recvfrom		- __NR_Linux] = { __L4_sys_recvfrom },
	[__NR_shutdown		- __NR_Linux] = { __L4_sys_shutdown },
	[__NR_setsockopt	- __NR_Linux] = { __L4_compat_setsockopt },
	[__NR_getsockopt	- __NR_Linux] = { __L4_sys_getsockopt },
	[__NR_sendmsg		- __NR_Linux] = { __L4_compat_sendmsg },
	[__NR_recvmsg		- __NR_Linux] = { __L4_compat_recvmsg },

//	[__NR_mq_open	- __NR_Linux] = { __L4_compat_mq_open },
	[__NR_mq_unlink	- __NR_Linux] = { __L4_sys_mq_unlink },
//	[__NR_mq_timedsend	- __NR_Linux] = { __L4_compat_mq_timedsend	},
//	[__NR_mq_timedreceive   - __NR_Linux] = { __L4_compat_mq_timedreceive },
//	[__NR_mq_notify		- __NR_Linux] = { __L4_compat_mq_notify },
//	[__NR_mq_getsetattr	- __NR_Linux] = { __L4_compat_mq_getsetattr },

	[__NR_vhangup	- __NR_Linux] = { __L4_sys_vhangup },
	[__NR_quotactl	- __NR_Linux] = { __L4_sys_quotactl },
	[__NR_nfsservctl	- __NR_Linux] = { __L4_sys_nfsservctl },
	[__NR_prctl	- __NR_Linux] = { __L4_sys_prctl },

//	[__NR_sendfile	- __NR_Linux] = { __L4_sys32_sendfile },
	[__NR_sendfile64	- __NR_Linux] = { __L4_sys_sendfile64 },

	[__NR_pivot_root- __NR_Linux] = { __L4_sys_pivot_root },
	[__NR_mincore	- __NR_Linux] = { __L4_sys_mincore },
	[__NR_madvise	- __NR_Linux] = { __L4_sys_madvise },
//	[__NR_readahead	- __NR_Linux] = { __L4_sys32_readahead },

//	[__NR_futex	- __NR_Linux] = { __L4_compat_futex },
//	[__NR_sched_setaffinity	- __NR_Linux] = { __L4_compat_sched_setaffinity },
//	[__NR_sched_getaffinity	- __NR_Linux] = { __L4_compat_sched_getaffinity },
	[__NR_io_setup	- __NR_Linux] = { __L4_sys_io_setup },
	[__NR_io_destroy	- __NR_Linux] = { __L4_sys_io_destroy },
	[__NR_io_getevents	- __NR_Linux] = { __L4_sys_io_getevents },
	[__NR_io_submit	- __NR_Linux] = { __L4_sys_io_submit },
	[__NR_io_cancel	- __NR_Linux] = { __L4_sys_io_cancel },

	[__NR_lookup_dcookie	- __NR_Linux] = { __L4_sys_lookup_dcookie },
	[__NR_epoll_create	- __NR_Linux] = { __L4_sys_epoll_create },
	[__NR_epoll_ctl	- __NR_Linux] = { __L4_sys_epoll_ctl },
	[__NR_epoll_wait	- __NR_Linux] = { __L4_sys_epoll_wait },
	[__NR_remap_file_pages	- __NR_Linux] = { __L4_sys_remap_file_pages },
};

l4_mips_abi_t l4_mips_abi_o32 = {
	(void*) &l4_mips_abi_o32_map,
	__NR_Linux,
	__NR_Linux + __NR_O32_Linux_syscalls,
	_MIPS_SIM_ABI32,
	0
};

#endif /* CONFIG_MIPS32_O32 */
