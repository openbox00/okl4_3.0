/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 99, 2003 by Ralf Baechle
 */
#ifndef _ASM_BYTEORDER_H
#define _ASM_BYTEORDER_H

#include <asm/types.h>

#ifdef __GNUC__
/* FIXME: We're not sure what this does or if its needed */
#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#endif /* __GNUC__ */

#if defined (__L4EB__)
#  include <linux/byteorder/big_endian.h>
#elif defined (__L4EL__)
#  include <linux/byteorder/little_endian.h>
#else
#  error "L4, but neither __L4EB__, nor __L4EL__???"
#endif

#endif /* _ASM_BYTEORDER_H */
