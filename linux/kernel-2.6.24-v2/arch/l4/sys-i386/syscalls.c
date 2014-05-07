#include <l4.h>


#include <asm/syscalls.h>
#include INC_SYSTEM(unistd.h)

#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/mman.h>

#define __NR_Linux	0

extern long sys_mmap2(unsigned long addr, unsigned long len, unsigned long prot,
		     unsigned long flags, unsigned long fd, unsigned long pgoff);


unsigned int l4_i386_abi_syscall_start = 0;
unsigned int l4_i386_abi_syscall_end = NR_syscalls;

l4_i386_abi_syscalls_t l4_i386_abi_syscalls[NR_syscalls] = {
	[__NR_exit	- __NR_Linux] = { __L4_sys_exit },
	[__NR_fork	- __NR_Linux] = { __L4_sys_fork },
	[__NR_vfork	- __NR_Linux] = { __L4_sys_vfork },
	[__NR_execve	- __NR_Linux] = { __L4_sys_execve },
	[__NR_clone	- __NR_Linux] = { __L4_sys_clone },
	[__NR_kill	- __NR_Linux] = { __L4_sys_kill },
	[__NR_tkill	- __NR_Linux] = { __L4_sys_tkill },
	[__NR_tgkill	- __NR_Linux] = { __L4_sys_tgkill },
	[__NR_getpid	- __NR_Linux] = { __L4_sys_getpid },

	[__NR_read	- __NR_Linux] = { __L4_sys_read },
	[__NR_write	- __NR_Linux] = { __L4_sys_write },
	[__NR_open	- __NR_Linux] = { __L4_sys_open },
	[__NR_close	- __NR_Linux] = { __L4_sys_close },
	[__NR_stat	- __NR_Linux] = { __L4_sys_stat },
	[__NR_stat64	- __NR_Linux] = { __L4_sys_stat64 },
	[__NR_fstat	- __NR_Linux] = { __L4_sys_fstat },
	[__NR_fstat64	- __NR_Linux] = { __L4_sys_fstat64 },
	[__NR_lstat	- __NR_Linux] = { __L4_sys_lstat },
	[__NR_lstat64	- __NR_Linux] = { __L4_sys_lstat64 },
	[__NR_lseek	- __NR_Linux] = { __L4_sys_lseek },
	[__NR__llseek	- __NR_Linux] = { __L4_sys__llseek },
	[__NR_mmap	- __NR_Linux] = { __L4_old_mmap },
	[__NR_mmap2	- __NR_Linux] = { __L4_sys_mmap },
	[__NR_mprotect	- __NR_Linux] = { __L4_sys_mprotect },
	[__NR_munmap	- __NR_Linux] = { __L4_sys_munmap },
	[__NR_mremap	- __NR_Linux] = { __L4_sys_mremap },
	[__NR_msync	- __NR_Linux] = { __L4_sys_msync },
	[__NR_mlock	- __NR_Linux] = { __L4_sys_mlock },
	[__NR_munlock	- __NR_Linux] = { __L4_sys_munlock },
	[__NR_mlockall	- __NR_Linux] = { __L4_sys_mlockall },
	[__NR_munlockall	- __NR_Linux] = { __L4_sys_munlockall },

	[__NR_poll	- __NR_Linux] = { __L4_sys_poll },
	[__NR_pread64	- __NR_Linux] = { __L4_sys_pread64 },
	[__NR_pwrite64	- __NR_Linux] = { __L4_sys_pwrite64 },
	[__NR_readv	- __NR_Linux] = { __L4_sys_readv },
	[__NR_writev	- __NR_Linux] = { __L4_sys_writev },
	[__NR_ioctl	- __NR_Linux] = { __L4_sys_ioctl },
	[__NR_access	- __NR_Linux] = { __L4_sys_access },
	[__NR_pipe	- __NR_Linux] = { __L4_sys_pipe },
	[__NR__newselect- __NR_Linux] = { __L4_sys_select },
	[__NR_select	- __NR_Linux] = { __L4_old_select },

	[__NR_creat	- __NR_Linux] = { __L4_sys_creat },
	[__NR_link	- __NR_Linux] = { __L4_sys_link },
	[__NR_unlink	- __NR_Linux] = { __L4_sys_unlink },
	[__NR_chdir	- __NR_Linux] = { __L4_sys_chdir },
	[__NR_time	- __NR_Linux] = { __L4_sys_time },
	[__NR_mknod	- __NR_Linux] = { __L4_sys_mknod },
	[__NR_chmod	- __NR_Linux] = { __L4_sys_chmod },
	[__NR_fchmod	- __NR_Linux] = { __L4_sys_fchmod },
	[__NR_chown32	- __NR_Linux] = { __L4_sys_chown },
	[__NR_fchown32	- __NR_Linux] = { __L4_sys_fchown },
	[__NR_lchown32	- __NR_Linux] = { __L4_sys_lchown },
	[__NR_chown	- __NR_Linux] = { __L4_sys_chown16 },
	[__NR_fchown	- __NR_Linux] = { __L4_sys_fchown16 },
	[__NR_lchown	- __NR_Linux] = { __L4_sys_lchown16 },
	[__NR_symlink	- __NR_Linux] = { __L4_sys_symlink },
	[__NR_readlink	- __NR_Linux] = { __L4_sys_readlink },
	[__NR_umask	- __NR_Linux] = { __L4_sys_umask },
	[__NR_fchdir	- __NR_Linux] = { __L4_sys_fchdir },
	[__NR_rename	- __NR_Linux] = { __L4_sys_rename },
	[__NR_mkdir	- __NR_Linux] = { __L4_sys_mkdir },
	[__NR_rmdir	- __NR_Linux] = { __L4_sys_rmdir },
	[__NR_readdir	- __NR_Linux] = { __L4_sys_readdir },
	[__NR_utime	- __NR_Linux] = { __L4_sys_utime },
	[__NR_utimes	- __NR_Linux] = { __L4_sys_utimes },

	[__NR_wait4	- __NR_Linux] = { __L4_sys_wait4 },
	[__NR_uname	- __NR_Linux] = { __L4_sys_uname },

	[__NR_restart_syscall - __NR_Linux] = { __L4_sys_restart_syscall },

	[__NR_brk	- __NR_Linux] = { __L4_sys_brk },
	[__NR_rt_sigaction	- __NR_Linux] = { __L4_sys_rt_sigaction },
	[__NR_rt_sigprocmask	- __NR_Linux] = { __L4_sys_rt_sigprocmask },
	[__NR_rt_sigreturn	- __NR_Linux] = { __L4_sys_rt_sigreturn },
	[__NR_rt_sigpending	- __NR_Linux] = { __L4_sys_rt_sigpending },
	[__NR_rt_sigtimedwait	- __NR_Linux] = { __L4_sys_rt_sigtimedwait },
	[__NR_rt_sigqueueinfo	- __NR_Linux] = { __L4_sys_rt_sigqueueinfo },
	[__NR_rt_sigsuspend	- __NR_Linux] = { __L4_sys_rt_sigsuspend },
	[__NR_sigaltstack	- __NR_Linux] = { __L4_sys_sigaltstack },
//	[__NR_signal		- __NR_Linux] = { __L4_sys_signal },
	[__NR_sigaction		- __NR_Linux] = { __L4_sys_sigaction },
	[__NR_sigprocmask	- __NR_Linux] = { __L4_sys_sigprocmask },
	[__NR_sigreturn		- __NR_Linux] = { __L4_sys_sigreturn },
	[__NR_sigpending	- __NR_Linux] = { __L4_sys_sigpending },
	[__NR_sigsuspend	- __NR_Linux] = { __L4_sys_sigsuspend },

	[__NR_fcntl	- __NR_Linux] = { __L4_sys_fcntl },
	[__NR_fcntl64	- __NR_Linux] = { __L4_sys_fcntl64 },
	[__NR_flock	- __NR_Linux] = { __L4_sys_flock },
	[__NR_fsync	- __NR_Linux] = { __L4_sys_fsync },
	[__NR_fdatasync	- __NR_Linux] = { __L4_sys_fdatasync },
	[__NR_truncate	- __NR_Linux] = { __L4_sys_truncate },
	[__NR_truncate64	- __NR_Linux] = { __L4_sys_truncate64 },
	[__NR_ftruncate	- __NR_Linux] = { __L4_sys_ftruncate },
	[__NR_ftruncate64	- __NR_Linux] = { __L4_sys_ftruncate64 },
	[__NR_getdents	- __NR_Linux] = { __L4_sys_getdents },
	[__NR_getdents64	- __NR_Linux] = { __L4_sys_getdents64 },
	[__NR_getcwd	- __NR_Linux] = { __L4_sys_getcwd },

	[__NR_ptrace	- __NR_Linux] = { __L4_sys_ptrace },

	[__NR_ipc	- __NR_Linux] = { __L4_sys_ipc },
	[__NR_sysinfo	- __NR_Linux] = { __L4_sys_sysinfo },
	[__NR_times	- __NR_Linux] = { __L4_sys_times },
	[__NR_syslog	- __NR_Linux] = { __L4_sys_syslog },

	[__NR_gettimeofday	- __NR_Linux] = { __L4_sys_gettimeofday },
	[__NR_settimeofday	- __NR_Linux] = { __L4_sys_settimeofday },
	[__NR_setrlimit	- __NR_Linux] = { __L4_sys_setrlimit },
	[__NR_ugetrlimit- __NR_Linux] = { __L4_sys_getrlimit },
	[__NR_getrusage	- __NR_Linux] = { __L4_sys_getrusage },

//	[__NR_socket	- __NR_Linux] = { __L4_sys_socket },
	[__NR_socketcall- __NR_Linux] = { __L4_sys_socketcall },

	[__NR_getuid32 	- __NR_Linux] = { __L4_sys_getuid },
	[__NR_getgid32 	- __NR_Linux] = { __L4_sys_getgid },
	[__NR_getuid 	- __NR_Linux] = { __L4_sys_getuid16 },
	[__NR_getgid 	- __NR_Linux] = { __L4_sys_getgid16 },
	[__NR_gettid 	- __NR_Linux] = { __L4_sys_gettid },
	[__NR_setuid32 	- __NR_Linux] = { __L4_sys_setuid },
	[__NR_setgid32 	- __NR_Linux] = { __L4_sys_setgid },
	[__NR_setuid 	- __NR_Linux] = { __L4_sys_setuid16 },
	[__NR_setgid 	- __NR_Linux] = { __L4_sys_setgid16 },
	[__NR_geteuid32	- __NR_Linux] = { __L4_sys_geteuid },
	[__NR_getegid32	- __NR_Linux] = { __L4_sys_getegid },
	[__NR_geteuid	- __NR_Linux] = { __L4_sys_geteuid16 },
	[__NR_getegid	- __NR_Linux] = { __L4_sys_getegid16 },
	[__NR_setpgid	- __NR_Linux] = { __L4_sys_setpgid },
	[__NR_getppid	- __NR_Linux] = { __L4_sys_getppid },
	[__NR_getpgrp	- __NR_Linux] = { __L4_sys_getpgrp },
	[__NR_setsid 	- __NR_Linux] = { __L4_sys_setsid },
	[__NR_setreuid32- __NR_Linux] = { __L4_sys_setreuid },
	[__NR_setregid32- __NR_Linux] = { __L4_sys_setregid },
	[__NR_getgroups32- __NR_Linux] = { __L4_sys_getgroups },
	[__NR_setgroups32- __NR_Linux] = { __L4_sys_setgroups },
	[__NR_setresuid32- __NR_Linux] = { __L4_sys_setresuid },
	[__NR_getresuid32- __NR_Linux] = { __L4_sys_getresuid },
	[__NR_setresgid32- __NR_Linux] = { __L4_sys_setresgid },
	[__NR_getresgid32- __NR_Linux] = { __L4_sys_getresgid },
	[__NR_setreuid- __NR_Linux] = { __L4_sys_setreuid16 },
	[__NR_setregid- __NR_Linux] = { __L4_sys_setregid16 },
	[__NR_getgroups- __NR_Linux] = { __L4_sys_getgroups16 },
	[__NR_setgroups- __NR_Linux] = { __L4_sys_setgroups16 },
	[__NR_setresuid- __NR_Linux] = { __L4_sys_setresuid16 },
	[__NR_getresuid- __NR_Linux] = { __L4_sys_getresuid16 },
	[__NR_setresgid- __NR_Linux] = { __L4_sys_setresgid16 },
	[__NR_getresgid- __NR_Linux] = { __L4_sys_getresgid16 },
	[__NR_getpgid	- __NR_Linux] = { __L4_sys_getpgid },
	[__NR_setfsuid32- __NR_Linux] = { __L4_sys_setfsuid },
	[__NR_setfsgid32- __NR_Linux] = { __L4_sys_setfsgid },
	[__NR_setfsuid- __NR_Linux] = { __L4_sys_setfsuid16 },
	[__NR_setfsgid- __NR_Linux] = { __L4_sys_setfsgid16 },
	[__NR_getsid 	- __NR_Linux] = { __L4_sys_getsid },
	[__NR_capget	- __NR_Linux] = { __L4_sys_capget },
	[__NR_capset 	- __NR_Linux] = { __L4_sys_capset },

	[__NR__sysctl	- __NR_Linux] = { __L4_sys_sysctl },

	[__NR_set_tid_address	- __NR_Linux] = { __L4_sys_set_tid_address },
	[__NR_timer_create	- __NR_Linux] = { __L4_sys_timer_create	},
	[__NR_timer_settime	- __NR_Linux] = { __L4_sys_timer_settime },
	[__NR_timer_gettime	- __NR_Linux] = { __L4_sys_timer_gettime },
	[__NR_timer_getoverrun	- __NR_Linux] = { __L4_sys_timer_getoverrun },
	[__NR_timer_delete	- __NR_Linux] = { __L4_sys_timer_delete },
	[__NR_clock_settime	- __NR_Linux] = { __L4_sys_clock_settime },
	[__NR_clock_gettime	- __NR_Linux] = { __L4_sys_clock_gettime },
	[__NR_clock_getres	- __NR_Linux] = { __L4_sys_clock_getres },
	[__NR_clock_nanosleep	- __NR_Linux] = { __L4_sys_clock_nanosleep },

	[__NR_ustat	- __NR_Linux] = { __L4_sys_ustat },
	[__NR_statfs	- __NR_Linux] = { __L4_sys_statfs },
	[__NR_fstatfs	- __NR_Linux] = { __L4_sys_fstatfs },
	[__NR_statfs64	- __NR_Linux] = { __L4_sys_statfs64 },
	[__NR_fstatfs64	- __NR_Linux] = { __L4_sys_fstatfs64 },
	[__NR_sysfs	- __NR_Linux] = { __L4_sys_sysfs },

	[__NR_chroot	- __NR_Linux] = { __L4_sys_chroot },
	[__NR_sync	- __NR_Linux] = { __L4_sys_sync },
	[__NR_mount	- __NR_Linux] = { __L4_sys_mount },
	[__NR_umount	- __NR_Linux] = { __L4_sys_oldumount },
	[__NR_umount2	- __NR_Linux] = { __L4_sys_umount },

	[__NR_swapon	- __NR_Linux] = { __L4_sys_swapon },
	[__NR_swapoff	- __NR_Linux] = { __L4_sys_swapoff },
	[__NR_reboot	- __NR_Linux] = { __L4_sys_reboot },
	[__NR_sethostname	- __NR_Linux] = { __L4_sys_sethostname },
	[__NR_setdomainname	- __NR_Linux] = { __L4_sys_setdomainname },
	[__NR_exit_group- __NR_Linux] = { __L4_sys_exit_group },

	[__NR_acct	- __NR_Linux] = { __L4_sys_acct },

	[__NR_getpriority	- __NR_Linux] = { __L4_sys_getpriority },
	[__NR_setpriority	- __NR_Linux] = { __L4_sys_setpriority },
	[__NR_sched_setparam	- __NR_Linux] = { __L4_sys_sched_setparam },
	[__NR_sched_getparam	- __NR_Linux] = { __L4_sys_sched_getparam },
	[__NR_sched_setscheduler- __NR_Linux] = { __L4_sys_sched_setscheduler },
	[__NR_sched_getscheduler- __NR_Linux] = { __L4_sys_sched_getscheduler },
	[__NR_sched_yield	- __NR_Linux] = { __L4_sys_sched_yield },
	[__NR_sched_get_priority_max	- __NR_Linux] = { __L4_sys_sched_get_priority_max },
	[__NR_sched_get_priority_min	- __NR_Linux] = { __L4_sys_sched_get_priority_min },
	[__NR_sched_rr_get_interval	- __NR_Linux] = { __L4_sys_sched_rr_get_interval },

	[__NR_adjtimex	- __NR_Linux] = { __L4_sys_adjtimex },
	[__NR_nice	- __NR_Linux] = { __L4_sys_nice },

	[__NR_uselib	- __NR_Linux] = { __L4_sys_uselib },

	[__NR_dup	- __NR_Linux] = { __L4_sys_dup },
	[__NR_dup2	- __NR_Linux] = { __L4_sys_dup2 },
	[__NR_pause	- __NR_Linux] = { __L4_sys_pause },
	[__NR_nanosleep	- __NR_Linux] = { __L4_sys_nanosleep },
	[__NR_getitimer	- __NR_Linux] = { __L4_sys_getitimer },
	[__NR_alarm	- __NR_Linux] = { __L4_sys_alarm },
	[__NR_setitimer	- __NR_Linux] = { __L4_sys_setitimer },

	[__NR_waitpid	- __NR_Linux] = { __L4_sys_waitpid },

	[__NR_create_module	- __NR_Linux] = { __L4_sys_create_module },
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

	[__NR_mq_open	- __NR_Linux] = { __L4_sys_mq_open },
	[__NR_mq_unlink	- __NR_Linux] = { __L4_sys_mq_unlink },
	[__NR_mq_timedsend	- __NR_Linux] = { __L4_sys_mq_timedsend	},
	[__NR_mq_timedreceive   - __NR_Linux] = { __L4_sys_mq_timedreceive },
	[__NR_mq_notify		- __NR_Linux] = { __L4_sys_mq_notify },
	[__NR_mq_getsetattr	- __NR_Linux] = { __L4_sys_mq_getsetattr },

	[__NR_vhangup	- __NR_Linux] = { __L4_sys_vhangup },
	[__NR_quotactl	- __NR_Linux] = { __L4_sys_quotactl },
	[__NR_nfsservctl	- __NR_Linux] = { __L4_sys_nfsservctl },
	[__NR_prctl	- __NR_Linux] = { __L4_sys_prctl },

	[__NR_sendfile	- __NR_Linux] = { __L4_sys_sendfile },
	[__NR_sendfile64	- __NR_Linux] = { __L4_sys_sendfile64 },

	[__NR_pivot_root- __NR_Linux] = { __L4_sys_pivot_root },
	[__NR_mincore	- __NR_Linux] = { __L4_sys_mincore },
	[__NR_madvise	- __NR_Linux] = { __L4_sys_madvise },
	[__NR_readahead	- __NR_Linux] = { __L4_sys_readahead },

	[__NR_futex	- __NR_Linux] = { __L4_sys_futex },
	[__NR_sched_setaffinity	- __NR_Linux] = { __L4_sys_sched_setaffinity },
	[__NR_sched_getaffinity	- __NR_Linux] = { __L4_sys_sched_getaffinity },
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
	[__NR_set_thread_area	- __NR_Linux] = { __L4_sys_set_thread_area },
	[__NR_get_thread_area	- __NR_Linux] = { __L4_sys_get_thread_area },
	[__NR_modify_ldt	- __NR_Linux] = { __L4_sys_modify_ldt },

	[__NR_faccessat		- __NR_Linux] = { __L4_sys_faccessat },
	[__NR_fchmodat		- __NR_Linux] = { __L4_sys_fchmodat },
	[__NR_fchownat		- __NR_Linux] = { __L4_sys_fchownat },
	[__NR_fstatat64		- __NR_Linux] = { __L4_sys_fstatat },
	[__NR_futimesat		- __NR_Linux] = { __L4_sys_futimesat },
	[__NR_linkat		- __NR_Linux] = { __L4_sys_linkat },
	[__NR_openat		- __NR_Linux] = { __L4_sys_openat },
	[__NR_readlinkat	- __NR_Linux] = { __L4_sys_readlinkat },
	[__NR_renameat		- __NR_Linux] = { __L4_sys_renameat },
	[__NR_symlinkat		- __NR_Linux] = { __L4_sys_symlinkat },
	[__NR_unlinkat		- __NR_Linux] = { __L4_sys_unlinkat },

	[__NR_tee		- __NR_Linux] = { __L4_sys_tee },
	[__NR_splice		- __NR_Linux] = { __L4_sys_splice },
	[__NR_vmsplice		- __NR_Linux] = { __L4_sys_vmsplice },

	[__NR_inotify_init	- __NR_Linux] = { __L4_sys_inotify_init },
	[__NR_inotify_add_watch	- __NR_Linux] = { __L4_sys_inotify_add_watch },
	[__NR_inotify_rm_watch	- __NR_Linux] = { __L4_sys_inotify_rm_watch },
	
	[__NR_waitid		- __NR_Linux] = { __L4_sys_waitid },

	[__NR_get_robust_list	- __NR_Linux] = { __L4_sys_get_robust_list },
	[__NR_set_robust_list	- __NR_Linux] = { __L4_sys_set_robust_list },

	[__NR_iopl		- __NR_Linux] = { __L4_sys_iopl },
	[__NR_ioperm		- __NR_Linux] = { __L4_sys_ioperm },
	[__NR_utimensat		- __NR_Linux] = { __L4_sys_utimensat },

	[__NR_timerfd		- __NR_Linux] = { __L4_sys_timerfd },
	[__NR_add_key		- __NR_Linux] = { __L4_sys_add_key },
	[__NR_fadvise64_64	- __NR_Linux] = { __L4_sys_fadvise64_64 },
	[__NR_fadvise64		- __NR_Linux] = { __L4_sys_fadvise64 },
#if 0	/*notyet*/
	[__NR_epoll_pwait	- __NR_Linux] = { __L4_sys_epoll_pwait },
	[__NR_fallocate		- __NR_Linux] = { __L4_sys_fallocate },
#endif
	[__NR_getcpu		- __NR_Linux] = { __L4_sys_getcpu },
	[__NR_get_mempolicy	- __NR_Linux] = { __L4_sys_get_mempolicy },
	[__NR_ioprio_get	- __NR_Linux] = { __L4_sys_ioprio_get },
	[__NR_ioprio_set	- __NR_Linux] = { __L4_sys_ioprio_set },
	[__NR_keyctl		- __NR_Linux] = { __L4_sys_keyctl },
#if 0	/*notyet*/
	[__NR_migrate_pages	- __NR_Linux] = { __L4_sys_migrate_pages },
#endif
	[__NR_mkdirat		- __NR_Linux] = { __L4_sys_mkdirat },
	[__NR_mknodat		- __NR_Linux] = { __L4_sys_mknodat },
	[__NR_move_pages	- __NR_Linux] = { __L4_sys_move_pages },
#if 0	/*notyet*/
	[__NR_ppoll		- __NR_Linux] = { __L4_sys_ppoll },
	[__NR_pselect6		- __NR_Linux] = { __L4_sys_pselect6 },
#endif
	[__NR_signalfd		- __NR_Linux] = { __L4_sys_signalfd },
	[__NR_unshare		- __NR_Linux] = { __L4_sys_unshare },

	[__NR_get_kernel_syms	- __NR_Linux] = { __L4_sys_get_kernel_syms },
};

