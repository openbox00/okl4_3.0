#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <okl4/env.h>

#if defined SERIAL_DRIVER
#include <serial/serial.h>
#endif /* SERIAL_DRIVER */

#include "../threadsafety.h"
#include "../stream.h"

extern struct __file __stdin;
extern struct __file __stdout;
extern struct __file __stderr;

static size_t
null_write(const void *data, long int position, size_t count, void *handle)
{
    return count;
}

static size_t
null_read(void *data, long int position, size_t count, void *handle)
{
    return 0;
}

static void
null_init(void)
{
    __stdin.read_fn = null_read;
    __stdout.write_fn = null_write;
    __stderr.write_fn = null_write;
}

#if defined SERIAL_DRIVER
static void
serial_stdio_init(void)
{
    __stdin.read_fn = serial_read;
    __stdout.write_fn = serial_write;
    __stderr.write_fn = serial_write;
}
#endif

static size_t
init_write(const void *data, long int position, size_t count, void *handle)
{
    /* If serial initialisation failed, fall back to /dev/null serial. */
#if defined SERIAL_DRIVER
    if (serial_init() == 0) {
        serial_stdio_init();
    } else {
#else
    if (1) {
#endif
        null_init();
    }

    /* Call the write function and return its result */
    return __stdout.write_fn(data, position, count, handle);
}

static size_t
init_read(void *data, long int position, size_t count, void *handle)
{
    /* If serial initialisation failed, fall back to /dev/null serial. */
#if defined SERIAL_DRIVER
    if (serial_init() == 0) {
        serial_stdio_init();
    } else {
#else
    if (1) {
#endif
        null_init();
    }

    /* Call the read function and return its result */
    return __stdin.read_fn(data, position, count, handle);
}

struct __file __stdin = {
    NULL,
    init_read,
    NULL,
    NULL,
    NULL,
    _IONBF,
    NULL,
    0,
    0
};

struct __file __stdout = {
    NULL,
    NULL,
    init_write,
    NULL,
    NULL,
    _IONBF,
    NULL,
    0,
    0
};

struct __file __stderr = {
    NULL,
    NULL,
    init_write,
    NULL,
    NULL,
    _IONBF,
    NULL,
    0,
    0
};

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;
