#ifndef _L4_ARM_PGTABLE_H
#define _L4_ARM_PGTABLE_H

//_PAGE_ATTRIBS	= 0x300	/* Page attributes, arch specific, 0 = Default */

#define _PAGE_NOCACHE	    0x100
#define _PAGE_WRITECOMBINE  0x200
#define _PAGE_WRITETHROUGH  0x300

#define pgprot_writecombine(prot) __pgprot(pgprot_val(prot) | _PAGE_WRITECOMBINE)

#endif /*_L4_ARM_PGTABLE_H*/
