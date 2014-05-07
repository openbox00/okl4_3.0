
/*
 * @LICENCE("Open Kernel Labs, Inc.", "2007")@
 */

/**
   @file ig_mmc.h
   COntains the structures and utilities used in the wombat-side 
   virtual device driver.
 */
#pragma once

#include <linux/workqueue.h>
#include <vmmc/vmmc.h>

/**
 */
struct ig_mmc_host
{
    struct mmc_host    *mmc;
    spinlock_t          lock;
    struct mmc_request *mrq;
    int                 size;
    int                 irq_gpio;
    int                 irq_mmc;
    objref_t            dev;
    L4_ThreadId_t       server;

    memsection_ref_t    shared_ms; /**< The shared memory segment for talking 
                                      with the iguana server */
    uintptr_t           shared_base; /**< shared memory segment as uintptr_t */
    struct work_struct  finish;
    int                 fin_wait;
    struct workqueue_struct *queue;
};
