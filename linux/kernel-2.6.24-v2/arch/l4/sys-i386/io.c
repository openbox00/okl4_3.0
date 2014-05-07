#include <l4.h>

#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/bug.h>

static unsigned long i386_io_encode(unsigned long port);
static void i386_barrier(void);

struct io_space io_space[1] = {
    { -1ul, i386_io_encode, i386_barrier },
};

void l4_init_io_space(unsigned int num)
{
	printk("%s called\n", __func__);
	//BUG();
	printk(KERN_ALERT "l4_init_io_space() is not written! evil may ensue...\n");
}

static unsigned long i386_io_encode(unsigned long port)
{
	printk("%s called\n", __func__);
	return port;
}

static void i386_barrier(void)
{
	printk("%s called\n", __func__);
}


