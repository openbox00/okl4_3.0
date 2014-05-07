/*
 * ldt.h
 *
 * Definitions of structures used with the modify_ldt system call.
 *
 * This is duplicated from native Linux because we want to change the
 * define value of LDT_ENTRIES to something smaller, L4 doesn't support
 * as many currently.
 *
 * On OKLinux the LDT is laid out in such a way that the LDT index
 * starts from zero and continues until GDT_ENTRY_TLS_MIN (which is 
 * non-zero).  At this point the NPTL TLS entries take over.
 *
 * Native Linux supports 8192 entries with dynamic LDT sizing, OKLinux
 * is a static small number.
 *
 * -gl
 */
#ifndef _LINUX_LDT_H
#define _LINUX_LDT_H

#include INC_SYSTEM(segment.h)

/* Maximum number of LDT entries supported. */
#define LDT_ENTRIES	GDT_ENTRY_TLS_MIN
/* The size of each LDT entry. */
#define LDT_ENTRY_SIZE	8

#ifndef __ASSEMBLY__
struct user_desc {
	unsigned int  entry_number;
	unsigned long base_addr;
	unsigned int  limit;
	unsigned int  seg_32bit:1;
	unsigned int  contents:2;
	unsigned int  read_exec_only:1;
	unsigned int  limit_in_pages:1;
	unsigned int  seg_not_present:1;
	unsigned int  useable:1;
};

#define MODIFY_LDT_CONTENTS_DATA	0
#define MODIFY_LDT_CONTENTS_STACK	1
#define MODIFY_LDT_CONTENTS_CODE	2

#endif /* !__ASSEMBLY__ */
#endif
