/*
 * Description: i386 sysenter setup
 */

#include <linux/init.h>
#include <linux/smp.h>
#include <linux/thread_info.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/string.h>
#include <linux/elf.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/module.h>

#include INC_SYSTEM2(pgtable.h)
#include INC_SYSTEM2(unistd.h)
#include INC_SYSTEM2(elf.h)
#include INC_SYSTEM2(fixmap.h)

#include <asm/tlbflush.h>
#include <asm/pgalloc.h>

#include <l4/arch/vregs.h>

unsigned int __read_mostly vdso_enabled = 1;

EXPORT_SYMBOL_GPL(vdso_enabled);

#ifdef CONFIG_IA32_VDSO_ENABLE

/* Defined in vsyscall-sysenter.S */
extern void SYSENTER_RETURN;

/***********************************************************
 * Relocate sysenter code
 ***********************************************************/

static __init void reloc_symtab(Elf32_Ehdr *ehdr,
				unsigned offset, unsigned size)
{
 	Elf32_Sym *sym = (void *)ehdr + offset;
	unsigned nsym = size / sizeof(*sym);
	unsigned i;

	for(i = 0; i < nsym; i++, sym++) {
		if (sym->st_shndx == SHN_UNDEF ||
	 	    sym->st_shndx == SHN_ABS)
			continue;  /* skip */

		if (sym->st_shndx > SHN_LORESERVE) {
			printk(KERN_INFO "VDSO: unexpected st_shndx %x\n",
			       sym->st_shndx);
			continue;
		}

		switch(ELF_ST_TYPE(sym->st_info)) {
		case STT_OBJECT:
		case STT_FUNC:
		case STT_SECTION:
		case STT_FILE:
			sym->st_value += VDSO_HIGH_BASE;
		}
	}
}

static __init void reloc_dyn(Elf32_Ehdr *ehdr, unsigned offset)
{
	Elf32_Dyn *dyn = (void *)ehdr + offset;

	for(; dyn->d_tag != DT_NULL; dyn++)
		switch(dyn->d_tag) {
		case DT_PLTGOT:
		case DT_HASH:
		case DT_STRTAB:
		case DT_SYMTAB:
		case DT_RELA:
		case DT_INIT:
		case DT_FINI:
		case DT_REL:
		case DT_DEBUG:
		case DT_JMPREL:
		case DT_VERSYM:
		case DT_VERDEF:
		case DT_VERNEED:
		case DT_ADDRRNGLO ... DT_ADDRRNGHI:
			/* definitely pointers needing relocation */
			dyn->d_un.d_ptr += VDSO_HIGH_BASE;
			break;

		case DT_ENCODING ... OLD_DT_LOOS-1:
		case DT_LOOS ... DT_HIOS-1:
			/* Tags above DT_ENCODING are pointers if
			   they're even */
			if (dyn->d_tag >= DT_ENCODING &&
			    (dyn->d_tag & 1) == 0)
				dyn->d_un.d_ptr += VDSO_HIGH_BASE;
			break;

		case DT_VERDEFNUM:
		case DT_VERNEEDNUM:
		case DT_FLAGS_1:
		case DT_RELACOUNT:
		case DT_RELCOUNT:
		case DT_VALRNGLO ... DT_VALRNGHI:
			/* definitely not pointers */
			break;

		case OLD_DT_LOOS ... DT_LOOS-1:
		case DT_HIOS ... DT_VALRNGLO-1:
		default:
			if (dyn->d_tag > DT_ENCODING)
				printk(KERN_INFO "VDSO: unexpected DT_tag %x\n",
				       dyn->d_tag);
			break;
		}
}

static __init void relocate_vdso(Elf32_Ehdr *ehdr)
{
	Elf32_Phdr *phdr;
	Elf32_Shdr *shdr;
	int i;

	BUG_ON(memcmp(ehdr->e_ident, ELFMAG, 4) != 0 ||
	       !elf_check_arch(ehdr) ||
	       ehdr->e_type != ET_DYN);

	ehdr->e_entry += VDSO_HIGH_BASE;

	/* rebase phdrs */
	phdr = (void *)ehdr + ehdr->e_phoff;
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr[i].p_vaddr += VDSO_HIGH_BASE;

		/* relocate dynamic stuff */
		if (phdr[i].p_type == PT_DYNAMIC)
			reloc_dyn(ehdr, phdr[i].p_offset);
	}

	/* rebase sections */
	shdr = (void *)ehdr + ehdr->e_shoff;
	for(i = 0; i < ehdr->e_shnum; i++) {
		if (!(shdr[i].sh_flags & SHF_ALLOC))
			continue;

		shdr[i].sh_addr += VDSO_HIGH_BASE;

		if (shdr[i].sh_type == SHT_SYMTAB ||
		    shdr[i].sh_type == SHT_DYNSYM)
			reloc_symtab(ehdr, shdr[i].sh_offset,
				     shdr[i].sh_size);
	}
}


/***********************************************************
 * Sysenter setup
 ***********************************************************/

/* Defined in vsyscall.S */
extern const char vsyscall_sysenter_start, vsyscall_sysenter_end;

static struct page *syscall_pages[1];

int __init
sysenter_setup(void)
{
	/* Obtain a page to contain the VDSO code. */
	void *syscall_page = (void *)get_zeroed_page(GFP_ATOMIC);
	syscall_pages[0] = virt_to_page(syscall_page);

	printk("vDSO mapped to %08lx.\n", __fix_to_virt(FIX_VDSO));

	/* 
	 * We assume that sysenter is supported by the CPU.
	 * Copy the code from the sysenter routines to the VDSO page.
	 */
	memcpy(syscall_page, 
	    &vsyscall_sysenter_start, 
	    &vsyscall_sysenter_end - &vsyscall_sysenter_start);

	/* Relocate the code. */
	relocate_vdso(syscall_page);

	return 0;
}

/* Setup a VMA at program startup for the vsyscall page */
int arch_setup_additional_pages(struct linux_binprm *bprm, int exstack)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr;
	int ret = 0;
        
	down_write(&mm->mmap_sem);

        addr = get_unmapped_area(NULL, 0, PAGE_SIZE, 0, 0);
        if (IS_ERR_VALUE(addr)) {
                ret = addr;
                goto up_fail;
        }

        /*
         * MAYWRITE to allow gdb to COW and set breakpoints
         *
         * Make sure the vDSO gets into every core dump.
         * Dumping its contents makes post-mortem fully
         * interpretable later without matching up the same
         * kernel and hardware config to see what PC values
         * meant.
         */
        ret = install_special_mapping(mm, addr, PAGE_SIZE,
                                      VM_READ|VM_EXEC|
                                      VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC|
                                      VM_ALWAYSDUMP,
                                      syscall_pages);

        if (ret) {
                goto up_fail;
        }

        current->mm->context.vdso = (void *)addr;

        /* printk("SYSENTER_RETURN is at %x\n", (void*)VDSO_SYM(&SYSENTER_RETURN)); */

 up_fail:
	up_write(&mm->mmap_sem);

	return ret;
}

#endif /* CONFIG_IA32_VDSO_ENABLE */
