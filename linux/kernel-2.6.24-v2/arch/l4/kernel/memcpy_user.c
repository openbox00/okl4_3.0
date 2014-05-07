
#if !defined(ARM_SHARED_DOMAINS) && !defined(CONFIG_DIRECT_VM)


#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/io.h>

/****

Handling of accessing data from USER space

*****/

extern int l4_do_page_fault(unsigned long address, long access, struct pt_regs *regs);

pte_t * lookup_pte(pgd_t *page_dir, unsigned long pf_address);
#define PF_EXECUTE (1)
#define PF_WRITE (2)
#define PF_READ (4)

//#define DEBUG_PARSE_PTABS

void flush_range_invalidate(L4_SpaceId_t spc, unsigned long start, unsigned long end)
{
#ifndef CONFIG_ARCH_I386
#ifdef ARM_PID_RELOC
	if (start < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		start += offset;
		end += offset;
	}
#endif
	/* We don't invalidate since it is currently a priviledged operation */
	//L4_CacheFlushRangeInvalidate( t, start, end);
	L4_CacheFlushRange(spc, start, end );
#endif
}

void flush_range(L4_SpaceId_t spc, unsigned long start, unsigned long end)
{
#ifndef CONFIG_ARCH_I386
#ifdef ARM_PID_RELOC
	if (start < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		start += offset;
		end += offset;
	}
#endif
	L4_CacheFlushRange( spc, start, end );
#endif
}

void flush_range_invalidate_kernel(unsigned long start, unsigned long end)
{
#ifndef CONFIG_ARCH_I386
	/* We don't invalidate since it is currently a priviledged operation */
	//L4_CacheFlushRangeInvalidate( t, start, end);
	L4_CacheFlushRange( L4_nilspace, start, end );
#endif
}

void flush_range_kernel(unsigned long start, unsigned long end)
{
#ifndef CONFIG_ARCH_I386
	L4_CacheFlushRange( L4_nilspace, start, end );
#endif
}

unsigned long
parse_ptabs(unsigned long address, unsigned long *offset, unsigned long access)
{
	pte_t *ptep = lookup_pte((pgd_t *)current->mm->pgd, address);
	struct thread_info *curinfo = current_thread_info();

#ifdef DEBUG_PARSE_PTABS
	printk("pp: pdir: %p, address: %lx, ptep: %p\n", 
	       (pgd_t *)current->mm->pgd, address, ptep);
#endif

    if ((ptep == NULL) || !pte_present(*ptep) ||
        ((access == PF_WRITE) && !pte_write(*ptep)))
    {
		unsigned long mode = user_mode_status(curinfo);

		set_usermode_status_false(curinfo);

		curinfo->regs.mode++;

		if (l4_do_page_fault(address, access, &curinfo->regs) == -1)
		{
			curinfo->regs.mode--;
			set_usermode_status(curinfo, mode);
			return -EFAULT;
		}
		curinfo->regs.mode--;
		set_usermode_status(curinfo, mode);

		if (ptep == NULL)
			ptep = lookup_pte((pgd_t *)current->mm->pgd, address);
		if (!pte_present(*ptep) || !pte_read(*ptep))
			panic("parse_ptabs: pte page still not present\n");
	}
	*ptep   = pte_mkyoung(*ptep);
	*offset = address & (~PAGE_MASK);
#ifdef DEBUG_PARSE_PTABS
	printk("pp: return %016lx\n",
			(unsigned long)phys_to_virt(pte_pfn(*ptep) << PAGE_SHIFT));
#endif
	return (unsigned long)phys_to_virt(pte_pfn(*ptep) << PAGE_SHIFT);
}


//#define DEBUG_MEMCPY_FROMFS

/*
 * This is an optimised strncpy/strcpy hybrid for __strncpy_from_user_page.
 * We copy until we hit a null or count is reached but do not fill null's
 * after the null is found (if count is higher than the source string length).
 */
static inline
void __do_strncpy_from_user_page(char *dest, const char *src, size_t count) {
	while (count-- && (*dest++ = *src++))
		; // Do nothing
}

static
int __strncpy_from_user_page(char * to, const char * from,
			     unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("%s: to: %p, from: %p, len: %08lx\n",
	       __func__, to, from, n);
#endif 
	page = parse_ptabs((unsigned long)from, &offset, PF_READ);
	if (page != -EFAULT) {
#ifdef DEBUG_MEMCPY_FROMFS
		printk("    %s reading from: %08lx\n",
		       __func__, (page + offset));
#endif 
		flush_range(current->mm->context.space_id, (unsigned long)from, (unsigned long)from+n);

		__do_strncpy_from_user_page(to, (char *)(page + offset), n);

		flush_range_invalidate_kernel(page + offset, page + offset + n);
		return 0;
	}

	return -EFAULT;
}


