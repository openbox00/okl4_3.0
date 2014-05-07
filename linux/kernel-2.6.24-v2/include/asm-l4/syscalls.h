/* 
 * asm-l4/syscalls.h
 *
 * Copyright 2004 National ICT Australia.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#ifndef _L4_SYSCALLS_H_
#define _L4_SYSCALLS_H_

#include <asm/macros.h>
#include <assert.h>

#define L4_RET_TYPE_INT		0
#define L4_RET_TYPE_LONG	1
#define L4_RET_TYPE_INT_32	2
#define L4_RET_TYPE_LONG_32	3

#define L4_SYS_FLAGS_NONE	0
#define L4_SYS_FLAGS_NEED_REGS	1

typedef unsigned long l4_syscall_handler_t(void);

typedef struct {
    l4_syscall_handler_t *fn;
    union {
	struct {
	    unsigned long args		: _L4_WORD/4;
	    unsigned long flags		: _L4_WORD/4;
	    unsigned long ret_type	: _L4_WORD/2;
	};
	unsigned long __x;
    };
} l4_syscall_entry_t;

extern l4_syscall_entry_t l4_syscall_table[];


/*
 * L4Linux internal syscall numbers
 */
#define __L4_sys_unimplemented	0
#define __L4_sys_exit		1
#define __L4_sys_fork		2
#define __L4_sys_vfork		3
#define __L4_sys_execve		4
#define __L4_sys_clone		5
#define __L4_sys_kill		6
#define __L4_sys_tkill		7
#define __L4_sys_tgkill		8
#define __L4_sys_getpid		9

#define __L4_sys_read		10
#define __L4_sys_write		11
#define __L4_sys_open		12
#define __L4_sys_close		13
#define __L4_sys_stat		14
#define __L4_sys_stat64		15	// 32-bit
#define __L4_sys_fstat		16
#define __L4_sys_fstat64	17	// 32-bit
#define __L4_sys_lstat		18
#define __L4_sys_lstat64	19	// 32-bit
#define __L4_sys_lseek		20
#define __L4_sys__llseek	21	// 32-bit

#define __L4_sys_poll		22
#define __L4_sys_pread64	23
#define __L4_sys_pwrite64	24
#define __L4_sys_readv		25
#define __L4_sys_writev		26
#define __L4_sys_ioctl		27
#define __L4_sys_access		28
#define __L4_sys_pipe		29
#define __L4_sys_select		30
#define __L4_sys_mmap		31
#define __L4_sys_mprotect	32
#define __L4_sys_munmap		33
#define __L4_sys_mremap		34
#define __L4_sys_msync		35

#define __L4_sys_mlock		36
#define __L4_sys_munlock	37
#define __L4_sys_mlockall	38
#define __L4_sys_munlockall	39

#define __L4_sys_creat		40
#define __L4_sys_link		41
#define __L4_sys_unlink		42
#define __L4_sys_chdir		43
#define __L4_sys_time		44
#define __L4_sys_mknod		45
#define __L4_sys_chmod		46
#define __L4_sys_chown		47
#define __L4_sys_fchmod		48
#define __L4_sys_fchown		49
#define __L4_sys_lchown		50
#define __L4_sys_symlink	51
#define __L4_sys_readlink	52
#define __L4_sys_umask		53
#define __L4_sys_fchdir		54
#define __L4_sys_rename		55
#define __L4_sys_mkdir		56
#define __L4_sys_rmdir		57
#define __L4_sys_readdir	58
#define __L4_sys_utime		59
#define __L4_sys_utimes		60

#define __L4_sys_brk			61
#define __L4_sys_rt_sigaction		62
#define __L4_sys_rt_sigprocmask		63
#define __L4_sys_rt_sigreturn		64
#define __L4_sys_rt_sigpending		65
#define __L4_sys_rt_sigtimedwait	66
#define __L4_sys_rt_sigqueueinfo	67
#define __L4_sys_rt_sigsuspend		68
#define __L4_sys_sigaltstack		69
#define __L4_sys_signal			70
#define __L4_sys_sigaction		71
#define __L4_sys_sigsuspend		72
#define __L4_sys_sigpending		73
#define __L4_sys_sigreturn              74
#define __L4_sys_sigprocmask            75

