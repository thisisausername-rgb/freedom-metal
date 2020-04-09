#include <errno.h>
#include <metal/timer.h>
#include <sys/time.h>
#include <sys/times.h>

/* Timing information for current process. From
   newlib/libc/include/sys/times.h the tms struct fields are as follows:

   - clock_t tms_utime  : user clock ticks
   - clock_t tms_stime  : system clock ticks
   - clock_t tms_cutime : children's user clock ticks
   - clock_t tms_cstime : children's system clock ticks

   Since maven does not currently support processes we set both of the
   children's times to zero. Eventually we might want to separately
   account for user vs system time, but for now we just return the total
   number of cycles since starting the program.  */
clock_t _times(struct tms *buf) {
    unsigned long long mcc;
    unsigned long long timebase;

    metal_timer_get_timebase_frequency(0, &timebase);
    metal_timer_get_cyclecount(0, &mcc);

    /* Convert from native resolution to published resolution */
    mcc = mcc * CLOCKS_PER_SEC / timebase;

    buf->tms_stime = 0;
    buf->tms_cutime = 0;
    buf->tms_cstime = 0;
    return buf->tms_utime = mcc;
}
