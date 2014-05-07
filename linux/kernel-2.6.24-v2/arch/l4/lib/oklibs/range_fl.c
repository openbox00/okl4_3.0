/* @LICENCE("National ICT Australia", "2004")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Description: range_fl implements a free list. The external interface is
 * basically alloc() and free(). A simple implementation would just be a linked 
 * list.
 * 
 * This implementation tries to improve on space needed to hold the free list.
 * We have a list of ranges. This means if we have a contiguous set of items
 * the space required is very small. As long as the list isn't too fragmented
 * this is a win. In the fragmented case we degenerate to a linked list.
 * 
 * The overhead with this is that on a free it must make sure the list remains
 * sorted, and also perform any range coalescing. E.g: If you have a list 1-3,
 * 5-6, and then free '4' you want to get 1-6.
 * 
 * Allocation is fairly straightforward, just take the front of the list and if 
 * you exhaust a range, then remove it from the linked list.
 * 
 * Authors: Ben Leslie <benjl@cse.unsw.edu.au>
 * 
 */

#include <asm/ll.h>
#include <stddef.h>
#include <asm/range_fl.h>

rfl_t
rfl_new(void)
{
    return ll_new();
}

int
rfl_insert_range(rfl_t rfl, uintptr_t from, uintptr_t to)
{
    struct ll *temp;
    struct range *range, *next_range;
    int added = 0;

#if defined(CONFIG_DEBUG)
    if (from > to) {
        /* Can't have a range like this */
        return E_RFL_INV_RANGE;
    }
#endif
    /* See we can append to existing */
    for (temp = rfl->next; temp != rfl; temp = temp->next) {
        range = temp->data;
#if defined(CONFIG_DEBUG)
        /* Check that the new range doesn't overlap with this existing range */
        if ((from >= range->from && from <= range->to) ||
            (to >= range->from && to <= range->to)) {
            return E_RFL_OVERLAP;
        }
#endif
        if (range->from == to + 1) {
            /* In this case can add to the start of this range */
            range->from = from;
            added = 1;
            break;
        }
        if (range->to == from - 1) {
            /* In this case we can add to the end of this range */
            next_range = temp->next->data;
            range->to = to;
            if (next_range != NULL && range->to + 1 == next_range->from) {
                /* Merge with next range */
                range->to = next_range->to;
                /* Now delete the next range */
                free(next_range);
                ll_delete(temp->next);
            }
            added = 1;
            break;
        }
        if (range->from > to) {
            /*
             * In this case we need to insert it before the existing range, so
             * we break now and let the logic at the end add it. 
             */
            break;
        }
    }

    if (added == 0) {
        /*
         * Couldn't extend an existing range, lets add a new range before the
         * current one. This keeps the list sorted. 
         */
        struct range *new_range = malloc(sizeof(struct range));

        if (new_range == NULL) {
            return E_RFL_NOMEM; /* Failure! */
        }
        ll_insert_before(temp, new_range);
        new_range->from = from;
        new_range->to = to;
    }

    return RFL_SUCCESS;
}

int
rfl_free(rfl_t rfl, uintptr_t val)
{
    return rfl_insert_range(rfl, val, val);
}

uintptr_t
rfl_alloc(rfl_t rfl)
{
    struct range *range;
    uintptr_t retval;

    if (rfl->next == rfl) {
        /* This is the no items left case */
        return -1UL;
    }
    range = rfl->next->data;
    retval = range->from;
    if (range->from == range->to) {
        /* None left in this range now, free resources */
        free(range);
        ll_delete(rfl->next);
    } else {
        /* There are more left in the range, just increment the from value */
        range->from++;
    }
    return retval;
}

#if 0
void
rfl_debug(rfl_t rfl, FILE *out)
{
    struct ll *temp;
    struct range *range;

    for (temp = rfl->next; temp != rfl; temp = temp->next) {
        range = temp->data;
        printf("from: %" PRIdPTR " to: %" PRIdPTR "\n", range->from, range->to);
    }
}
#endif