#define __L4_sys_fcntl		76
#define __L4_sys_fcntl64	77	// 32-bit
#define __L4_sys_flock		78
#define __L4_sys_fsync		79
#define __L4_sys_fdatasync	80
#define __L4_sys_truncate	81
#define __L4_sys_truncate64	82
#define __L4_sys_ftruncate	83
#define __L4_sys_ftruncate64	84
#define __L4_sys_getdents	85
#define __L4_sys_getdents64	86
#define __L4_sys_getcwd		87

#define __L4_sys_ptrace		89

#define __L4_sys_ipc		90
#define __L4_sys_sysinfo	91
#define __L4_sys_times		92
#define __L4_sys_syslog		93

//#define __L4_sys_socket		94
#define __L4_sys_socketcall	95

#define __L4_sys_gettimeofday	96
#define __L4_sys_settimeofday	97
#define __L4_sys_setrlimit	98
#define __L4_sys_getrlimit	99
#define __L4_sys_getrusage	100

#define __L4_sys_getuid		101
#define __L4_sys_getgid		102
#define __L4_sys_gettid		103
#define __L4_sys_setuid		104
#define __L4_sys_setgid		105
#define __L4_sys_geteuid	106
#define __L4_sys_getegid	107
#define __L4_sys_setpgid	108
#define __L4_sys_getppid	109
#define __L4_sys_getpgrp	110
#define __L4_sys_setsid		111
#define __L4_sys_setreuid	112
#define __L4_sys_setregid	113
#define __L4_sys_getgroups	114
#define __L4_sys_setgroups	115
#define __L4_sys_setresuid	116
#define __L4_sys_getresuid	117
#define __L4_sys_setresgid	118
#define __L4_sys_getresgid	119
#define __L4_sys_getpgid	120
#define __L4_sys_setfsuid	121
#define __L4_sys_setfsgid	122
#define __L4_sys_getsid		123
#define __L4_sys_capget		124
#define __L4_sys_capset		125

#define __L4_sys_syscall	126
#define __L4_sys_wait4		127
#define __L4_sys_uname		128

#define __L4_sys_restart_syscall	129

#define __L4_sys_sysctl		130

#define __L4_sys_getpriority		131
#define __L4_sys_setpriority		132
#define __L4_sys_sched_setparam		133
#define __L4_sys_sched_getparam		134
#define __L4_sys_sched_setscheduler	135
#define __L4_sys_sched_getscheduler	136
#define __L4_sys_sched_yield		137
#define __L4_sys_sched_get_priority_max	138
#define __L4_sys_sched_get_priority_min	139
#define __L4_sys_sched_rr_get_interval	140

#define __L4_sys_timer_create	141
#define __L4_sys_timer_settime		(__L4_sys_timer_create+1)
#define __L4_sys_timer_gettime		(__L4_sys_timer_create+2)
#define __L4_sys_timer_getoverrun	(__L4_sys_timer_create+3)
#define __L4_sys_timer_delete		(__L4_sys_timer_create+4)
#define __L4_sys_clock_settime		(__L4_sys_timer_create+5)
#define __L4_sys_clock_gettime		(__L4_sys_timer_create+6)
#define __L4_sys_clock_getres		(__L4_sys_timer_create+7)
#define __L4_sys_clock_nanosleep	(__L4_sys_timer_create+8)

#define __L4_sys_ustat		150
#define __L4_sys_statfs		151
#define __L4_sys_fstatfs	152
#define __L4_sys_statfs64	153	// 32-bit arch only
#define __L4_sys_fstatfs64	154	// 32-bit arch only
#define __L4_sys_sysfs		155

