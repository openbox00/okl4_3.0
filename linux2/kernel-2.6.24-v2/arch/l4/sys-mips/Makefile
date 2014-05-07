CFLAGS += -save-temps -O2 -G 0 -mlong64 -mno-abicalls -non_shared -msplit-addresses -mabi=64 -march=r4000 -Wa,-mabi=o64
ELF_ARCH = mips
ELF_FORMAT = elf32-mips
obj-y = crt0.o syscalls.o syscalls64.o syscalls64_n32.o syscalls64_o32.o \
	semaphore.o signal.o io.o sys_mips.o unaligned.o user.o
obj-$(CONFIG_COMPAT) += linux32.o ioctl32.o
obj-$(CONFIG_MIPS32_O32) += binfmt_elfo32.o


vmlinux.img: vmlinux
	echo "SECTIONS { .mod_root : { *(.data) } }" > $@.lds
	$(LD) -T $@.lds -r --oformat default -o $@ -bbinary $<
	$(RM) $@.lds

# Other systems need to do some special initialisation
sysprepare:
	echo $(ARCH): no system preparation necessary

ELFDIR    := contrib/elf-loader
ELFSCRIPT := $(L4ROOT)/user/$(ELFDIR)/platform/$(PLAT)/linker.lds
ELFOBJS   := $(foreach DIR,common platform/$(PLAT),\
	$(wildcard $(L4BUILDDIR)/$(ELFDIR)/$(DIR)/*.o))
ELFMODS   := $(foreach MOD,kernel sigma0,$(L4BUILDDIR)/$(ELFDIR)/$(MOD).img)

vmlinux.bootimg: vmlinux.img $(ELFOBJS) $(ELFMODS)
	$(LD) $(LDFLAGS) $(LDDIRS) \
	-L$(L4BUILDDIR)/lib -T$(ELFSCRIPT) -o $@ \
	$(ELFOBJS) $(ELFMODS) $< -lio

install: vmlinux.bootimg
	@echo INSTALLING LINUX!
	cp vmlinux.bootimg /tftpboot

SULIMA=/home/carl/work/sulima/u4600
sulima: vmlinux.bootimg install
	$(SULIMA) vmlinux.bootimg
