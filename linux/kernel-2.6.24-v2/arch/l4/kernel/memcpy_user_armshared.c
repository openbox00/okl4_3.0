#if defined(ARM_SHARED_DOMAINS) || defined(CONFIG_DIRECT_VM)


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

extern int run_with_fault_handler(char *arg1, char *arg2, unsigned long arg3, void (*func), int type);

extern int l4_do_page_fault(unsigned long address, long access, struct pt_regs *regs);

pte_t * lookup_pte(pgd_t *page_dir, unsigned long pf_address);
#define PF_EXECUTE (1)
#define PF_WRITE (2)
#define PF_READ (4)

int strncpy_user(char * dest, const char *src, size_t count);

//#define DEBUG_PARSE_PTABS

void flush_range_invalidate(L4_SpaceId_t spc, unsigned long start, unsigned long end)
{
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
	L4_CacheFlushRange( spc, start, end );
}

void flush_range(L4_SpaceId_t spc, unsigned long start, unsigned long end)
{
#ifdef ARM_PID_RELOC
	if (start < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		start += offset;
		end += offset;
	}
#endif
	L4_CacheFlushRange( spc, start, end );
}

void flush_range_invalidate_kernel(unsigned long start, unsigned long end)
{
	/* We don't invalidate since it is currently a priviledged operation */
	//L4_CacheFlushRangeInvalidate( t, start, end);
	L4_CacheFlushRange( L4_nilspace /* linux space */, start, end );
}

void flush_range_kernel(unsigned long start, unsigned long end)
{
	L4_CacheFlushRange( L4_nilspace /* linux space */, start, end );
}

/*
 * This is called if we fault on the user's address
 */
int fault_handler(unsigned long access)
{
	unsigned long fault_address = __L4_ARM_Utcb()[__L4_TCR_KERNEL_RESERVED0];
	struct thread_info *curinfo = current_thread_info();
	int ret;

	unsigned long mode = user_mode_status(curinfo);

#ifdef ARM_PID_RELOC
	L4_Word_t offset = (unsigned long)current->mm->context.pid << 25;

	if (fault_address >= offset && (fault_address < (offset+0x2000000)))
	{
		fault_address -= offset;
	}
#endif

	set_usermode_status_false(curinfo);
	curinfo->regs.mode ++;

	ret = l4_do_page_fault(fault_address, access, &curinfo->regs);

	curinfo->regs.mode --;
	set_usermode_status(curinfo, mode);

	if (unlikely(ret == -1))
	{
		return -EFAULT;
	}

	return 0;
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
	    ((access == PF_WRITE) && !pte_write(*ptep))) {
		unsigned long mode = user_mode_status(curinfo);
		int ret;

		set_usermode_status_false(curinfo);
		curinfo->regs.mode ++;

		ret = l4_do_page_fault(address, access, &curinfo->regs);

		curinfo->regs.mode --;
		if (ret == -1)
		{
			return -EFAULT;
		}
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

static
int __strncpy_from_user_page(char * to, const char * from,
			     unsigned long n)
{
	int res;

#ifdef ARM_PID_RELOC
	if ((unsigned long)from < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		from += offset;
	}
#endif
	if ((unsigned long)from >= TASK_SIZE)
		return -EFAULT;
	res = run_with_fault_handler(to, (char *)from, n, strncpy_user, PF_READ);

	return res;
}


static
int __memcpy_from_user_page(char * to, const char * from,
			     unsigned long n)
{
	int res;

#ifdef ARM_PID_RELOC
	if ((unsigned long)from < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		from += offset;
	} else
#endif
	if ((unsigned long)from >= TASK_SIZE)
		return -EFAULT;
	res = run_with_fault_handler(to, (char *)from, n, memcpy, PF_READ);

	if (res == -EFAULT)
		return res;

	return 0;
}

static
int __memcpy_to_user_page(char * to, const char * from,
			     unsigned long n)
{
	int res;

#ifdef ARM_PID_RELOC
	if ((unsigned long)to < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		to += offset;
	} else
#endif
	if ((unsigned long)to >= TASK_SIZE) {
		return -EFAULT;
	}
	res = run_with_fault_handler(to, (char *)from, n, memcpy, PF_WRITE);

	if (res == -EFAULT)
		return res;

	return 0;
}

long
strncpy_from_user(char *dst, const char *src, long count)
{
	unsigned long copy_size = (unsigned long)src & (~PAGE_MASK);
	int res;
	unsigned long n = count;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		return strncpy_user(dst, src, count);
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
		if (res < copy_size)
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
		if (res < copy_size)
			return (count-n)+res;
		n -= copy_size;
	}
	return count;
}

//#define DEBUG_CLEAR_IN_PAGE

static
int __clear_in_user_page(char * addr, unsigned long n)
{
	int res;

#ifdef ARM_PID_RELOC
	if ((unsigned long)addr < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		addr += offset;
	} else
#endif
	if ((unsigned long)addr >= TASK_SIZE)
		return -EFAULT;
	res = run_with_fault_handler(addr, 0, n, memset, PF_WRITE);

	if (res == -EFAULT)
		return res;

	return 0;
}

extern inline int __strnlen_from_user_page(const char *from,
					   unsigned long n, unsigned long *len)
{
	int res;

#ifdef ARM_PID_RELOC
	if ((unsigned long)from < 0x2000000UL)
	{
		unsigned long offset = (unsigned long)current->mm->context.pid << 25;
		from += offset;
	} else
#endif
	if ((unsigned long)from >= TASK_SIZE)
		return -EFAULT;
	res = run_with_fault_handler((char *)from, (char *)n, n, strnlen, PF_READ);

	if (res == -EFAULT)
		return res;

	*len += res;

	return (res != n);
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
}

extern void clear_user_page(void *addr, unsigned long vaddr,
	struct page *page)
{
	memset(addr, 0, PAGE_SIZE);

	flush_range_kernel((unsigned long)addr, (unsigned long)addr + PAGE_SIZE);
}

long clear_user(void __user *to, unsigned long n)
{
	return __clear_user(to, n);
}

/*
 * XXX In theory this should be much quicker due to less checking but
 * for now it isn't -gl
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

/* COPY Loops */

/**
 * strncpy_user - Copy a length-limited, %NUL-terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @count: The maximum number of bytes to copy
 *
 * The result is not %NUL-terminated if the source exceeds
 * @count bytes.
 */
int strncpy_user(char * dest, const char *src, size_t count)
{
	char *tmp = dest;
	size_t len = count;

	while (count) {
		char c = *src++;
		*tmp++ = c;

		if (!c) {
		    break;
		}
		count--;
	}

	return len - count;
}

#endif
