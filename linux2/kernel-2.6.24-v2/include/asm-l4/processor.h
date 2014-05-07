#ifndef _L4_PROCESSOR_H_
#define _L4_PROCESSOR_H_

#include <asm/macros.h>
#include <l4.h>

typedef struct {
	unsigned long seg;
} mm_segment_t;

struct task_struct;

#define current_text_addr() ((void *) 0)

extern long kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);

/* Free all resources held by a thread. */
extern void release_thread(struct task_struct *);

#define thread_saved_pc(tsk) ((unsigned long) 0)
#define prepare_to_copy(tsk)        do { } while (0)

struct pt_regs;

extern void start_thread(struct pt_regs * regs, unsigned long pc, unsigned long sp);

extern inline void release_thread (struct task_struct *dead_task) { } 

extern inline void exit_thread (void) { } 


#define KSTK_EIP(p) (0)
#define KSTK_ESP(p) (0)

#define get_wchan(p) (0)

#include INC_SYSTEM2(processor.h)

#endif /* _L4_PROCESSOR_H_ */
