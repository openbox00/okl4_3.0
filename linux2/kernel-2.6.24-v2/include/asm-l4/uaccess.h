/* 
 * Copyright (C) 2002 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#ifndef __L4_UACCESS_H
#define __L4_UACCESS_H

#include <asm-generic/uaccess.h>

#define VERIFY_READ 0
#define VERIFY_WRITE 1

#define MAKE_MM_SEG(s)	((mm_segment_t) { (s) })

#define KERNEL_DS	MAKE_MM_SEG(0)
#define USER_DS		MAKE_MM_SEG(_TIF_USER_OR_KERNEL)

#define get_ds()	(KERNEL_DS)
#define get_fs()	MAKE_MM_SEG(current_thread_info()->flags & _TIF_USER_OR_KERNEL)
#define set_fs(x)   current_thread_info()->flags =		\
			((current_thread_info()->flags & ~_TIF_USER_OR_KERNEL) | (x.seg))

#define segment_eq(a, b) ((a).seg == (b).seg)

#define __copy_from_user(to, from, n) copy_from_user(to, from, n)
#define __copy_to_user(to, from, n) copy_to_user(to, from, n)

#define __copy_to_user_inatomic __copy_to_user
#define __copy_from_user_inatomic __copy_from_user

#define __get_user(x, ptr) \
({ \
        const __typeof__(ptr) __private_ptr = ptr; \
        __typeof__(*(__private_ptr)) __private_val; \
        int __private_ret = -EFAULT; \
        (x) = 0; \
       if (__copy_from_user((void*)&__private_val, (__private_ptr), \
           sizeof(*(__private_ptr))) == 0) {\
               (x) = (__typeof__(*(__private_ptr))) __private_val; \
               __private_ret = 0; \
       } \
        __private_ret; \
})


 

#define get_user(x, ptr) \
({ \
        const __typeof__(*(ptr)) *private_ptr = (ptr); \
        (access_ok(VERIFY_READ, private_ptr, sizeof(*private_ptr)) ? \
	 __get_user(x, private_ptr) : ((x) = 0, -EFAULT)); \
})

#define __put_user(x, ptr) \
({ \
	__typeof__(ptr) __private_ptr = ptr;	     \
	__typeof__(*(__private_ptr)) __private_val;  \
	int __private_ret = -EFAULT;			     \
	__private_val = (__typeof__(*(__private_ptr))) (x);  \
	if (__copy_to_user((__private_ptr), &__private_val,   \
			   sizeof(*(__private_ptr))) == 0) {  \
		__private_ret = 0;			      \
	}						      \
	__private_ret;					      \
})

#define put_user(x, ptr) \
({ \
        __typeof__(*(ptr)) *private_ptr = (ptr); \
        (access_ok(VERIFY_WRITE, private_ptr, sizeof(*private_ptr)) ? \
	 __put_user(x, private_ptr) : -EFAULT); \
})

#define copy_in_user(to,from,n)		({	BUG(); 0; })
#define __copy_in_user(to,from,n)	({	BUG(); 0; })

extern long strnlen_user (const char* src, long len);

#define strlen_user(str) strnlen_user(str, ~0UL >> 1)


struct exception_table_entry
{
	unsigned long insn;
	unsigned long fixup;
};

extern long clear_user(void *to, unsigned long n);
extern long __clear_user (void *to, unsigned long n);
extern unsigned long copy_from_user(void *to, const void *from,
				    unsigned long size);
extern unsigned long copy_to_user(void *to, const void *from,
				    unsigned long size);

extern long strncpy_from_user(char *dst, const char *src, long count);

static inline int verify_area(int type, const void * addr, unsigned long size)
{
	return 0;
}

#define access_ok(type,addr,size) 1

#endif


/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-file-style: "linux"
 * End:
 */
