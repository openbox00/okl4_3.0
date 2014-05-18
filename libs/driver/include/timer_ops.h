#ifndef _DRIVER_TIMER_OPS_H_
#define _DRIVER_TIMER_OPS_H_

#include <driver/driver_ops.h>

struct timer_ops {
    struct driver_ops d_ops;
    int (*timeout) (void *device, uint64_t time, callback_t callback,
                    callback_data_t callback_data, uintptr_t key);
    uint64_t (*current_time) (void *device);
};

static inline int
timer_timeout(void *device, uint64_t time,
              callback_t callback, callback_data_t callback_data, uintptr_t key)
{
    struct driver_instance *dp = (struct driver_instance *)device;

    return dp->classp->ops.t_ops->timeout(dp, time, callback,
                                          callback_data, key);
}

static inline uint64_t
timer_current_time(void *device)
{
    struct driver_instance *dp = (struct driver_instance *)device;

    return dp->classp->ops.t_ops->current_time(dp);
}

#endif /* _DRIVER_TIMER_OPS_H_ */
