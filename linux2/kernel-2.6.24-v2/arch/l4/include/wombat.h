#include <asm/setup.h>
#include <linux/mm.h>
#include <asm/mmzone.h>

#define STACK_SIZE 0x2000

extern bootmem_area_t bootmem_area[MAX_PHYSMEM_RANGES];
extern int bootmem_areas;

#if defined(CONFIG_IGUANA)
extern cap_t *first_cap;

#define KERNEL_THREADS 2
#define TIMER_THREAD 0
#define MAIN_THREAD 1
#endif
