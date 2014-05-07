#ifndef _L4_MIPS_SYSCALL_DEFS_H_
#define _L4_MIPS_SYSCALL_DEFS_H_

extern l4_syscall_handler_t     sys_mmap;
extern l4_syscall_handler_t     sys_bind;
extern l4_syscall_handler_t     sys_connect;
extern l4_syscall_handler_t     sys_listen;
extern l4_syscall_handler_t     sys_accept;
extern l4_syscall_handler_t     sys_getsockname;
extern l4_syscall_handler_t     sys_getpeername;
extern l4_syscall_handler_t     sys_socketpair;
extern l4_syscall_handler_t     sys_send;
extern l4_syscall_handler_t     sys_sendto;
extern l4_syscall_handler_t     sys_recv;
extern l4_syscall_handler_t     sys_recvfrom;
extern l4_syscall_handler_t     sys_shutdown;
extern l4_syscall_handler_t     sys_setsockopt;
extern l4_syscall_handler_t     sys_getsockopt;
extern l4_syscall_handler_t     sys_sendmsg;
extern l4_syscall_handler_t     sys_recvmsg;
extern l4_syscall_handler_t     sys_nfsservctl;


#define ARCH_SPECIFIC_SYSCALL_LINKAGE									\
	[__L4_sys_pipe  ]   = { sys_pipe, {{ .args = 0, .flags = 1, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_mmap	]   = { sys_mmap, {{ .args = 6, .ret_type = L4_RET_TYPE_LONG }} },		\
	[__L4_sys_bind	]   = { sys_bind, {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_connect    	] = { sys_connect,  {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_listen     	] = { sys_listen,   {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_accept     	] = { sys_accept,   {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_getsockname	] = { sys_getsockname,	{{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_getpeername	] = { sys_getpeername,	{{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_socketpair 	] = { sys_socketpair,	{{ .args = 4, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_send       	] = { sys_send,	    {{ .args = 4, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_sendto     	] = { sys_sendto,   {{ .args = 6, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_recv       	] = { sys_recv,	    {{ .args = 4, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_recvfrom   	] = { sys_recvfrom, {{ .args = 6, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_shutdown   	] = { sys_shutdown, {{ .args = 2, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_setsockopt 	] = { sys_setsockopt,	{{ .args = 5, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_getsockopt 	] = { sys_getsockopt,	{{ .args = 5, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_sendmsg    	] = { sys_sendmsg,  {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_recvmsg    	] = { sys_recvmsg,  {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_nfsservctl	] = { sys_nfsservctl,	{{ .args = 3, .ret_type = L4_RET_TYPE_INT }} },	\
	COMPAT_SPECIFIC_SYSCALL_LINKAGE

#ifdef CONFIG_COMPAT
extern l4_syscall_handler_t     compat_sys_fcntl64;
extern l4_syscall_handler_t     sys32_newuname;
extern l4_syscall_handler_t     sys32_sigreturn;
extern l4_syscall_handler_t     sys32_sigaction;
extern l4_syscall_handler_t     compat_sys_sigprocmask;
extern l4_syscall_handler_t     compat_sys_sigpending;
extern l4_syscall_handler_t     sys32_sigsuspend;

extern l4_syscall_handler_t     sys32_rt_sigreturn;
extern l4_syscall_handler_t     sys32_rt_sigaction;
extern l4_syscall_handler_t     sys32_rt_sigprocmask;
extern l4_syscall_handler_t     sys32_rt_sigpending;
extern l4_syscall_handler_t     sys32_rt_sigtimedwait;
extern l4_syscall_handler_t     sys32_rt_sigqueueinfo;
extern l4_syscall_handler_t     sys32_rt_sigsuspend;

extern l4_syscall_handler_t     sys32_waitpid;
extern l4_syscall_handler_t     compat_sys_wait4;

extern l4_syscall_handler_t     compat_sys_ioctl;
extern l4_syscall_handler_t     compat_sys_fcntl;
extern l4_syscall_handler_t     sys_stime;
extern l4_syscall_handler_t     sys_newstat;
extern l4_syscall_handler_t     sys_newlstat;
extern l4_syscall_handler_t     sys_newfstat;
extern l4_syscall_handler_t     compat_sys_newstat;
extern l4_syscall_handler_t     compat_sys_newlstat;
extern l4_syscall_handler_t     compat_sys_newfstat;
extern l4_syscall_handler_t	compat_sys_statfs;
extern l4_syscall_handler_t	compat_sys_fstatfs;
extern l4_syscall_handler_t	compat_sys_getrusage;
extern l4_syscall_handler_t	sys_getdents64;

extern l4_syscall_handler_t     sys32_sysinfo;

extern l4_syscall_handler_t     sys32_execve;

extern l4_syscall_handler_t     compat_sys_nanosleep;

extern l4_syscall_handler_t     old_mmap;
extern l4_syscall_handler_t     sys32_mmap2;

extern l4_syscall_handler_t     sys32_getdents;
extern l4_syscall_handler_t     sys_oldumount;

extern l4_syscall_handler_t     compat_sys_setsockopt;
extern l4_syscall_handler_t     compat_sys_sendmsg;
extern l4_syscall_handler_t     compat_sys_recvmsg;

extern l4_syscall_handler_t     compat_sys_readv;
extern l4_syscall_handler_t     compat_sys_writev;

extern l4_syscall_handler_t	sys32_gettimeofday;
extern l4_syscall_handler_t	compat_sys_getrlimit;
extern l4_syscall_handler_t	compat_sys_setrlimit;
extern l4_syscall_handler_t	sys32_llseek;
extern l4_syscall_handler_t	compat_sys_utime;

extern l4_syscall_handler_t	sys32_adjtimex;


#define	COMPAT_SPECIFIC_SYSCALL_LINKAGE	\
	[__L4_compat_sys_fcntl64] = { compat_sys_fcntl64, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys32_newuname   ] = { sys32_newuname, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys32_sigreturn	] = { sys32_sigreturn, {{ .args = 0, .flags = 1, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys32_sigaction	] = { sys32_sigaction, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys32_sigprocmask	] = { compat_sys_sigprocmask, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_sigpending	] = { compat_sys_sigpending, {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_sigsuspend	] = { sys32_sigsuspend, {{ .args = 0, .flags = 1, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys32_rt_sigreturn] = { sys32_rt_sigreturn, {{ .args = 0, .flags = 1, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys32_rt_sigaction] = { sys32_rt_sigaction, {{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys32_rt_sigprocmask] = { sys32_rt_sigprocmask, {{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_rt_sigpending] = { sys32_rt_sigpending, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys32_rt_sigtimedwait] = { sys32_rt_sigtimedwait, {{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_rt_sigqueueinfo] = { sys32_rt_sigqueueinfo, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_waitpid	] = { sys32_waitpid, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_compat_sys_wait4	] = { compat_sys_wait4, {{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },\
	[__L4_compat_sys_ioctl	] = { compat_sys_ioctl, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_compat_sys_fcntl	] = { compat_sys_fcntl, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys_newstat	] = { sys_newstat, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys_newlstat	] = { sys_newlstat, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys_newfstat	] = { sys_newfstat, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_compat_sys_newstat]  = { compat_sys_newstat, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_compat_sys_newlstat] = { compat_sys_newlstat, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_compat_sys_newfstat] = { compat_sys_newfstat, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_sysinfo	] = { sys32_sysinfo, {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_execve	] = { sys32_execve, {{ .args = 0, .flags = 1, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_compat_sys_nanosleep] = { compat_sys_nanosleep, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys32_mmap2	] = { sys32_mmap2, {{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },\
	[__L4_old_mmap		] = { old_mmap, {{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },\
	[__L4_sys32_getdents	] = { sys32_getdents, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },\
	[__L4_sys_oldumount ]	= { sys_oldumount,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_getdents64]	= { sys_getdents64,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_stime	]   = { sys_stime,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_setsockopt 	] = { compat_sys_setsockopt,	{{ .args = 5, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_sendmsg    	] = { compat_sys_sendmsg,  {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys_recvmsg    	] = { compat_sys_recvmsg,  {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_compat_sys_readv    	] = { compat_sys_readv,	    {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_compat_sys_writev    	] = { compat_sys_writev,    {{ .args = 3, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_sys32_gettimeofday	] = { sys32_gettimeofday,   {{ .args = 2, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_compat_sys_getrlimit   ] = { compat_sys_getrlimit,    {{ .args = 2, .ret_type = L4_RET_TYPE_LONG }} },  \
	[__L4_compat_sys_setrlimit ]    = { compat_sys_setrlimit,   {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_compat_sys_statfs    ]    = { compat_sys_statfs,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_compat_sys_fstatfs   ]    = { compat_sys_fstatfs,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_compat_sys_getrusage   ]  = { compat_sys_getrusage,   {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys32__llseek ]    =	{ sys32_llseek,	    {{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_compat_sys_utime	] =	{ compat_sys_utime, {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys32_adjtimex	] =	{ sys32_adjtimex,   {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
/* end of list */

#else
#define COMPAT_SPECIFIC_SYSCALL_LINKAGE
#endif

#endif /* _L4_MIPS_SYSCALL_DEFS_H_ */
