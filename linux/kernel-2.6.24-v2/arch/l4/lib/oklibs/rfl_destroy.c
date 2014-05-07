/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Author: Ben Leslie 
 */

#include <asm/ll.h>
#include <asm/range_fl.h>

void
rfl_destroy(rfl_t rfl)
{
    ll_free(rfl);
}
