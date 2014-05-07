#ifndef __ASMi386_ELF_H
#define __ASMi386_ELF_H

/*
 * ELF register definitions..
 */

#include <asm/ptrace.h>
#include <asm/user.h>
#include <asm/processor.h>
//#include <asm/system.h>		/* for savesegment */
#include <asm/fixmap.h> /* For FIX_VDSO */

#define R_386_NONE	0
#define R_386_32	1
#define R_386_PC32	2
#define R_386_GOT32	3
#define R_386_PLT32	4
#define R_386_COPY	5
#define R_386_GLOB_DAT	6
#define R_386_JMP_SLOT	7
#define R_386_RELATIVE	8
#define R_386_GOTOFF	9
#define R_386_GOTPC	10
#define R_386_NUM	11


struct user_i387_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
};
/*
 * This is the old layout of "struct pt_regs", and
 * is still the layout used by user mode (the new
 * pt_regs doesn't have all registers as the kernel
 * doesn't use the extra segment registers)
 */
struct user_regs_struct {
	long ebx, ecx, edx, esi, edi, ebp, eax;
	unsigned short ds, __ds, es, __es;
	unsigned short fs, __fs, gs, __gs;
	long orig_eax, eip;
	unsigned short cs, __cs;
	long eflags, esp;
	unsigned short ss, __ss;
};


typedef unsigned long elf_greg_t;

#define ELF_NGREG (sizeof (struct user_regs_struct) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef struct user_i387_struct elf_fpregset_t;
typedef struct user_fxsr_struct elf_fpxregset_t;

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) \
	(((x)->e_machine == EM_386) || ((x)->e_machine == EM_486))

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS32
#define ELF_DATA	ELFDATA2LSB
#define ELF_ARCH	EM_386

/* SVR4/i386 ABI (pages 3-31, 3-32) says that when the program starts %edx
   contains a pointer to a function which might be registered using `atexit'.
   This provides a mean for the dynamic linker to call DT_FINI functions for
   shared libraries that have been loaded before the code runs.

   A value of 0 tells we have no such handler. 

   We might as well make sure everything else is cleared too (except for %esp),
   just to make things more deterministic.
 */
/*#define ELF_PLAT_INIT(_r, load_addr)	do { \
	_r->ebx = 0; _r->ecx = 0; _r->edx = 0; \
	_r->esi = 0; _r->edi = 0; _r->ebp = 0; \
	_r->eax = 0; \
} while (0)*/

	// this is for exec. think we need somthing like pt_regs in user_task_structure
//#warning FIXME
#if 0
#define ELF_PLAT_INIT(_r, load_addr)	do { \
	i386_put_ebx(_r, 0);		\
	i386_put_ecx(_r, 0);		\
	i386_put_edx(_r, 0);		\
	i386_put_esi(_r, 0);		\
	i386_put_edi(_r, 0);		\
	i386_put_ebp(_r, 0);		\
	i386_put_eax(_r, 0);		\
} while (0)
#endif

#define USE_ELF_CORE_DUMP	1
#define ELF_EXEC_PAGESIZE	4096

/* This is the location that an ET_DYN program is loaded if exec'ed.  Typical
   use of this is to invoke "./ld.so someprog" to test out a new version of
   the loader.  We need to make sure that it is out of the way of the program
   that it will "exec", and that there is sufficient room for the brk.  */

#define ELF_ET_DYN_BASE         (TASK_SIZE / 3 * 2)

/* regs is struct pt_regs, pr_reg is elf_gregset_t (which is
   now struct_user_regs, they are different) */

#define ELF_CORE_COPY_REGS(pr_reg, regs)		\
	i386_elf_core_copy_regs(&(pr_reg), (regs));

/* This yields a mask that user programs can use to figure out what
   instruction set this CPU supports.  This could be done in user space,
   but it's not easy, and we've already done it here.  */

#define ELF_HWCAP	(0/*boot_cpu_data.x86_capability[0] XXX*/)

/* This yields a string that ld.so will use to load implementation
   specific libraries for optimization.  This is more specific in
   intent than poking at uname or /proc/cpuinfo.

   For the moment, we have only optimizations for the Intel generations,
   but that could change... */

#define ELF_PLATFORM  (utsname()->machine)

/*
 * Architecture-neutral AT_ values in 0-17, leave some room
 * for more of them, start the x86-specific ones at 32.
 */
#define AT_SYSINFO		32
#define AT_SYSINFO_EHDR		33

#ifdef __KERNEL__
#define SET_PERSONALITY(ex, ibcs2) set_personality((ibcs2)?PER_SVR4:PER_LINUX)

//extern int dump_task_regs (struct task_struct *, elf_gregset_t *);
//extern int dump_task_fpu (struct task_struct *, elf_fpregset_t *);
extern int dump_task_extended_fpu (struct task_struct *, struct user_fxsr_struct *);