#define __L4_sys_chroot		156
#define __L4_sys_sync		157
#define __L4_sys_mount		158
#define __L4_sys_umount		159

#define __L4_sys_swapon		160
#define __L4_sys_swapoff	161
#define __L4_sys_reboot		162
#define __L4_sys_sethostname	163
#define __L4_sys_setdomainname	164
#define	__L4_sys_exit_group	165

#define __L4_sys_acct		166
#define __L4_sys_adjtimex	167
#define __L4_sys_nice		168

#define __L4_sys_uselib		169

#define __L4_sys_dup		170
#define __L4_sys_dup2		171
#define __L4_sys_pause		172
#define __L4_sys_nanosleep	173
#define __L4_sys_getitimer	174
#define __L4_sys_alarm		175
#define __L4_sys_setitimer	176

#define __L4_sys_create_module	177
#define __L4_sys_init_module	178
#define __L4_sys_delete_module	179

#define __L4_sys_setxattr	180
#define __L4_sys_lsetxattr	181
#define __L4_sys_fsetxattr	182
#define __L4_sys_getxattr	183
#define __L4_sys_lgetxattr	184
#define __L4_sys_fgetxattr	185
#define __L4_sys_listxattr	186
#define __L4_sys_llistxattr	187
#define __L4_sys_flistxattr	188
#define __L4_sys_removexattr	189
#define __L4_sys_lremovexattr	190
#define __L4_sys_fremovexattr	191

#define __L4_sys_mq_open	192
#define __L4_sys_mq_unlink	193
#define __L4_sys_mq_timedsend	194
#define __L4_sys_mq_timedreceive	195
#define __L4_sys_mq_notify	196
#define __L4_sys_mq_getsetattr	197

#define __L4_sys_vhangup	198
#define __L4_sys_quotactl	199
#define __L4_sys_nfsservctl	200
#define __L4_sys_prctl		201

#define __L4_sys_sendfile	202	// 32-bit arch only
#define __L4_sys_sendfile64	203

#define __L4_sys_pivot_root	204
#define __L4_sys_mincore	205
#define __L4_sys_madvise	206
#define __L4_sys_readahead	207

#define __L4_sys_futex		208
#define __L4_sys_sched_setaffinity	209
#define __L4_sys_sched_getaffinity	210
#define __L4_sys_io_setup	211
#define __L4_sys_io_destroy	212
#define __L4_sys_io_getevents	213
#define __L4_sys_io_submit	214
#define __L4_sys_io_cancel	215

#define	__L4_sys_lookup_dcookie	216
#define __L4_sys_epoll_create	217
#define __L4_sys_epoll_ctl	218
#define __L4_sys_epoll_wait	219
#define __L4_sys_remap_file_pages	220

#define __L4_sys_fadvise64_64	221
#define __L4_sys_pciconfig_iobase	222
#define __L4_sys_pciconfig_read	223
#define __L4_sys_pciconfig_write	224

#define __L4_sys_waitid		225

#define __L4_sys_set_thread_area	226
#define __L4_sys_get_thread_area	227
#define __L4_sys_set_tid_address	228

#define __L4_sys_faccessat		230
#define __L4_sys_fchmodat		231
#define __L4_sys_fchownat		232
#define __L4_sys_fstatat		233
#define __L4_sys_futimesat		224
#define __L4_sys_linkat			235
#define __L4_sys_openat			236
#define __L4_sys_readlinkat		237
#define __L4_sys_renameat		238
#define __L4_sys_symlinkat		239
#define __L4_sys_unlinkat		240

#define __L4_sys_tee			241
#define __L4_sys_splice			242
#define __L4_sys_vmsplice		243

