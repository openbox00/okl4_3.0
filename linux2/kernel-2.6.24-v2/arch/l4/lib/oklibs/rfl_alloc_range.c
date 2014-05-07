/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Author: Ben Leslie 
 */

#include <asm/range_fl.h>
#include <asm/ll.h>

uintptr_t
rfl_alloc_range(rfl_t rfl, unsigned long size)
{
        struct range *range;
        uintptr_t retval;
        rfl_t cur;

        /*
         * Find size entries to allocate 
         */
        if (rfl->next == rfl) {
                return 0;
        }

        cur = rfl->next;
        while (cur != rfl) {
                range = cur->data;
                retval = range->from;
                if (range->from + size - 1 == range->to) {
                        free(range);
                        ll_delete(cur);
                        return retval;
                } else if (range->from + size - 1 < range->to) {
                        range->from += size;
                        return retval;
                } else {
                        cur = cur->next;
                }
        }

        /*
         * Didn't find any valid sized things 
         */
        return 0;
}

uintptr_t
rfl_find_free_range(rfl_t rfl, unsigned long size)
{
    struct range *range;
    uintptr_t retval;
    rfl_t cur;

    /*
     * Find size entries to allocate 
     */
    if (rfl->next == rfl) {
        return 0;
    }

    cur = rfl->next;
    while (cur != rfl) {
        range = cur->data;
        retval = range->from;
        if (range->from + size - 1 == range->to) {
            return retval;
        } else if (range->from + size - 1 < range->to) {
            return retval;
        } else {
            cur = cur->next;
        }
    }

    /*
     * Didn't find any valid sized things 
     */
    return 0;
}


uintptr_t
rfl_alloc_specific_range(rfl_t rfl, uintptr_t from, unsigned long size)
{
    struct range *range;
    uintptr_t retval;
    rfl_t cur;

    /*
     * Find size entries to allocate 
     */
    if (rfl->next == rfl) {
        return 0;
    }

    cur = rfl->next;
    do {
        range = cur->data;
        retval = range->from;
        if(range->from <= from && range->to > from &&
            range->to >= (from + size - 1)) {
            if(range->from == from && 
                range->from + size - 1 == range->to) {
                free(range);
                ll_delete(cur);
                return retval;
            } else if (range->from == from) {
                range->from += size;
                return retval;
            } else if (from + size - 1 == range->to) {
                range->to -= size;
                return range->to;
            } else {
                struct range *new_range = malloc(sizeof(struct range));

                if(new_range == 0) {
                    return E_RFL_NOMEM; /* Failure! */
                }
                ll_insert_before(cur, new_range);
                new_range->from = range->from;
                new_range->to = from - 1;
                range->from = from + size;
                return from;
            }
        } else {
            cur = cur->next;
        }
    } while (cur != rfl);

    /*
     * Unable to fulfil request
     */
    return 0;
}
