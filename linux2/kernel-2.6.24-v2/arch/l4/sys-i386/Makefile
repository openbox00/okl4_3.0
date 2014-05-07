CONFIG_IA32_VDSO_ENABLE = 1

KBUILD_CFLAGS += -O2
ELF_ARCH = i386
ELF_FORMAT = elf32-i386

obj-y = crt0.o delay.o signal.o syscalls.o semaphore.o io.o sys_i386.o bitops.o user.o ptrace.o process.o ldt.o traps.o processor.o string.o dma.o memcpy.o strstr.o ioport.o alternative.o

ifdef CONFIG_IA32_VDSO_ENABLE
obj-y += sysenter.o vsyscall.o
endif

obj-$(CONFIG_MODULES)	+= module.o
obj-$(CONFIG_PCI_MMCONFIG) += pci_mmconfig.o

obj-$(CONFIG_PCI) += pci.o pci_legacy.o 
vmlinux.img: vmlinux
	echo "SECTIONS { .mod_root : { *(.data) } }" > $@.lds
	$(LD) -T $@.lds -r --oformat default -o $@ -bbinary $<
	$(RM) $@.lds

# Other systems need to do some special initialisation
sysprepare:
	echo $(ARCH): no system preparation necessary. 

ELFDIR    := contrib/elf-loader
ELFSCRIPT := $(L4ROOT)/user/$(ELFDIR)/platform/$(PLAT)/linker.lds
ELFOBJS   := $(foreach DIR,common platform/$(PLAT),\
	$(wildcard $(L4BUILDDIR)/$(ELFDIR)/$(DIR)/*.o))
ELFMODS   := $(foreach MOD,kernel sigma0,$(L4BUILDDIR)/$(ELFDIR)/$(MOD).img)

LIBGCC=$(CC) -print-libgcc-file-name
LIBGCCDIR=`$(LIBGCC) | sed 's,/[[^/]]*,,'`

ifdef CONFIG_IA32_VDSO_ENABLE
# vsyscall.o contains the vsyscall DSO images as __initdata.
# We must build both images before we can assemble it.
# Note: kbuild does not track this dependency due to usage of .incbin
$(obj)/vsyscall.o: $(obj)/vsyscall-sysenter.so
targets += vsyscall-sysenter.o vsyscall-sysenter.so
targets += vsyscall-note.o vsyscall.lds

# The DSO images are built using a special linker script.
quiet_cmd_syscall = SYSCALL $@
	cmd_syscall = $(CC) -m elf_i386 -nostdlib $(SYSCFLAGS_$(@F)) \
			-Wl,-T,$(filter-out FORCE,$^) -o $@

export CPPFLAGS_vsyscall.lds += -P -C -U$(ARCH)

vsyscall-flags = -shared -s -Wl,-soname=linux-gate.so.1 \
		$(call ld-option, -Wl$(comma)--hash-style=sysv)
SYSCFLAGS_vsyscall-sysenter.so	= $(vsyscall-flags)

$(obj)/vsyscall-sysenter.so: \
$(obj)/vsyscall-%.so: $(src)/vsyscall.lds \
			$(obj)/vsyscall-%.o $(obj)/vsyscall-note.o FORCE
	$(call if_changed,syscall)

# We also create a special relocatable object that should mirror the symbol
# table and layout of the linked DSO.  With ld -R we can then refer to
# these symbols in the kernel code rather than hand-coded addresses.
extra-y += vsyscall-syms.o
$(obj)/built-in.o: $(obj)/vsyscall-syms.o
$(obj)/built-in.o: ld_flags += -R $(obj)/vsyscall-syms.o

SYSCFLAGS_vsyscall-syms.o = -r
$(obj)/vsyscall-syms.o: $(src)/vsyscall.lds \
		$(obj)/vsyscall-sysenter.o $(obj)/vsyscall-note.o FORCE
	$(call if_changed,syscall)
endif

vmlinux.bootimg: vmlinux.img $(ELFOBJS) $(ELFMODS)
	$(LD) $(LDFLAGS) $(LDDIRS)\
	-L$(L4BUILDDIR)/lib -T$(ELFSCRIPT)  -o $@ \
		$(ELFOBJS) $(ELFMODS) $< -lio $(LIBGCCDIR) 

boot:	vmlinux
	@scp vmlinux carl@serp:/tftp/roottask.i386
