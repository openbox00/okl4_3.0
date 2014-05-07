KBUILD_CFLAGS += -pipe -O2
ELF_ARCH = arm
ELF_FORMAT = elf32-arm
obj-y = crt0.o div64.o findbit.o lib1funcs.o setbit.o clearbit.o ashldi3.o ashrdi3.o \
	syscalls.o semaphore.o delay.o signal.o changebit.o sys_arm.o io.o \
	testsetbit.o testclearbit.o user.o memset.o memcpy.o ptrace.o \
	exception.o run_with_fault_handler.o io-writesl.o io-writesb.o io-readsl-armv4.o \
	io-readsw-armv4.o io-writesw-armv4.o memmove.o \
	io.o dma.o mmu.o process.o dma-mapping.o

obj-$(CONFIG_MODULES)		+= module.o

ifeq ($(CONFIG_AEABI),y)
CFLAGS_ABI	:=-mabi=aapcs-linux -mno-thumb-interwork
else
CFLAGS_ABI	:=$(call cc-option,-mapcs-32,-mabi=apcs-gnu) $(call cc-option,-mno-thumb-interwork,)
endif

KBUILD_CFLAGS	+= -Iinclude3 $(CFLABS_ABI)

# From arch/arm/Makefile - should be updated for more recent than 2.6.9
# machine-$(CONFIG_ARCH_CO285)      := footbridge
# machine-$(CONFIG_ARCH_SHARK)      := shark
machine-$(CONFIG_SA1100)          := sa1100
machine-$(CONFIG_ARCH_PXA)        := pxa
machine-$(CONFIG_KZM)             := kzm
machine-$(CONFIG_VERSATILE)       := versatile
# machine-$(CONFIG_ARCH_L7200)      := l7200
# machine-$(CONFIG_ARCH_INTEGRATOR) := integrator
# machine-$(CONFIG_ARCH_CAMELOT)    := epxa10db
# machine-$(CONFIG_ARCH_CLPS711X)   := clps711x
# machine-$(CONFIG_ARCH_IOP3XX)     := iop3xx
machine-$(CONFIG_ARCH_IXP4XX)     := ixp4xx
# machine-$(CONFIG_ARCH_IXP2000)    := ixp2000
# machine-$(CONFIG_ARCH_OMAP)       := omap
# machine-$(CONFIG_ARCH_S3C2410)    := s3c2410
# machine-$(CONFIG_ARCH_LH7A40X)    := lh7a40x
# machine-$(CONFIG_ARCH_VERSATILE_PB) := versatile
# machine-$(CONFIG_ARCH_IMX)        := imx
# machine-$(CONFIG_ARCH_H720X)      := h720x

ifneq ($(machine-y),)
 obj-y += mach-$(machine-y)/
endif 

# Create include3/asm/ and symlink include3/asm/arch to the mach-$(machine) 
# directory if it exists

ifeq ($(incdir-y),)
incdir-y := $(machine-y)
endif
INCDIR   := arch-$(incdir-y)

include3/asm/.arch: $(wildcard include/config/arch/*.h) 
	@echo '  SYMLINK include3/asm/arch -> include/asm-l4/arm/$(INCDIR)'
ifneq ($(KBUILD_SRC),)
	$(Q)mkdir -p include3/asm
	$(Q)ln -fsn $(srctree)/include/asm-l4/arm/hardware include3/asm/hardware
	#$(Q)ln -fsn $(srctree)/include/asm-l4/arm/$(INCDIR) include3/asm/arch
	#$(Q)ln -fsn $(srctree)/include/asm-l4/arm/mach include3/asm/mach
else
	$(Q)mkdir -p include3/asm
	$(Q)ln -fsn $(srctree)/include/asm-l4/arm/hardware include3/asm/hardware
	#$(Q)ln -fsn hardware include/asm/hardware
	#$(Q)ln -fsn $(INCDIR) include/asm-l4/arch
	#$(Q)ln -fsn mach include3/asm/mach
endif
	@touch $@


include3: 
	$(Q)if [ ! -d include3 ]; then mkdir -p include3; fi;

archprepare: include3 include3/asm/.arch

# clean out include files here?
archclean:

#vmlinux.img: vmlinux
#$(CROSS_COMPILE)strip vmlinux
#	echo "SECTIONS { .mod_root : { *(.data) } }" > $@.lds
#	$(LD) -T $@.lds -r --oformat default -o $@ -bbinary $<
#	$(RM) $@.lds

#ELFDIR    := contrib/elf-loader
#ELFSCRIPT := $(L4ROOT)/user/$(ELFDIR)/platform/$(PLAT)/linker.lds
#ELFOBJS   := $(foreach DIR,common platform/$(PLAT),\
#	$(wildcard $(L4BUILDDIR)/$(ELFDIR)/$(DIR)/*.o))
#ELFMODS   := $(foreach MOD,kernel sigma0,$(L4BUILDDIR)/$(ELFDIR)/$(MOD).img)
#
#LIBGCC=$(CC) -print-libgcc-file-name
#LIBGCCDIR=`$(LIBGCC) | sed 's,/[[^/]]*,,'`

#vmlinux.bootimg: vmlinux.img $(ELFOBJS) $(ELFMODS)
#	$(LD) $(LDFLAGS) $(LDDIRS)\
#	-L$(L4BUILDDIR)/lib -T$(ELFSCRIPT) -o $@ \
#		$(ELFOBJS) $(ELFMODS) $< -lio $(LIBGCCDIR) 

#vmlinux.bootimg.bin: vmlinux.bootimg
#	$(OBJCOPY) -O binary $< $@

#install: vmlinux.bootimg.bin
#	@echo INSTALLING LINUX!
#	cp vmlinux.bootimg.bin /tftpboot/vmlinux.pleb

#skyeye: vmlinux.bootimg install
#	@arm-elf-run vmlinux.bootimg
