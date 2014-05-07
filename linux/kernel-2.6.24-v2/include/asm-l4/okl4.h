#ifndef __L4_OKL4_H_
#define __L4_OKL4_H_

extern L4_SpaceId_t linux_space;

void okl4_map_page(mm_context_t *context, unsigned long virt_addr, unsigned
		long src_addr, unsigned rwx, unsigned long attrib);
void okl4_map_page_kernel(unsigned long virt_addr, unsigned
		long src_addr, unsigned rwx, unsigned long attrib);
void okl4_unmap_page(mm_context_t *context, unsigned long virt_addr);
void okl4_unmap_page_kernel(unsigned long virt_addr);

void okl4_unmap_page_size(mm_context_t *context, unsigned long virt_addr,
		unsigned long page_shift);

L4_ThreadId_t
okl4_create_sys_thread(L4_SpaceId_t space, L4_ThreadId_t pager, L4_ThreadId_t scheduler,
       void *utcb, word_t unit);

L4_ThreadId_t
okl4_create_thread(L4_SpaceId_t space, L4_ThreadId_t pager, L4_ThreadId_t scheduler,
		void *utcb, L4_ThreadId_t *handle_rv);

L4_SpaceId_t
okl4_create_space(L4_Fpage_t utcb_area, L4_Word_t max_prio);

void okl4_delete_thread(L4_ThreadId_t thrd);
void okl4_delete_space(L4_SpaceId_t space);

#if defined(ARM_PID_RELOC)
int okl4_space_set_pid(L4_SpaceId_t space, unsigned int pid);
#endif

#if defined(ARM_SHARED_DOMAINS)
void okl4_share_domain(L4_SpaceId_t space);
void okl4_unshare_domain(L4_SpaceId_t space);
#endif

#endif
