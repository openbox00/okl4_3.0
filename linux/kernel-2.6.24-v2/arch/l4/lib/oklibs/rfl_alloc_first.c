/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Author: Ben Leslie 
 */

#include <asm/range_fl.h>

uintptr_t
rfl_alloc_first(rfl_t rfl, unsigned long *size)
{
    struct range *range;
    uintptr_t retval;

    if (rfl->next == rfl) {
        return -1UL;
    }
    range = rfl->next->data;
    retval = range->from;
    *size = range->to - range->from + 1;
    free(range);
    ll_delete(rfl->next);
    return retval;
}