#define __L4_sys_timerfd		244
#define __L4_sys_add_key		245
#define __L4_sys_epoll_pwait		246
#define __L4_sys_eventfd		247
#define __L4_sys_fallocate		248
#define __L4_sys_getcpu			249
#define __L4_sys_get_mempolicy		250
#define __L4_sys_get_robust_list	251
#define __L4_sys_set_robust_list	252
#define __L4_sys_inotify_add_watch	253
#define __L4_sys_inotify_init		254
#define __L4_sys_inotify_rm_watch	255
#define __L4_sys_ioprio_get		256
#define __L4_sys_ioprio_set		257
#define __L4_sys_kexec_load		258
#define __L4_sys_keyctl			259
#define __L4_sys_mbind			260
#define __L4_sys_migrate_pages		261
#define __L4_sys_mkdirat		262
#define __L4_sys_mknodat		263
#define __L4_sys_move_pages		264
#define __L4_sys_ppoll			265
#define __L4_sys_pselect6		266
#define __L4_sys_request_key		267
#define __L4_sys_signalfd		268
#define __L4_sys_utimensat		269
#define __L4_sys_unshare		270
#define __L4_sys_get_kernel_syms	271
#define __L4_sys_fadvise64		272
/* These are currently ARM-specific. */
#define __L4_sys_socket			273
#define __L4_sys_bind			274
#define __L4_sys_connect		275
#define __L4_sys_listen			276
#define __L4_sys_accept			277
#define __L4_sys_getsockname		278
#define __L4_sys_getpeername		279
#define __L4_sys_socketpair		280
#define __L4_sys_getsockopt		281
#define __L4_sys_setsockopt		282
#define __L4_sys_send			283
#define __L4_sys_sendto			284
#define __L4_sys_recv			285
#define __L4_sys_recvfrom		286
#define __L4_sys_shutdown		287
#define __L4_sys_recvmsg		288
#define __L4_sys_sendmsg		289
#define __L4_sys_msgsnd			290
#define __L4_sys_msgrcv			291
#define __L4_sys_msgget			292
#define __L4_sys_msgctl			293
#define __L4_sys_shmat			294
#define __L4_sys_shmdt			295
#define __L4_sys_shmget			296
#define __L4_sys_shmctl			297
#define __L4_sys_semop			298
#define __L4_sys_semget			299
#define __L4_sys_semctl			300

#define __L4_sys_main_end		300


#ifndef	CONFIG_UID16

#define __L4_sys_l4_last	__L4_sys_main_end

#else /* CONFIG_UID16 */

#define	__L4_sys_chown16	__L4_sys_main_end + 1
#define	__L4_sys_lchown16	__L4_sys_main_end + 2
#define	__L4_sys_fchown16	__L4_sys_main_end + 3
#define	__L4_sys_getuid16	__L4_sys_main_end + 4
#define	__L4_sys_getgid16	__L4_sys_main_end + 5
#define	__L4_sys_setuid16	__L4_sys_main_end + 6
#define	__L4_sys_setgid16	__L4_sys_main_end + 7
#define	__L4_sys_geteuid16	__L4_sys_main_end + 8
#define	__L4_sys_getegid16	__L4_sys_main_end + 9
#define	__L4_sys_setreuid16	__L4_sys_main_end + 10
#define	__L4_sys_setregid16	__L4_sys_main_end + 11
#define	__L4_sys_getgroups16	__L4_sys_main_end + 12
#define	__L4_sys_setgroups16	__L4_sys_main_end + 13
#define	__L4_sys_setresuid16	__L4_sys_main_end + 14
#define	__L4_sys_getresuid16	__L4_sys_main_end + 15
#define	__L4_sys_setresgid16	__L4_sys_main_end + 16
#define	__L4_sys_getresgid16	__L4_sys_main_end + 17
#define	__L4_sys_setfsuid16	__L4_sys_main_end + 18
#define	__L4_sys_setfsgid16	__L4_sys_main_end + 19

#define __L4_sys_l4_last	__L4_sys_main_end + 20

#endif /* CONFIG_UID16 */

#include INC_SYSTEM2(syscalls.h)

#endif /* _L4_SYSCALLS_H_ */
