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
//#include <asm/signal.h>
//#include <asm/ucontext.h>
//#include INC_SYS(sigframe.h)

#define DEFINE(sym, val) \
        asm volatile("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() asm volatile("\n->" : : )

int main(void)
{
#if 0
    DEFINE(SIZEOF_SIGFRAME,	sizeof (struct sigframe));
    DEFINE(SIZEOF_SIGRTFRAME,	sizeof (struct rt_sigframe));
    
    BLANK();

    DEFINE(L4_SIGFRAME_SIGIP_OFFSET,	offsetof (struct sigframe, sig_ip));
    DEFINE(L4_SIGRTFRAME_SIGIP_OFFSET,	offsetof (struct rt_sigframe, sig_ip));

    DEFINE(L4_SIGFRAME_EDI_OFFSET,	offsetof (struct sigframe, sc.edi));
    DEFINE(L4_SIGFRAME_ESI_OFFSET,	offsetof (struct sigframe, sc.esi));
    DEFINE(L4_SIGFRAME_EBP_OFFSET,	offsetof (struct sigframe, sc.ebp));
    DEFINE(L4_SIGFRAME_EAX_OFFSET,	offsetof (struct sigframe, sc.eax));
    DEFINE(L4_SIGFRAME_EBX_OFFSET,	offsetof (struct sigframe, sc.ebx));
    DEFINE(L4_SIGFRAME_ECX_OFFSET,	offsetof (struct sigframe, sc.ecx));
    DEFINE(L4_SIGFRAME_EDX_OFFSET,	offsetof (struct sigframe, sc.edx));

    DEFINE(L4_SIGRTFRAME_EDI_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.edi));
    DEFINE(L4_SIGRTFRAME_ESI_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.esi));
    DEFINE(L4_SIGRTFRAME_EBP_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.ebp));
    DEFINE(L4_SIGRTFRAME_EAX_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.eax));
    DEFINE(L4_SIGRTFRAME_EBX_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.ebx));
    DEFINE(L4_SIGRTFRAME_ECX_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.ecx));
    DEFINE(L4_SIGRTFRAME_EDX_OFFSET,	offsetof (struct rt_sigframe, uc.uc_mcontext.edx));

    DEFINE(L4_RESTORE_EDI_OFFSET,	offsetof (struct sigcontext, edi));
    DEFINE(L4_RESTORE_ESI_OFFSET,	offsetof (struct sigcontext, esi));
    DEFINE(L4_RESTORE_EBP_OFFSET,	offsetof (struct sigcontext, ebp));
    DEFINE(L4_RESTORE_EAX_OFFSET,	offsetof (struct sigcontext, eax));
    DEFINE(L4_RESTORE_EBX_OFFSET,	offsetof (struct sigcontext, ebx));
    DEFINE(L4_RESTORE_ECX_OFFSET,	offsetof (struct sigcontext, ecx));
    DEFINE(L4_RESTORE_EDX_OFFSET,	offsetof (struct sigcontext, edx));
    DEFINE(L4_RESTORE_ESP_OFFSET,	offsetof (struct sigcontext, esp));
    
#endif
    return 0; 
}
