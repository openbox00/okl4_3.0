/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		MIPS specific IP/TCP/UDP checksumming routines
 *
 * Authors:	Ralf Baechle, <ralf@waldorf-gmbh.de>
 *		Lots of code moved from tcp.c and ip.c; see those files
 *		for more names.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * $Id: checksum.c,v 1.2 2004/03/16 22:03:02 cvansch Exp $
 */
#include <linux/module.h>
#include <linux/types.h>

#include <net/checksum.h>
#include <asm/byteorder.h>
#include <asm/string.h>
#include <asm/uaccess.h>

static inline unsigned short from32to16(unsigned int x)
{
	/* 32 bits --> 16 bits + carry */
	x = (x & 0xffff) + (x >> 16);
	/* 16 bits + carry --> 16 bits including carry */
	x = (x & 0xffff) + (x >> 16);
	return (unsigned short)x;
}

static inline unsigned int do_csum(const unsigned char * buff, int len)
{
	int odd, count;
	unsigned int result = 0;

	if (len <= 0)
		goto out;
	odd = 1 & (unsigned long) buff;
	if (odd) {
		result = be16_to_cpu(*buff);
		len--;
		buff++;
	}
	count = len >> 1;		/* nr of 16-bit words.. */
	if (count) {
		if (2 & (unsigned long) buff) {
			result += *(unsigned short *) buff;
			count--;
			len -= 2;
			buff += 2;
		}
		count >>= 1;		/* nr of 32-bit words.. */
		if (count) {
			unsigned int carry = 0;
			do {
				unsigned int w = *(unsigned int *) buff;
				count--;
				buff += 4;
				result += carry;
				result += w;
				carry = (w > result);
			} while (count);
			result += carry;
			result = (result & 0xffff) + (result >> 16);
		}
		if (len & 2) {
			result += *(unsigned short *) buff;
			buff += 2;
		}
	}
	if (len & 1)
		result += le16_to_cpu(*buff);
	result = from32to16(result);
	if (odd)
		result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
	return result;
}

/*
 * computes a partial checksum, e.g. for TCP/UDP fragments
 */
__wsum csum_partial(const void *buff, int len, __wsum sum)
{
	unsigned int result = do_csum(buff, len);

	/* add in old sum, and carry.. */
	result += sum;
	if(sum > result)
		result += 1;
	return result;
}

EXPORT_SYMBOL(csum_partial);

#ifndef CONFIG_ARCH_I386
/*
 * copy while checksumming, otherwise like csum_partial
 */
__wsum csum_partial_copy_nocheck(const void *src, void *dst, 
				       int len,
#if defined(CONFIG_ARCH_ARM)
					__wsum sum
#else
					unsigned int sum
#endif
)
{
	/*
	 * It's 2:30 am and I don't feel like doing it real ...
	 * This is lots slower than the real thing (tm)
	 */
	sum = csum_partial(src, len, sum);
	memcpy(dst, src, len);

	return sum;
}
EXPORT_SYMBOL(csum_partial_copy_nocheck);

/*
 * Copy from userspace and compute checksum.  If we catch an exception
 * then zero the rest of the buffer.
 */
__wsum csum_partial_copy_from_user (const void *src, void *dst,
                                          int len,
#if defined(CONFIG_ARCH_ARM)
					__wsum sum,
#else
					unsigned int sum,
#endif
                                          int *err_ptr)
{
	int missing;

	missing = copy_from_user(dst, src, len);
	if (missing) {
		memset(dst + len - missing, 0, missing);
		*err_ptr = -EFAULT;
	}
		
	return csum_partial(dst, len, sum);
}

EXPORT_SYMBOL(csum_partial_copy_from_user);
#else /* CONFIG_ARCH_I386 */

asmlinkage __wsum
csum_partial_copy_generic(const void *src, void *dst, int len, __wsum sum,
		            int *src_err_ptr, int *dst_err_ptr)
{
	if (src_err_ptr)
	{
		int missing;

		missing = copy_from_user(dst, src, len);
		if (missing) {
			memset(dst + len - missing, 0, missing);
			*src_err_ptr = -EFAULT;
		}
		
	    return csum_partial((const unsigned char*)dst, len, sum);
	} else
	if (dst_err_ptr) {
		sum = csum_partial((const unsigned char*)src, len, sum);

		if (access_ok(VERIFY_WRITE, dst, len)) {
			if (copy_to_user(dst, src, len) == 0)
				return sum;
		}
		if (len)
			*dst_err_ptr = -EFAULT;

		return -1; /* invalid checksum */
	} else {
		sum = csum_partial((const unsigned char*)src, len, sum);
		memcpy(dst, src, len);
		return sum;
	}
}

#endif