#include <linux/file.h>
#include <linux/mm.h>
#include <linux/sem.h>

/* common code for old and new mmaps */
static inline long do_mmap2(
	unsigned long addr, unsigned long len,
	unsigned long prot, unsigned long flags,
	unsigned long fd, unsigned long pgoff)
{
	int error = -EBADF;
	struct file * file = NULL;

	flags &= ~(MAP_EXECUTABLE | MAP_DENYWRITE);
	if (!(flags & MAP_ANONYMOUS)) {
		file = fget(fd);
		if (!file)
			goto out;
	}

	down_write(&current->mm->mmap_sem);
	error = do_mmap_pgoff(file, addr, len, prot, flags, pgoff);
	up_write(&current->mm->mmap_sem);

	if (file)
		fput(file);
out:
	return error;
}

asmlinkage long sys_mmap2(unsigned long addr, unsigned long len,
	unsigned long prot, unsigned long flags,
	unsigned long fd, unsigned long pgoff)
{
	return do_mmap2(addr, len, prot, flags, fd, pgoff);
}

/*
 * Perform the select(nd, in, out, ex, tv) and mmap() system
 * calls. Linux/i386 didn't use to be able to handle more than
 * 4 system call parameters, so these system calls used a memory
 * block for parameter passing..
 */

