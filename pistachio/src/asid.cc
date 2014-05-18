/*
 * Description: Generic L4 ASID management
 */
#include <l4.h>
#include <space.h>
#include <tcb.h>

/* Only compile if ASIDS required */
#if defined(CONFIG_MAX_NUM_ASIDS)

#if !defined(CONFIG_ASIDS_DIRECT)
asid_t * asid_cache_t::lookup(hw_asid_t hw_asid)
{
    space_t *space;
    if (asids[hw_asid].asid == (space_ref)~0) {
        space = get_kernel_space();
    } else {
        space = get_space_list()->lookup_space(spaceid_t::spaceid(asids[hw_asid].asid));
        if (space == NULL) {
            return (asid_t*)-1;
        }
    }
    return space->get_asid();
}
#endif

hw_asid_t asid_cache_t::allocate(space_t *space, asid_t *asid)
{
    ASSERT(DEBUG, !asid->is_valid());
    hw_asid_t idx =(int)asid_t::invalid;

    asid_lock.lock();
#ifdef CONFIG_ASIDS_LRU
    if (first_free == -1) {
        idx = asids[lru_head].prev;
        move_asid(idx, true, false);

        asid_t * old = get_asid_cache()->lookup(idx);
        old->preempt();
    } else {
        idx = first_free;
        first_free = asids[first_free].next;
    }

    move_asid(idx, false, true);
#endif /* CONFIG_ASIDS_LRU */

#ifdef CONFIG_ASIDS_ROUNDR
    static hw_asid_t next = 0;
    space_t *cur_spc = get_current_space();

    if (!cur_spc) {
        cur_spc = get_kernel_space();
    }
    hw_asid_t current = cur_spc->get_asid()->value();

again:
    idx = next;

#if defined(CONFIG_ASIDS_DIRECT)
    while (asids[idx].asid == (asid_t*)-1)
#else
    while (asids[idx].asid == INVALID_REF)
#endif
    {
        idx ++;
        if (idx >= CONFIG_MAX_NUM_ASIDS) {
            idx = 0;
        }
    }
    next = idx + 1;
    if (next >= CONFIG_MAX_NUM_ASIDS) {
        next = 0;
    }
    if (idx == current) {
        goto again;
    }

    if (asids[idx].asid != FREE_REF) {
        asid_t * old = get_asid_cache()->lookup(idx);
        old->preempt();
    }

#endif /* CONFIG_ASIDS_ROUNDR */

#ifdef CONFIG_ASIDS_RANDR
#error fixme
#endif

#ifdef CONFIG_ASIDS_STATIC
    static hw_asid_t next = 0;
    word_t count = 0;

    idx = next;

#if defined(CONFIG_ASIDS_DIRECT)
    while (asids[idx].asid == (asid_t*)-1)
#else
    while ((asids[idx].asid == INVALID_REF) || (asids[idx].asid != FREE_REF))
#endif
    {
        idx ++;
        count ++;
        if (idx >= CONFIG_MAX_NUM_ASIDS) {
            idx = 0;
        }
        if (count >= CONFIG_MAX_NUM_ASIDS) {
            TRACEF("run out of free asids\n");
            asid_lock.unlock();
            return asid_t::invalid;
        }
    }
    next = idx + 1;
    if (next >= CONFIG_MAX_NUM_ASIDS) {
        next = 0;
    }

#endif

#if defined(CONFIG_ASIDS_DIRECT)
    asids[idx].asid = asid;
#else
    asids[idx].asid = (space_ref)space->get_space_id().get_spaceno();
#endif

    asid_lock.unlock();
    return idx;
}

void asid_cache_t::set_valid(hw_asid_t start, hw_asid_t end)
{
    hw_asid_t idx;
    asid_lock.lock();

#if defined(CONFIG_ASIDS_LRU)
    hw_asid_t *prev, old_head;

    old_head = first_free;
    prev = &first_free;
#endif

    for (idx = start; idx <= end; idx++)
    {
#if defined(CONFIG_ASIDS_DIRECT)
        if (asids[idx].asid != (asid_t *) -1)
            continue;
#else
        if (asids[idx].asid != INVALID_REF)
            continue;
#endif

        asids[idx].asid = FREE_REF;
#if defined(CONFIG_ASIDS_LRU)
        *prev = idx;
        prev = &asids[idx].next;
#endif
    }

#if defined(CONFIG_ASIDS_LRU)
    *prev = old_head;
#endif
    asid_lock.unlock();
}

#endif /* !CONFIG_MAX_NUM_ASIDS */
