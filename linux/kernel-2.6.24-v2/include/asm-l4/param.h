#ifndef _L4_PARAM_H
#define _L4_PARAM_H

#include <asm/page.h>

/* FIXME: This is system specific */
#define EXEC_PAGESIZE   PAGE_SIZE

#ifndef NGROUPS
#define NGROUPS         32
#endif

#ifndef NOGROUP
#define NOGROUP         (-1)
#endif

#define MAXHOSTNAMELEN  64      /* max length of hostname */

#ifdef __KERNEL__
#define HZ 100
#define USER_HZ	100	   /* .. some user interfaces are in "ticks" */
#define CLOCKS_PER_SEC (USER_HZ)  /* frequency at which times() counts */
#endif

#endif