struct mmap_arg_struct {
	unsigned long addr;
	unsigned long len;
	unsigned long prot;
	unsigned long flags;
	unsigned long fd;
	unsigned long offset;
};

asmlinkage int old_mmap(struct mmap_arg_struct __user *arg)
{
	struct mmap_arg_struct a;
	int err = -EFAULT;

	if (copy_from_user(&a, arg, sizeof(a)))
		goto out;

	err = -EINVAL;
	if (a.offset & ~PAGE_MASK)
		goto out;

	err = do_mmap2(a.addr, a.len, a.prot, a.flags, a.fd, a.offset >> PAGE_SHIFT);
out:
	return err;
}

asmlinkage ssize_t sys_pread64(unsigned int fd, char __user *buf,
		size_t count, u32 pos1, u32 pos2);
asmlinkage ssize_t sys_pwrite64(unsigned int fd, const char __user *buf,
		size_t count, u32 pos1, u32 pos2);
asmlinkage long sys_truncate64(const char __user * path, u32 length1, u32 length2);
asmlinkage long sys_ftruncate64(unsigned int fd, u32 length1, u32 length2);
asmlinkage ssize_t sys_readahead(int fd, u32 offset1, u32 offset2, size_t count);
asmlinkage long sys_fadvise64_64(int fd, u32 offset1, u32 offset2,
		u32 len1, u32 len2, int advice);

