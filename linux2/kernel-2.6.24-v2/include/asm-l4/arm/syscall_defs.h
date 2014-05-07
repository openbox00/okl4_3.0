#ifndef _L4_ARM_SYSCALL_DEFS_H_
#define _L4_ARM_SYSCALL_DEFS_H_

extern l4_syscall_handler_t     old_mmap;
extern l4_syscall_handler_t     sys_mmap;
extern l4_syscall_handler_t	sys_sigaction;
extern l4_syscall_handler_t	sys_sigsuspend;
extern l4_syscall_handler_t	sys_sigreturn;
extern l4_syscall_handler_t	sys_rt_sigsuspend;
extern l4_syscall_handler_t	sys_sigaltstack;
extern l4_syscall_handler_t     sys_oldumount;
extern l4_syscall_handler_t     sys_stime;
extern l4_syscall_handler_t     old_select;
extern l4_syscall_handler_t	sys_ptrace;
extern l4_syscall_handler_t	sys_arm_syscall;
extern l4_syscall_handler_t	old_readdir;
extern l4_syscall_handler_t	sys_ipc;
extern l4_syscall_handler_t	sys_nfsservctl;
extern l4_syscall_handler_t	sys_pciconfig_iobase;
extern l4_syscall_handler_t	sys_arm_set_tls;
extern l4_syscall_handler_t	sys_arm_breakpoint;
extern l4_syscall_handler_t	sys_set_tid_address;

#define ARCH_SPECIFIC_SYSCALL_LINKAGE									\
	[__L4_sys_pipe  ] = { sys_pipe,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_mmap	] = { sys_mmap,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_old_mmap	] = { old_mmap,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_sigaction	] = { sys_sigaction,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_sigsuspend	] = { sys_sigsuspend,	{{ .args = 3, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_sigreturn	] = { sys_sigreturn,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_rt_sigsuspend	] = { sys_rt_sigsuspend,{{ .args = 2, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_rt_sigreturn	] = { sys_rt_sigreturn,	{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_sigaltstack	] = { sys_sigaltstack,	{{ .args = 2, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_oldumount ] = { sys_oldumount,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_stime	]   = { sys_stime,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_old_select]   = { old_select,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_ptrace]   = { sys_ptrace,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_syscall]   = { sys_arm_syscall,{{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} },	\
	[__L4_sys_readdir   ]	= { old_readdir,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_ipc	]   = { sys_ipc,	{{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_nfsservctl	]   = { sys_nfsservctl,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_INT }} },	\
	[__L4_sys_pciconfig_iobase ]   = { sys_pciconfig_iobase,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_arm_set_tls]   = { sys_arm_set_tls,    {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_arm_breakpoint]   = { sys_arm_breakpoint,    {{ .args = 0, .flags = L4_SYS_FLAGS_NEED_REGS, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_set_tid_address]   = { sys_set_tid_address,    {{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_socket]   = { sys_socket,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_bind]   = { sys_bind,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_connect]   = { sys_connect,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_listen]   = { sys_listen,    {{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_accept]   = { sys_accept,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_getsockname]   = { sys_getsockname,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_getpeername]   = { sys_getpeername,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_socketpair]   = { sys_socketpair,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_getsockopt]   = { sys_getsockopt,    {{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_setsockopt]   = { sys_setsockopt,    {{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_send]   = { sys_send,    {{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_sendto]   = { sys_sendto,    {{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_recv]   = { sys_recv,    {{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_recvfrom]   = { sys_recvfrom,    {{ .args = 6, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_shutdown]   = { sys_shutdown,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_recvmsg]   = { sys_recvmsg,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_sendmsg]   = { sys_sendmsg,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_semop]   = { sys_semop,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_semget]   = { sys_semget,    {{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_semctl]   = { sys_semctl,    {{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_msgsnd]   = {sys_msgsnd,	{{ .args = 4, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_msgrcv]   = {sys_msgrcv,	{{ .args = 5, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_msgget]   = {sys_msgget,	{{ .args = 2, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_msgctl]   = {sys_msgctl,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_shmat]   = {sys_shmat,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_shmdt]   = {sys_shmdt,	{{ .args = 1, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_shmget]   = {sys_shmget,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
	[__L4_sys_shmctl]   = {sys_shmctl,	{{ .args = 3, .flags = 0, .ret_type = L4_RET_TYPE_LONG }} }, \
/* end list */


#endif /* _L4_ARM_SYSCALL_DEFS_H_ */
