#include <l4.h>


#include <linux/kernel.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/bug.h>

#define PCMCIA_IO_0_BASE 0x20000000
#define MECR_BASE	 0xA0000000
#define MECR		 0x00000018

static unsigned long arm_io_encode(unsigned long port);
static void arm_barrier(void);

struct io_space io_space[1] = {
    { -1ul, arm_io_encode, arm_barrier },
};

void l4_init_io_space(unsigned int num)
{
}

static unsigned long arm_io_encode(unsigned long port)
{
	return port;
}

static void arm_barrier(void)
{
}

/*
 * Copy data from IO memory space to "real" memory space.
 * This needs to be optimized.
 */
void _memcpy_fromio(void *to, unsigned long from, size_t count)
{
	unsigned char *t = to;
	while (count) {
		count--;
		*t = readb(from);
		t++;
		from++;
	}
}

/*
 * Copy data from "real" memory space to IO memory space.
 * This needs to be optimized.
 */
void _memcpy_toio(unsigned long to, const void *from, size_t count)
{
	const unsigned char *f = from;
	while (count) {
		count--;
		writeb(*f, to);
		f++;
		to++;
	}
}

/*
 * "memset" on IO memory space.
 * This needs to be optimized.
 */
void _memset_io(unsigned long dst, int c, size_t count)
{
	while (count) {
		count--;
		writeb(c, dst);
		dst++;
	}
}
