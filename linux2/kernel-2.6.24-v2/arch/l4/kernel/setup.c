#include "l4.h"
#include "assert.h"



#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/root_dev.h>
#include <linux/initrd.h>
#include <linux/init.h>
#include <linux/utsname.h>
#include <linux/console.h>
#include <asm/page.h>
#include <asm/tlbflush.h>
#include <asm/setup.h>

#if defined(CONFIG_IGUANA)
#include <iguana/memsection.h>
#include <iguana/cap.h>
#include <iguana/thread.h>
#endif
#if defined(CONFIG_CELL)
#include <okl4/env.h>
#include <okl4/types.h>
#endif

#ifdef CONFIG_VT
#include <linux/tty.h>
#include <linux/console.h>
#include <linux/ioport.h>
#include <linux/screen_info.h>
#include <asm/io.h>

unsigned long VGA_offset;

struct screen_info screen_info = {
#if defined(CONFIG_FRAMEBUFFER_CONSOLE)
	.orig_video_lines	= 30,
#else
	.orig_video_lines	= 25,
#endif
	.orig_video_cols	= 80,
	.orig_x			= 0,
	.orig_y			= 25,
	.orig_video_mode	= 0,
	/*
 	 * The below is not exactly correct.  Setting to VLFB will
 	 * make the framebuffer work for cirrus logic.  Setting to
 	 * 1 will provide blank screen.  But you need 1 for vga
 	 * to work.
 	 */
#if defined(CONFIG_FRAMEBUFFER_CONSOLE)
	.orig_video_isVGA	= VIDEO_TYPE_VLFB,
#else
	.orig_video_isVGA	= 1,
#endif
	.orig_video_points	= 16
}; 
#define IORESOURCE_RAM (IORESOURCE_BUSY | IORESOURCE_MEM)
static struct resource video_ram_resource = { 
	.name = "Video RAM area", 
	.start = 0xa0000, 
	.end = 0xbffff, 
	.flags = IORESOURCE_RAM
};

void __init __setup_vga(void)
{
#ifdef CONFIG_IGUANA
	char* vga_obj;
        envitem_t *ig_ref;

	if ((ig_ref = (envitem_t *)iguana_getenv("vga")) == NULL)
		L4_KDB_Enter("didn't find the iguana ref to vga");

	if ((vga_obj = env_memsection_base(ig_ref)) == NULL)
		L4_KDB_Enter("vga_obj returned is 0");

	VGA_offset = (unsigned long)vga_obj - video_ram_resource.start;
	request_resource(&iomem_resource, &video_ram_resource);
#endif
}


#endif /* CONFIG_VT */

#define PFN_UP(x)	(((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)	((x) << PAGE_SHIFT)

#define STR(x)   #x
#define XSTR(x)  STR(x)

extern char boot_command_line[];
L4_Fpage_t utcb_area;

extern unsigned long start_phys_mem, end_phys_mem;
extern void * __rd_start, * __rd_end;

extern void enable_early_printk(void);

void __init
setup_machine_name(void)
{
	snprintf(init_utsname()->machine, __NEW_UTS_LEN, XSTR(__SYSTEM__));
}

extern void setup_tls(int thread_num);

struct drive_info_struct { char dummy[32]; } drive_info;

#if defined(CONFIG_CELL)
void okl4_env_lookup_address(const char *name, unsigned long *addr)
{
	word_t *seg;
	okl4_env_segments_t *segs;
	int i;

	seg = (word_t *)okl4_env_get(name);
	segs = OKL4_ENV_GET_SEGMENTS("SEGMENTS");
	if (!seg || !segs)
		goto nosegs;
	for (i = 0; i < segs->num_segments; i++) {
		if (segs->segments[i].segment == *seg) {
			*addr = segs->segments[i].virt_addr;
			break;
		}
	}

	if (i == segs->num_segments) {
nosegs:
		panic("Could not find %s", name);
		*addr = 0;
	}
}
#endif

/* 
 * Setup L4 architecture requirements:
 *   kip_area, utcb_area, physical memory, current,
 * Init user address space, ramdisk, console, early consle
 */
