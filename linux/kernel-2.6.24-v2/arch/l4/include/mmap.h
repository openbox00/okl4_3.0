#ifndef _MMAP_H
#define _MMAP_H

#include <linux/types.h>
#include <linux/bitmap.h>
#include <linux/spinlock.h>

/* largevm from 1gb - TASK_SIZE */
#define LARGEVM_START		0x40000000UL
#define LARGEVM_END			TASK_SIZE

#define SIZE_32MB	0x2000000UL
#define SIZE_1GB	0x40000000UL

extern unsigned long largevm_bitmap[];

extern spinlock_t mmap_lock;

extern void init_largevm_bitmap(void);

extern int orderof(unsigned long size);

extern uintptr_t roundup_mb(uintptr_t len);

extern uintptr_t rounddown_mb(uintptr_t val);

#if defined(CONFIG_CELL)
int okl4_add_window(mm_context_t *, unsigned long addr);
void okl4_remove_window(mm_context_t *, unsigned long addr);
#else
inline int okl4_add_window(mm_context_t *context, unsigned long addr)
{
#if defined(ARM_SHARED_DOMAINS)
	return eas_add_window(context->eas, addr, 20);
#else
	return 0;
#endif
}
inline void okl4_remove_window(mm_context_t *context, unsigned long addr)
{
#if defined(ARM_SHARED_DOMAINS)
	eas_remove_window(context->eas, addr, 20);
#endif
}
#endif

#endif /* _MMAP_H */