#define ELF_CORE_COPY_TASK_REGS(tsk, elf_regs)		\
	i386_elf_core_copy_regs_task((tsk), (elf_regs))

void i386_elf_core_copy_regs(elf_gregset_t *, struct pt_regs *);
int i386_elf_core_copy_regs_task(struct task_struct *, elf_gregset_t *);

/*
 * This is the range that is readable by user mode, and things
 * acting like user mode such as get_user_pages.
 */
#define FIXADDR_USER_START	(__fix_to_virt(FIX_VDSO))
#define FIXADDR_USER_END	(FIXADDR_USER_START + PAGE_SIZE)

struct linux_binprm;

extern unsigned int vdso_enabled;

#ifdef CONFIG_IA32_VDSO_ENABLE

#define VDSO_HIGH_BASE		(__fix_to_virt(FIX_VDSO))
#define VDSO_CURRENT_BASE	((unsigned long)current->mm->context.vdso)
#define VDSO_PRELINK		0 /* Used by vsyscall.lds.S */

#define VDSO_SYM(x) \
		(VDSO_CURRENT_BASE + (unsigned long)(x) - VDSO_PRELINK)

#define VDSO_HIGH_EHDR		((const struct elfhdr *) VDSO_HIGH_BASE)
#define VDSO_EHDR		((const struct elfhdr *) VDSO_CURRENT_BASE)


extern void __kernel_vsyscall;

#define VDSO_ENTRY		VDSO_SYM(&__kernel_vsyscall)

#define ARCH_HAS_SETUP_ADDITIONAL_PAGES
extern int arch_setup_additional_pages(struct linux_binprm *bprm,
                                       int executable_stack);

# define ARCH_DLINFO							\
do if (vdso_enabled) {							\
		NEW_AUX_ENT(AT_SYSINFO,	VDSO_ENTRY);			\
		NEW_AUX_ENT(AT_SYSINFO_EHDR, VDSO_CURRENT_BASE);        \
} while (0)

#endif /* ifdef CONFIG_IA32_VDSO_ENABLE */


/*
 * Not yet for these: there is no way to get FP state out of L4.  :-(
 * -gl
 */
//#define ELF_CORE_COPY_FPREGS(tsk, elf_fpregs) dump_task_fpu(tsk, elf_fpregs)
//#define ELF_CORE_COPY_XFPREGS(tsk, elf_xfpregs) dump_task_extended_fpu(tsk, elf_xfpregs)

#if 0

/*
 * These macros parameterize elf_core_dump in fs/binfmt_elf.c to write out
 * extra segments containing the vsyscall DSO contents.  Dumping its
 * contents makes post-mortem fully interpretable later without matching up
 * the same kernel and hardware config to see what PC values meant.
 * Dumping its extra ELF program headers includes all the other information
 * a debugger needs to easily find how the vsyscall DSO was being used.
 */
#define ELF_CORE_EXTRA_PHDRS		(VSYSCALL_EHDR->e_phnum)
#define ELF_CORE_WRITE_EXTRA_PHDRS					      \
do {									      \
	const struct elf_phdr *const vsyscall_phdrs =			      \
		(const struct elf_phdr *) (VSYSCALL_BASE		      \
					   + VSYSCALL_EHDR->e_phoff);	      \
	int i;								      \
	Elf32_Off ofs = 0;						      \
	for (i = 0; i < VSYSCALL_EHDR->e_phnum; ++i) {			      \
		struct elf_phdr phdr = vsyscall_phdrs[i];		      \
		if (phdr.p_type == PT_LOAD) {				      \
			BUG_ON(ofs != 0);				      \
			ofs = phdr.p_offset = offset;			      \
			phdr.p_memsz = PAGE_ALIGN(phdr.p_memsz);	      \
			phdr.p_filesz = phdr.p_memsz;			      \
			offset += phdr.p_filesz;			      \
		}							      \
		else							      \
			phdr.p_offset += ofs;				      \
		phdr.p_paddr = 0; /* match other core phdrs */		      \
		DUMP_WRITE(&phdr, sizeof(phdr));			      \
	}								      \
} while (0)
#define ELF_CORE_WRITE_EXTRA_DATA					      \
do {									      \
	const struct elf_phdr *const vsyscall_phdrs =			      \
		(const struct elf_phdr *) (VSYSCALL_BASE		      \
					   + VSYSCALL_EHDR->e_phoff);	      \
	int i;								      \
	for (i = 0; i < VSYSCALL_EHDR->e_phnum; ++i) {			      \
		if (vsyscall_phdrs[i].p_type == PT_LOAD)		      \
			DUMP_WRITE((void *) vsyscall_phdrs[i].p_vaddr,	      \
				   PAGE_ALIGN(vsyscall_phdrs[i].p_memsz));    \
	}								      \
} while (0)
#endif

#endif

#endif
