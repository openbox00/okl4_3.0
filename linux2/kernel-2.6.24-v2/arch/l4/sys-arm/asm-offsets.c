/*
 * Copyright (C) 2004 National ICT Australia
 *     
 * Generate definitions needed by assembly language modules.
 * This code generates raw asm output which is post-processed to extract
 * and format the required data.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#define INC_SYS(file) <../sys-__SYSTEM__/file>

#include <linux/sched.h>
#include <asm/signal.h>
#include <asm/ucontext.h>
#include INC_SYS(sigframe.h)

#define DEFINE(sym, val) \
        asm volatile("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() asm volatile("\n->" : : )

int main(void)
{
    DEFINE(SIZEOF_SIGFRAME,	sizeof (struct sigframe));
    DEFINE(SIZEOF_SIGRTFRAME,	sizeof (struct rt_sigframe));
    
    BLANK();

    DEFINE(L4_SIGFRAME_SIGIP_OFFSET,	offsetof (struct sigframe, sig_ip));
    DEFINE(L4_SIGRTFRAME_SIGIP_OFFSET,	offsetof (struct rt_sigframe, sig_ip));

    DEFINE(L4_SIGFRAME_R0_OFFSET,	offsetof (struct sigframe, sc.arm_r0));
    DEFINE(L4_SIGFRAME_R1_OFFSET,	offsetof (struct sigframe, sc.arm_r1));
    DEFINE(L4_SIGFRAME_R2_OFFSET,	offsetof (struct sigframe, sc.arm_r2));
    DEFINE(L4_SIGFRAME_R3_OFFSET,	offsetof (struct sigframe, sc.arm_r3));
    DEFINE(L4_SIGFRAME_R4_OFFSET,	offsetof (struct sigframe, sc.arm_r4));
    DEFINE(L4_SIGFRAME_R5_OFFSET,	offsetof (struct sigframe, sc.arm_r5));
    DEFINE(L4_SIGFRAME_R6_OFFSET,	offsetof (struct sigframe, sc.arm_r6));
    DEFINE(L4_SIGFRAME_R7_OFFSET,	offsetof (struct sigframe, sc.arm_r7));
    DEFINE(L4_SIGFRAME_R8_OFFSET,	offsetof (struct sigframe, sc.arm_r8));
    DEFINE(L4_SIGFRAME_R9_OFFSET,	offsetof (struct sigframe, sc.arm_r9));
    DEFINE(L4_SIGFRAME_R10_OFFSET,	offsetof (struct sigframe, sc.arm_r10));
    DEFINE(L4_SIGFRAME_R11_OFFSET,	offsetof (struct sigframe, sc.arm_fp));
    DEFINE(L4_SIGFRAME_R12_OFFSET,	offsetof (struct sigframe, sc.arm_ip));
    DEFINE(L4_SIGFRAME_SP_OFFSET,	offsetof (struct sigframe, sc.arm_sp));
    DEFINE(L4_SIGFRAME_LR_OFFSET,	offsetof (struct sigframe, sc.arm_lr));
    DEFINE(L4_SIGFRAME_USIG_OFFSET,	offsetof (struct sigframe, usig));
    DEFINE(L4_SIGFRAME_RETLR_OFFSET,	offsetof (struct sigframe, lr));
    DEFINE(L4_SIGFRAME_SIGIP_OFFSET,	offsetof (struct sigframe, sig_ip));

    BLANK();

    DEFINE(L4_SIGRTFRAME_R0_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r0));
    DEFINE(L4_SIGRTFRAME_R1_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r1));
    DEFINE(L4_SIGRTFRAME_R2_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r2));
    DEFINE(L4_SIGRTFRAME_R3_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r3));
    DEFINE(L4_SIGRTFRAME_R4_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r4));
    DEFINE(L4_SIGRTFRAME_R5_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r5));
    DEFINE(L4_SIGRTFRAME_R6_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r6));
    DEFINE(L4_SIGRTFRAME_R7_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r7));
    DEFINE(L4_SIGRTFRAME_R8_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r8));
    DEFINE(L4_SIGRTFRAME_R9_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r9));
    DEFINE(L4_SIGRTFRAME_R10_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_r10));
    DEFINE(L4_SIGRTFRAME_R11_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_fp));
    DEFINE(L4_SIGRTFRAME_R12_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_ip));
    DEFINE(L4_SIGRTFRAME_SP_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_sp));
    DEFINE(L4_SIGRTFRAME_LR_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.arm_lr));

    DEFINE(L4_SIGRTFRAME_PINFO_OFFSET,	offsetof (struct rt_sigframe, pinfo));
    DEFINE(L4_SIGRTFRAME_PUC_OFFSET,	offsetof (struct rt_sigframe, puc));
    DEFINE(L4_SIGRTFRAME_USIG_OFFSET,	offsetof (struct rt_sigframe, usig));
    DEFINE(L4_SIGRTFRAME_RETLR_OFFSET,	offsetof (struct rt_sigframe, lr));
    DEFINE(L4_SIGRTFRAME_SIGIP_OFFSET,	offsetof (struct rt_sigframe, sig_ip));

    BLANK();

    DEFINE(L4_RESTORE_R0_OFFSET,	offsetof (struct sigcontext, arm_r0));
    DEFINE(L4_RESTORE_R1_OFFSET,	offsetof (struct sigcontext, arm_r1));
    DEFINE(L4_RESTORE_R2_OFFSET,	offsetof (struct sigcontext, arm_r2));
    DEFINE(L4_RESTORE_R3_OFFSET,	offsetof (struct sigcontext, arm_r3));
    DEFINE(L4_RESTORE_R4_OFFSET,	offsetof (struct sigcontext, arm_r4));
    DEFINE(L4_RESTORE_R5_OFFSET,	offsetof (struct sigcontext, arm_r5));
    DEFINE(L4_RESTORE_R6_OFFSET,	offsetof (struct sigcontext, arm_r6));
    DEFINE(L4_RESTORE_R7_OFFSET,	offsetof (struct sigcontext, arm_r7));
    DEFINE(L4_RESTORE_R8_OFFSET,	offsetof (struct sigcontext, arm_r8));
    DEFINE(L4_RESTORE_R9_OFFSET,	offsetof (struct sigcontext, arm_r9));
    DEFINE(L4_RESTORE_R10_OFFSET,	offsetof (struct sigcontext, arm_r10));
    DEFINE(L4_RESTORE_R11_OFFSET,	offsetof (struct sigcontext, arm_fp));
    DEFINE(L4_RESTORE_R12_OFFSET,	offsetof (struct sigcontext, arm_ip));
    DEFINE(L4_RESTORE_SP_OFFSET,	offsetof (struct sigcontext, arm_sp));
    DEFINE(L4_RESTORE_LR_OFFSET,	offsetof (struct sigcontext, arm_lr));
    
    return 0; 
}
