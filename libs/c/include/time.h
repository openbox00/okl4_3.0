/*
 * Author: Ben Leslie
 * Created: Fri Sep 24 2004
 * Description: time.h as per ISOC99
 */

#ifndef _TIME_H_
#define _TIME_H_

#include <stddef.h>             /* For NULL, size_t */
#include <sys/types.h>          /* for time_t, etc */

#define CLOCKS_PER_SEC 1        /* Arbitrary, 1s resolution */

typedef long clock_t;
typedef long timer_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year; /* years since 1900 */
    int tm_wday; /* day of week [0,6] - Sunday = 0 */
    int tm_yday;
    int tm_isdst; /* Daylight savings flag */
};

/* FIXME: this should go to the right place. */
#define TZNAME_MAX 6

extern long timezone;
extern int daylight;
extern char tzname[2][TZNAME_MAX+1];

/* 7.23.2 Time manipulation functions */
clock_t clock(void);
double difftime(time_t time1, time_t time2);
time_t mktime(struct tm *timeptr);
time_t time(time_t *timer);

/* 7.23.3 Time conversion functions */
void tzset(void);
/* XXX: _tzset should be removed when we have environment support.
 * It is here for testing only */
void _tzset(char *);
char *asctime(const struct tm *timeptr);
char *asctime_r(const struct tm *timeptr, char *buf);
struct tm *gmtime(const time_t *timer);
struct tm *gmtime_r(const time_t *timer, struct tm *res);
struct tm *localtime(const time_t *timer);
struct tm *localtime_r(const time_t *timer, struct tm *res);

/* Add the time zone offset */
static inline void
_tz_add_offset(time_t *timer)
{
    tzset();
    *timer += timezone;
}

/* As per spec pg. 341 */
static inline char *
ctime(const time_t *timer)
{
    return asctime(localtime(timer));
}

/* Thread safe version */
static inline char *
ctime_r(const time_t *timer, char *buf)
{
    return asctime_r(localtime(timer), buf);
}

size_t strftime(char *s, size_t maxsize, const char *format,
                const struct tm *timeptr);

#ifdef __USE_POSIX
#include <posix/time.h>
#endif

#endif /* _TIME_H_ */
