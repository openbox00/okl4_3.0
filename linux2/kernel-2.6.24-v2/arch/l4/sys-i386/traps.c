/*
 *  linux/arch/i386/traps.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Pentium III FXSR, SSE support
 *      Gareth Hughes <gareth@valinux.com>, May 2000
 *
 *  Modified for OKLinux by Geoffrey Lee < glee at ok-labs dot com >
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <l4.h>

#include INC_SYSTEM2(processor.h)

/*
 * not much to see here ...
 */
struct desc_struct default_ldt[] = { { 0, 0 }, { 0, 0 }, { 0, 0 },
                { 0, 0 }, { 0, 0 } };

