#include <linux/init.h>

__INITDATA

.globl vsyscall_sysenter_start, vsyscall_sysenter_end
vsyscall_sysenter_start:
.incbin "arch/l4/sys-i386/vsyscall-sysenter.so"
vsyscall_sysenter_end:

__FINIT
