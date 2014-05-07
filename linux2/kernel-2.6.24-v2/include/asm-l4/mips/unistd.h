#ifndef __L4_MIPS_UNISTD_H
#define __L4_MIPS_UNISTD_H

//#define __ARCH_WANT_STAT64
#define __ARCH_WANT_SYS_LLSEEK
#define __ARCH_WANT_SYS_TIME
#define __ARCH_WANT_SYS_UTIME
#define __ARCH_WANT_SYS_ALARM
#define __ARCH_WANT_SYS_RT_SIGACTION
#define __ARCH_WANT_SYS_SIGPENDING
#define __ARCH_WANT_SYS_SIGPROCMASK
#define __ARCH_WANT_SYS_PAUSE
#define __ARCH_WANT_SYS_GETPGRP
#define __ARCH_WANT_SYS_SOCKETCALL
#define __ARCH_WANT_IPC_PARSE_VERSION

#ifdef CONFIG_COMPAT
#define __ARCH_WANT_SYS_OLDUMOUNT
#define __ARCH_WANT_SYS_NICE
#endif 

#endif /* __L4_MIPS_UNISTD_H */
