/*
 * arch/l4/kernel/mmap.c
 *
 * Copyright (c) Open Kernel Labs
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 * Revision history:
 *  24 Oct 2007 Hal Ashburner
 *      - Break arch_get_unmapped_area out of main.c
 */

#include <linux/mman.h>
#include <linux/bitmap.h>
#include <asm/range_fl.h>
#include <linux/a.out.h>
#include <linux/spinlock.h>

#include <mmap.h>

/* Changed during early boot? (We hope?) */
pgd_t swapper_pg_dir[1024];

unsigned long shm_align_mask = PAGE_SIZE - 1;	/* Sane caches */
#define SUPER_SHIFT     16
unsigned long super_align_mask = ((1UL<<SUPER_SHIFT) - 1); /* 16k pages */

#define SUPER_ALIGN(addr,pgoff)				\
	((((addr) + (super_align_mask)) & ~super_align_mask) +	\
	 (((pgoff) << SUPER_SHIFT) & super_align_mask))

#ifdef ARM_PID_RELOC
/* 
 * bitmap of available memory for large vm allocator
 * 1024 bits to represent 1024 1MB regions
 */
unsigned long largevm_bitmap[32];

/* lock for largevm_bitmap */
spinlock_t mmap_lock;

/* Initialise the mmap range free list */
void init_largevm_bitmap(void) 
{
	bitmap_release_region(largevm_bitmap, 0, 10);
}

int orderof(unsigned long size)
{
	int bits;
	unsigned long num = size;

	for (bits = 0; size != 0; bits++) {
		size >>= 1;
	}

	if (num - (1 << (bits - 1)))
		bits++;

	return bits;
}

/* round up to the nearest MB */
uintptr_t roundup_mb(uintptr_t val)
{
	if (val & 0x000fffff)
		val = (val & 0xfff00000) + 0x100000;
	return val;
}

/* round down to the nearest MB */
uintptr_t rounddown_mb(uintptr_t val)
{
	return (val & 0xfff00000);
}

/* Leave 1MB for the stack */
#define STACK_BOTTOM   (STACK_TOP - 0x100000) 

/* 0 - 1GB used for PID relocation */
#define PID_RELOC_END  (0x40000000UL)

#define STACK_PID_REGION(addr, len)                \
	((STACK_BOTTOM <= addr && addr < PID_RELOC_END) ||      \
	(STACK_BOTTOM <= (addr+len) && (addr+len) < PID_RELOC_END))

#if defined(CONFIG_CELL)
int okl4_add_window(mm_context_t *context, unsigned long addr)
{
	L4_Fpage_t src_fpage;
	extern L4_SpaceId_t linux_space;
	int ret;

	src_fpage = L4_FpageLog2(addr, 20);
	L4_Set_Rights(&src_fpage, 7);
	L4_Set_Meta(&src_fpage);    // fault via callback

	L4_LoadMR(1, src_fpage.raw);

	L4_LoadMR(0, context->space_id.raw);
	ret = L4_MapControl(linux_space, L4_MapCtrl_MapWindow);
        return (ret == 0);
}

void okl4_remove_window(mm_context_t *context, unsigned long addr)
{
	L4_Fpage_t src_fpage;
	extern L4_SpaceId_t linux_space;
	int ret;

	src_fpage = L4_FpageLog2(addr, 20);
	L4_Set_Rights(&src_fpage, 0);
	L4_Set_Meta(&src_fpage);    // fault via callback

	L4_LoadMR(1, src_fpage.raw);

	L4_LoadMR(0, context->space_id.raw);
	ret = L4_MapControl(linux_space, L4_MapCtrl_MapWindow);
}
#endif
#endif