void __init
setup_arch (char **command_line) 
{ 
	unsigned long base, area;

#if defined(CONFIG_IGUANA)
	setup_tls(1);
#endif

	/* Return the command line to the rest of the kernel */
	boot_command_line[COMMAND_LINE_SIZE-1] = '\0';
	*command_line = boot_command_line;

        if (L4_UtcbIsKernelManaged()) {
                utcb_area = L4_Nilpage;
        } else {
                /* Currently hardcoded to 1024 L4 threads per linux
                 * user address space */
                area = L4_GetUtcbSize() * 1024;
                
	        /*
                 * Find some area to put the utcb in outside user's
                 * area.  When the KIP was present, 16 pages were
                 * reserved for it, so keep the same spacing here
                 * because the equation is not fully understood.
                 */
		base = PAGE_ALIGN(TASK_SIZE) + 16 * PAGE_SIZE  + area;
                /* Round address to the 'area' boundary. */
		base = (base + (area-1)) & (~(area-1));

		utcb_area = L4_Fpage(base, L4_GetUtcbSize() * 1024);
        }

 	/* Initialise our machine name */
	setup_machine_name();

	/* FIXME: (why?) */
	start_phys_mem = __pa(start_phys_mem);
	end_phys_mem = __pa(end_phys_mem);

	/* Initialise paging */
	paging_init();

	/* Thread info setup. */
	/* FIXME:  remember for SMP startup */
	current_tinfo(smp_processor_id()) = 
	    (unsigned long)&init_thread_union.thread_info;
	task_thread_info(current)->user_tid = L4_nilthread;
	task_thread_info(current)->user_handle = L4_nilthread;

#ifdef CONFIG_EARLY_PRINTK
	/* early console initialisation */
	enable_early_printk();
#endif

	/* Ramdisk setup */
#ifdef CONFIG_BLK_DEV_INITRD
	/* Board specific code should have set up initrd_start and initrd_end */
	ROOT_DEV = Root_RAM0;
        /* FIXME! */
	initrd_start = 0; //naming_lookup("ramdisk");
	initrd_end = 0; //naming_lookup("ramdisk_end");
	printk("end: %lx\n", initrd_end);
	initrd_below_start_ok = 1;

	if (initrd_start) {
		unsigned long initrd_size =
			((unsigned char *)initrd_end) -
			((unsigned char *)initrd_start);
		printk("Initial ramdisk at: 0x%p (%lu bytes)\n",
			(void *)initrd_start, initrd_size);
	}
#endif /* CONFIG_BLK_DEV_INITRD  */

#ifdef CONFIG_VT
#if defined(CONFIG_VGA_CONSOLE)
	__setup_vga();
	conswitchp = &vga_con;
#elif defined(CONFIG_DUMMY_CONSOLE)
	conswitchp = &dummy_con;
#endif /* CONFIG_VGA_CONSOLE */
	screen_info.lfb_base	= 0xe000000;
	screen_info.lfb_size	= 600*800;
	screen_info.lfb_height	= 600;
	screen_info.lfb_width	= 800;
#endif /* CONFIG_VT */

	panic_timeout = 1;

#if defined(CONFIG_CELL)
	/* L4-specific -gl,cvs */
	{
		extern unsigned long TIMER_BASE, SERIAL_BASE;
		okl4_env_lookup_address("MAIN_TIMER_MEM0", &TIMER_BASE);
		okl4_env_lookup_address("MAIN_SERIAL_MEM0", &SERIAL_BASE);
	}
#if defined(CONFIG_VERSATILE)
	{
		extern unsigned long ETH_BASE, CLCD_BASE, VERSATILE_SYS_BASE,
					       KMI0_BASE, KMI1_BASE;
		okl4_env_lookup_address("MAIN_ETH_MEM0", &ETH_BASE);
		okl4_env_lookup_address("MAIN_VERSATILESYS_MEM0", &VERSATILE_SYS_BASE);
		okl4_env_lookup_address("MAIN_CLCD_MEM0", &CLCD_BASE);
		okl4_env_lookup_address("MAIN_KMI0_MEM0", &KMI0_BASE);
		okl4_env_lookup_address("MAIN_KMI1_MEM0", &KMI1_BASE);
	}
#endif
#if defined(CONFIG_ARCH_GUMSTIX)
	{
		extern unsigned long GPIO_BASE, DMAC_BASE;
		extern unsigned long PXA_CS1_PHYS, PXA_CS1_DMA;
		extern unsigned long PXA_CS2_PHYS, PXA_CS2_DMA;
		okl4_env_lookup_address("MAIN_GPIO_MEM0", &GPIO_BASE);
		okl4_env_lookup_address("MAIN_DMA_MEM0", &DMAC_BASE);
		okl4_env_lookup_address("MAIN_CS_MEM1", &PXA_CS1_PHYS);
		okl4_env_lookup_address("MAIN_CS_MEM2", &PXA_CS2_PHYS);

		PXA_CS1_DMA = *((unsigned long *)okl4_env_get("cs_mem1_physical"));
		PXA_CS2_DMA = *((unsigned long *)okl4_env_get("cs_mem2_physical"));
	}
#endif

#endif	/*CELL*/
}

#if defined(CONFIG_CELL)
unsigned long SERIAL_BASE;
unsigned long TIMER_BASE;
unsigned long ETH_BASE;
unsigned long CLCD_BASE;
unsigned long KMI0_BASE;
unsigned long KMI1_BASE;
unsigned long GPIO_BASE;
unsigned long DMAC_BASE;
unsigned long VERSATILE_SYS_BASE;
unsigned long PXA_CS1_PHYS;
unsigned long PXA_CS2_PHYS;
unsigned long PXA_CS1_DMA;
unsigned long PXA_CS2_DMA;
#endif