static
int __memcpy_from_user_page(char * to, const char * from,
			     unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("%s: to: %p, from: %p, len: %08lx\n",
	       __func__, to, from, n);
#endif 
	page = parse_ptabs((unsigned long)from, &offset, PF_READ);
	if (page != -EFAULT) {
#ifdef DEBUG_MEMCPY_FROMFS
		printk("    %s reading from: %08lx\n",
		       __func__, (page + offset));
#endif
		flush_range(current->mm->context.space_id, (unsigned long)from, (unsigned long)from + n);

		memcpy(to, (char *)(page + offset), n);

		flush_range_invalidate_kernel(page + offset, page + offset + n);
		return 0;
	}

	return -EFAULT;
}

static
int __memcpy_to_user_page(char * to, const char * from,
			     unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("%s: to: %p, from: %p, len: %08lx\n",
	       __func__, to, from, n);
#endif 
	page = parse_ptabs((unsigned long)to, &offset, PF_WRITE);
	if (page != -EFAULT) {
#ifdef DEBUG_MEMCPY_FROMFS
		printk("    %s writing to: %08lx\n",
		       __func__, (page + offset));
#endif 
//printk("%s %p -> %p[%p] (%ld)\n", __func__, from, to, (char *)(page + offset), n);
		flush_range( current->mm->context.space_id, (unsigned long)to, (unsigned long)to + n);

		memcpy((char *)(page + offset), from, n);

		flush_range_kernel(page + offset, page + offset + n);

		return 0;
	}

	return -EFAULT;
}

long
strncpy_from_user(char *dst, const char *src, long count)
{
	unsigned long copy_size = (unsigned long)src & (~PAGE_MASK);
	int res;
	unsigned long n = count;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		strncpy(dst, src, count);
		return strlen(src);
	}

	if (copy_size) {
		copy_size = min(PAGE_SIZE - copy_size, n);
		res = __strncpy_from_user_page(dst, src, copy_size);
		n -= copy_size;
		if (res == -EFAULT) {
#ifdef DEBUG_STRNCPY_USER
			printk("strncpy_from_user EFAULT\n");
#endif
			return -EFAULT;
		}
		if ((res = strnlen(dst, copy_size)) < copy_size)
			return res;
	}
	while (n) {
		src +=copy_size;
		dst += copy_size;
		copy_size = min((unsigned long)PAGE_SIZE, n);
		res = __strncpy_from_user_page(dst, src, copy_size);
		if (res == -EFAULT) {
#ifdef DEBUG_STRNCPY_USER
			printk("strncpy_from_user EFAULT...\n");
#endif
			return -EFAULT;
		}
		if ((res = strnlen(dst, copy_size)) < copy_size)
			return (count-n)+res;
		n -= copy_size;
	}
	return count;
}

//#define DEBUG_CLEAR_IN_PAGE

static
int __clear_in_user_page(char * addr, unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_CLEAR_IN_PAGE
	printk("%s: addr: %p, len: %08lx\n",
	       __func__, addr, n);
#endif 
	page = parse_ptabs((unsigned long)addr, &offset, PF_WRITE);

	if ((offset + n) > PAGE_SIZE)
	    return -EFAULT;

	if (page != -EFAULT) {
#ifdef DEBUG_CLEAR_IN_PAGE
		printk("    %s clearing from: %08lx to %08lx\n",
		       __func__, (page + offset), (page + offset + n));
#endif 
		flush_range( current->mm->context.space_id, (unsigned long)addr, (unsigned long)addr + n);

		memset((char *)(page + offset), 0, n);

		flush_range_kernel(page + offset, page + offset + n);

		return 0;
	}

	return -EFAULT;
}

extern inline int __strnlen_from_user_page(const char *from,
					   unsigned long n, unsigned long *len)
{
	unsigned long page, offset;
	int res;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("%s from: %p, len: %08lx\n", __func__, from, n);
#endif 
	page = parse_ptabs((unsigned long)from, &offset, PF_READ);
	if (page != -EFAULT) {
#ifdef DEBUG_MEMCPY_FROMFS
		printk("    %s reading from: %08lx\n",
		       __func__, (page + offset));
#endif
		flush_range( current->mm->context.space_id, (unsigned long)from, (unsigned long)from + n);

		res = strnlen((char*)(page + offset), n);

		flush_range_invalidate_kernel(page + offset, page + offset + n);
		/* after finishing the search operation end is either
		 * - zero: max number of bytes searched
		 * - non zero: end of string reached, res containing
		 *      the number of remaining bytes
		 */
		*len += res;
		return (res != n);
	}
	return -EFAULT;
}

//#define DEBUG_MEMCPY_KERNEL

/* strnlen returns the number of bytes in a string. We calculate the number
 * simply by substracting the number of bytes remaining from the
 * maximal length. The number of bytes remaining is (n + res) with n
 * beeing the number of bytes to copy from the next pages and res the
 * number of remaining bytes after reaching the '\0' */
