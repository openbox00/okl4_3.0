#ifndef __L4_ARM_UNISTD_H
#define __L4_ARM_UNISTD_H

#define __ARCH_WANT_STAT64
#define __ARCH_WANT_SYS_LLSEEK
#define __ARCH_WANT_SYS_TIME
#define __ARCH_WANT_SYS_UTIME
#define __ARCH_WANT_SYS_ALARM
#define __ARCH_WANT_SYS_RT_SIGACTION
#define __ARCH_WANT_SYS_SIGPENDING
#define __ARCH_WANT_SYS_SIGPROCMASK
#define __ARCH_WANT_SYS_PAUSE
#define __ARCH_WANT_SYS_GETPGRP
#define __ARCH_WANT_SYS_OLDUMOUNT
#define __ARCH_WANT_SYS_NICE
#define __ARCH_WANT_SYS_SOCKETCALL
#define __ARCH_WANT_IPC_PARSE_VERSION
#define __ARCH_WANT_OLD_READDIR

/*
 * Make sure this +5 is free, please refer to include/asm-arm/unistd.h
 * But this is better than patching the mainline Linux source.
 */
#undef __ARM_NR_set_tls
#define __ARM_NR_set_tls		(__ARM_NR_BASE+5)

#endif /* __L4_ARM_UNISTD_H */