unsigned long arch_get_unmapped_area(struct file *filp, unsigned long addr,
	unsigned long len, unsigned long pgoff, unsigned long flags)
{
	struct vm_area_struct * vmm;
	int do_color_align;

	if (flags & MAP_FIXED) {
		/*
		 * We do not accept a shared mapping if it would violate
		 * cache aliasing constraints.
		 */
		if ((flags & MAP_SHARED) && (addr & shm_align_mask))
			return -EINVAL;
		return addr;
	}

	if (len > TASK_SIZE)
		return -ENOMEM;

	do_color_align = 0;
	if (filp || (flags & MAP_SHARED))
		do_color_align = 1;

#ifdef ARM_PID_RELOC
	if (addr) {
		if (do_color_align)
			addr = SUPER_ALIGN(addr, pgoff);
		else
			addr = PAGE_ALIGN(addr);
		vmm = find_vma(current->mm, addr);
		if (TASK_SIZE - len >= addr &&
			(!vmm || addr + len <= vmm->vm_start)) {
			/* Don't overwrite the stack or PID relocation region */
			if (!STACK_PID_REGION(addr,len)) {
				if (addr >= SIZE_32MB) {
					uintptr_t res;
					/* 
					 * try to allocate from the address
					 * space specific mmap range free list
					 */
					res = rfl_alloc_specific_range(current->mm->context.mmap_free_list,
													addr, len);
					if (res)
						return res;
				} else {
					return addr;
				}
			}
		}
	}
	addr = TASK_UNMAPPED_BASE;

	if (do_color_align)
		addr = SUPER_ALIGN(addr, pgoff);
	else
		addr = PAGE_ALIGN(addr);

	for (vmm = find_vma(current->mm, addr); ; vmm = vmm->vm_next) {
		/* At this point:  (!vmm || addr < vmm->vm_end). */
		if (TASK_SIZE - len < addr) {
			return -ENOMEM;
		}
		/* 
		 * Using this address will overwrite the stack 
		 * or the PID relocation region 
		 */
		if (STACK_PID_REGION(addr,len)) {
			addr = SIZE_1GB;
		}

		if (addr >= SIZE_32MB) {
			uintptr_t res;
			/* 
			 * First try to allocate from our address
			 * space specific mmap range free list
			 */
			res = rfl_find_free_range(current->mm->context.mmap_free_list, len);
			if (res) {
				return res;
			} else {
				int i, order;

				order = orderof(roundup_mb(len) >> 20);

				/* 
				 * not enough space in free list, grab memory from 
				 * the largevm bitmap 
				 */
				spin_lock(&mmap_lock);
				res = (uintptr_t)bitmap_find_free_region(largevm_bitmap, 1024, order);
				spin_unlock(&mmap_lock);
				/* 
				 * If there is space in the global bitmap, add that 
				 * range to our local mmap range free list
				 */
				if ((long)res >= 0) {
					res = LARGEVM_START + (res * (1 << 20));
					rfl_insert_range(current->mm->context.mmap_free_list,
										res, res + ((1<<order)*(1<<20)) - 1);
					for (i=0; i<(1<<order); i++) {
						okl4_add_window(&current->mm->context, res + (i*(1<<20)));
					}
				}
				return res;
			}
		}

		if (!vmm || addr + len <= vmm->vm_start) {
			return addr;
		}
		addr = vmm->vm_end;
		if (do_color_align)
			addr = SUPER_ALIGN(addr, pgoff);
	}
#else
	if (addr) {
		if (do_color_align)
			addr = SUPER_ALIGN(addr, pgoff);
		else
			addr = PAGE_ALIGN(addr);
		vmm = find_vma(current->mm, addr);
		if (TASK_SIZE - len >= addr &&
			(!vmm || addr + len <= vmm->vm_start)) {
			return addr;
		}
	}
	addr = TASK_UNMAPPED_BASE;

	if (do_color_align)
		addr = SUPER_ALIGN(addr, pgoff);
	else
		addr = PAGE_ALIGN(addr);

	for (vmm = find_vma(current->mm, addr); ; vmm = vmm->vm_next) {
		/* At this point:  (!vmm || addr < vmm->vm_end). */
		if (TASK_SIZE - len < addr)
			return -ENOMEM;
		if (!vmm || addr + len <= vmm->vm_start) {
			return addr;
		}
		addr = vmm->vm_end;
		if (do_color_align)
			addr = SUPER_ALIGN(addr, pgoff);
	}
#endif
}
