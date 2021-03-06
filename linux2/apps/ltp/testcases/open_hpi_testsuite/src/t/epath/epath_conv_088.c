/* -*- linux-c -*-
 * 
 * (C) Copyright IBM Corp. 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Sean Dague <http://dague.net/sean>
 *
 */

#include <string.h>
#include <SaHpi.h>
#include <epath_utils.h>

/**
 * main: epathstr -> epath test
 * 
 * This test tests whether an entity path string is converted into
 * an entity path properly.  
 *
 * TODO: a more extensive set of tests would be nice, might need to create a
 * perl program to generate that code
 * 
 * Return value: 0 on success, 1 on failure
 **/
int main(int argc, char **argv) 
{
        SaHpiEntityPathT tmp_ep;
        char *entity_root = "{PROCESSOR_MODULE,99}{POWER_SYSTEM_BOARD,14}";
        
        string2entitypath(entity_root, &tmp_ep);
        
        if(tmp_ep.Entry[0].EntityType != SAHPI_ENT_POWER_SYSTEM_BOARD) 
                return 1;
        
        if(tmp_ep.Entry[0].EntityInstance != 14)
                return 1;
        
        if(tmp_ep.Entry[1].EntityType != SAHPI_ENT_PROCESSOR_MODULE)
                return 1;
        
        if(tmp_ep.Entry[1].EntityInstance != 99)
                return 1;
        
        return 0;
}
