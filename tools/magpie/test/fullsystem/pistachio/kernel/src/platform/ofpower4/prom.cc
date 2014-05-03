/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	platform/ofpower4/prom.cc
 * Description:	OpenFirmware Power4 Setup.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: prom.cc,v 1.2 2003/09/24 19:05:45 skoglund Exp $
 *
 ***************************************************************************/

#include INC_ARCH(of1275.h)
#include INC_ARCH(rtas.h)
#include INC_ARCH(1275tree.h)
#include INC_GLUE(hwspace.h)
#include INC_API(kernelinterface.h)

#include <debug.h>

word_t boot_cpuid SECTION(".init.data");
word_t boot_cpukhz SECTION(".init.data");
word_t boot_buskhz SECTION(".init.data");

/*
 * Map the position-independent device tree, and install.
 */
SECTION(".init") void of1275_tree_map( addr_t low, addr_t high )
{
    addr_t vaddr = phys_to_virt(low);

    prom_print_hex( "1275 tree found at", (word_t)vaddr );
    prom_puts( "\n\r" );

    get_of1275_tree()->init( (char *)vaddr );
}


/*
 * Finds and installs the position-independent copy of the
 * OpenFirmware device tree.
 */
SECTION(".init") void of1275_tree_init( kernel_interface_page_t *kip )
{
    // Look for the position-independent copy of the OpenFirmware device tree
    // in the kip's memory descriptors.
    for( word_t i = 0; i < kip->memory_info.get_num_descriptors(); i++ ) 
    {
	memdesc_t *mdesc = kip->memory_info.get_memdesc( i );
	if( (mdesc->type() == OF1275_KIP_TYPE) && 
		(mdesc->subtype() == OF1275_KIP_SUBTYPE) )
	{
	    of1275_tree_map( mdesc->low(), mdesc->high() );
	    return;
	}
    }

    // Not found.  Things won't work, but ...
    prom_puts( "*** Error: the boot loader didn't supply a copy of the\n\r"
	       "*** Open Firmware device tree!\n\r" );
    get_of1275_tree()->init( NULL );
}


/* Initialise the Platform
 * We are called in real mode - no relocation.
 * Map the kernel so that normal operation can continue
 */
void SECTION(".init") init_plat( word_t ofentry )
{
    /* Initialise the Open Firmware interface used to setup the RTAS */
    get_of1275()->init(ofentry);

    /* Initialise position independant the device tree */
    of1275_tree_init( get_kip() );

    /* Initialise the RTAS */
    get_rtas()->init_arch();

    word_t pvr;
    asm volatile (
	"mfpvr	    %0;"
	: "=r" (pvr)
    );
    switch( (pvr>>16) & 0xffff )
    {
    case 0x35: prom_puts( "Detected Power4 (Spinnaker) " );	break;
    case 0x38: prom_puts( "Detected Power4+	" );	break;
    case 0x40: prom_puts( "Detected Power3	" );	break;
    case 0x41: prom_puts( "Detected Power3+	" );	break;
    default:
	prom_print_hex( "Unknown Processor Version", (pvr >> 16) & 0xffff );
	prom_puts( ", " );
    }
    prom_print_hex( "Revision", (pvr & 0xffff) );
    prom_puts( "\n\r" );

    switch( (pvr>>16) & 0xffff )
    {
    case 0x35: ;
    case 0x38: break;
    default:
	prom_exit( "Unsupported CPU type\n\r" );
    }

    u32_t *prop_val, len, cpu, cpu_hz, bus_hz;
    of1275_device_t *chosen = get_of1275_tree()->find( "/chosen" );

    if ( !chosen->get_prop( "cpu", (char **)&prop_val, &len ))
    {
	prom_exit( "Unable get property \"cpu\" in /chosen\n\r" );
    }
    of1275_phandle_t cpu_pkg = *prop_val;
//    of1275_phandle_t cpu_pkg = get_of1275()->instance_to_package( *prop_val );
    get_of1275()->get_prop( cpu_pkg, "reg", &cpu, sizeof(cpu));

    get_of1275()->get_prop( cpu_pkg, "clock-frequency", &cpu_hz, sizeof(cpu_hz));
    get_of1275()->get_prop( cpu_pkg, "bus-frequency", &bus_hz, sizeof(bus_hz));
    
    boot_cpuid = cpu;
    boot_cpukhz = cpu_hz/1000;
    boot_buskhz = bus_hz/1000;

    prom_print_hex( "Boot cpu is", boot_cpuid );
    prom_puts( "\n\r" );
}
