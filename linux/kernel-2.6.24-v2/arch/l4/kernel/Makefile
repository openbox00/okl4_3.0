extra-y := vmlinux.lds
core-y += arch/l4/kernel/fs/
obj-y := reboot.o glue.o main.o init_task.o time.o memcpy_user.o \
	syscalls.o proc.o process.o syscall_impl.o setup.o \
	signal.o sys_iguana.o cache.o exit.o syscall_loop.o \
	mmap.o irq.o

obj-$(CONFIG_L4_CLOCKSOURCE) += l4clksrc.o
obj-$(CONFIG_OKL4FS) 	+= intervm_loop.o
obj-$(CONFIG_MODULES)	+= syms.o
obj-$(CONFIG_ARCH_ARM)  += memcpy_user_armshared.o