ssize_t fixup_sys_pread64(struct pt_regs *regs)
{
	return sys_pread64(i386_ebx(regs), (char*)i386_ecx(regs), i386_edx(regs),
			i386_esi(regs),  i386_edi(regs));
}

ssize_t fixup_sys_pwrite64(struct pt_regs *regs)
{
	return sys_pwrite64(i386_ebx(regs), (char*)i386_ecx(regs), i386_edx(regs),
			i386_esi(regs),  i386_edi(regs));
}

long fixup_sys_truncate64(struct pt_regs *regs)
{
	return sys_truncate64((char*)i386_ebx(regs), i386_ecx(regs), i386_edx(regs));
}

long fixup_sys_ftruncate64(struct pt_regs *regs)
{
	return sys_ftruncate64(i386_ebx(regs), i386_ecx(regs), i386_edx(regs));
}

ssize_t fixup_sys_readahead(struct pt_regs *regs)
{
	return sys_readahead(i386_ebx(regs), i386_ecx(regs), i386_edx(regs), i386_esi(regs));
}

long fixup_sys_fadvise64_64(struct pt_regs *regs)
{
	return sys_fadvise64_64(i386_ebx(regs), i386_ecx(regs), i386_edx(regs),
			i386_esi(regs), i386_edi(regs), i386_ebp(regs));
}
