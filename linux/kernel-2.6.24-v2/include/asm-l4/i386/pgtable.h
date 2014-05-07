#ifndef _L4_I386_PGTABLE_H
#define _L4_I386_PGTABLE_H

//_PAGE_ATTRIBS	= 0x300	/* Page attributes, arch specific, 0 = Default */

/*
 * on x86 write-combining is same as no cache.  However, let L4 take 
 * care of this.
 */ 
#define _PAGE_NOCACHE		0x100
#define _PAGE_WRITETHROUGH	0x200
#define _PAGE_WRITECOMBINE	0x300

#define pgprot_writecombine(prot) __pgprot(pgprot_val(prot) | _PAGE_NOCACHE)

#endif /*_L4_I386_PGTABLE_H*/