long strnlen_user(const char *src, long n)
{
	unsigned long search_size = PAGE_SIZE - ((unsigned long)src & (~PAGE_MASK));
	int res;
	unsigned long len=0;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("strnlen_user called from: %08lx, src: %p, %ld\n", 
	       (long) __builtin_return_address(0), src, n);
#endif 

	if (segment_eq(get_fs(), KERNEL_DS))
	{
		len = strnlen(src, n);
#ifdef DEBUG_MEMCPY_KERNEL
		printk("kernel strnlen_user %p, %ld = %ld\n", src, n, len);
#endif
		return len + 1;
	}

	if (!search_size)
		search_size = PAGE_SIZE;
	if (search_size>n)
		search_size = n;
	
	while (n > 0) {
		res = __strnlen_from_user_page(src, search_size, &len);
		if (res == -EFAULT)
			return 0; /* EFAULT */
		else if (res)
			return len + 1;
		
		src += search_size;
		n   -= search_size;
		search_size = PAGE_SIZE;
	}
	
	return 0; /* EFAULT */
}


void copy_user_page(void *vto, void *vfrom, unsigned long vaddr,
	struct page *to)
{
	vaddr = vaddr & PAGE_MASK;     /* align vaddr */
	flush_range_invalidate( current->mm->context.space_id, vaddr, vaddr + PAGE_SIZE);

	memcpy(vto, vfrom, PAGE_SIZE);

	flush_range_kernel((unsigned long)vto, (unsigned long)vto + PAGE_SIZE);
	flush_range_kernel((unsigned long)vfrom, (unsigned long)vfrom + PAGE_SIZE);
}

extern void clear_user_page(void *addr, unsigned long vaddr,
	struct page *page)
{
	//vaddr = vaddr & PAGE_MASK;     /* align vaddr */

	memset(addr, 0, PAGE_SIZE);

	flush_range_kernel((unsigned long)addr, (unsigned long)addr + PAGE_SIZE);
}

long clear_user(void __user *to, unsigned long n)
{
	return __clear_user(to, n);
}

/*
 * XXX In theory this should be much quicker but for now it isn't.
 */
long __clear_user (void __user *to, unsigned long n)
{
	unsigned long clear_size = (unsigned long)to & (~PAGE_MASK);
	int res;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		memset(to, 0, n);
		return 0;
	}

	if (clear_size) {
		clear_size = min(PAGE_SIZE - clear_size, n);
		res = __clear_in_user_page(to, clear_size);
		if (res != 0) {
			goto clear_user_out;
		}
		n -= clear_size;
	}
	while (n) {
		to = (void*) ((unsigned long)to + clear_size);
		clear_size = min((unsigned long)PAGE_SIZE, n);
		res = __clear_in_user_page(to, clear_size);
		if (res != 0) {
			goto clear_user_out;
		}
		n -= clear_size;
	}
clear_user_out:
	return 0;
}

unsigned long
copy_from_user(void *dst, const void __user *src,
	       unsigned long count)
{
	unsigned long copy_size = (unsigned long)src & (~PAGE_MASK);
	int res;
	unsigned long n = count;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		memcpy(dst, src, count);
		return 0;
	}

	if (copy_size) {
		copy_size = min(PAGE_SIZE - copy_size, n);
		res = __memcpy_from_user_page(dst, src, copy_size);
		if (res != 0) {
			return n;
		}
		n -= copy_size;
	}
	while (n) {
		src = (void*)((unsigned long)src + copy_size);
		dst = (void*)((unsigned long)dst + copy_size);
		copy_size = min((unsigned long)PAGE_SIZE, n);
		res = __memcpy_from_user_page(dst, src, copy_size);
		if (res != 0) {
			return n;
		}
		n -= copy_size;
	}
	return 0;
}

unsigned long
copy_to_user(void __user *dst, const void *src,
	     unsigned long count)
{
	unsigned long copy_size = (unsigned long)dst & (~PAGE_MASK);
	int res;
	unsigned long n = count;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		memcpy(dst, src, count);
		return 0;
	}

	if (copy_size) {
		copy_size = min(PAGE_SIZE - copy_size, n);
		res = __memcpy_to_user_page(dst, src, copy_size);
		if (res != 0) {
			goto copy_to_user_out;
		}
		n -= copy_size;
	}
	while (n) {
		src = (void*)((unsigned long)src + copy_size);
		dst = (void*)((unsigned long)dst + copy_size);
		copy_size = min((unsigned long)PAGE_SIZE, n);
		res = __memcpy_to_user_page(dst, src, copy_size);
		if (res != 0) {
			goto copy_to_user_out;
		}
		n -= copy_size;
	}
copy_to_user_out:
	return n;
}

#endif
