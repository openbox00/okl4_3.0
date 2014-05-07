#ifndef _L4_I386_SYSCALL_DEFS_H_
#define _L4_I386_SYSCALL_DEFS_H_

extern l4_syscall_handler_t     sys_waitpid;
extern l4_syscall_handler_t     old_mmap;
extern l4_syscall_handler_t     sys_mmap2;
extern l4_syscall_handler_t     sys_oldumount;
extern l4_syscall_handler_t	sys_sigaction;
extern l4_syscall_handler_t	sys_sigsuspend;
extern l4_syscall_handler_t	sys_sigreturn;
extern l4_syscall_handler_t	sys_rt_sigsuspend;
extern l4_syscall_handler_t	sys_sigaltstack;
extern l4_syscall_handler_t     sys_stime;
extern l4_syscall_handler_t     old_select;
extern l4_syscall_handler_t     sys_ptrace;
extern l4_syscall_handler_t	old_readdir;
extern l4_syscall_handler_t	sys_ipc;
extern l4_syscall_handler_t	sys_nfsservctl;
extern l4_syscall_handler_t	sys_set_tid_address;
extern l4_syscall_handler_t	sys_set_thread_area;
extern l4_syscall_handler_t	sys_get_thread_area;
extern l4_syscall_handler_t	sys_modify_ldt;
extern l4_syscall_handler_t	sys_iopl;
extern l4_syscall_handler_t	sys_ioperm;


#define ARCH_SPECIFIC_SYSCALL_LINKAGE									\
	[__L4_sys_pipe  ] = { sys_pipe,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_waitpid ] = { sys_waitpid,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_mmap	]   = { sys_mmap2,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_old_mmap	]   = { old_mmap,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_oldumount ] = { sys_oldumount,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_sigaction	] = { sys_sigaction,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_sigsuspend	] = { sys_sigsuspend,	{{ .args = 3, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_sigreturn	] = { sys_sigreturn,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_rt_sigsuspend	] = { sys_rt_sigsuspend,{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_rt_sigreturn	] = { sys_rt_sigreturn,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_sigaltstack	] = { sys_sigaltstack,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_stime	]   = { sys_stime,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_old_select]   = { old_select,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_ptrace]   = { sys_ptrace,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_readdir   ]	= { old_readdir,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_ipc	]   = { sys_ipc,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_nfsservctl	]   = { sys_nfsservctl,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_modify_ldt	]   = { sys_modify_ldt,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_set_tid_address] = { sys_set_tid_address, {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_set_thread_area] = { sys_set_thread_area, {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_get_thread_area] = { sys_get_thread_area, {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
        [__L4_sys_iopl  ] = { sys_iopl, {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
        [__L4_sys_ioperm	] = { sys_ioperm, {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \

/* end list */

#endif /* _L4_I386_SYSCALL_DEFS_H_ */
