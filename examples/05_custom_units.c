#include <inttypes.h>
#include <stdio.h>
#include <ttak/timing/timing.h>

/* Calendar units built on top of TT_HOUR. Integers only. */
#define TT_DAY(n)        (TT_HOUR(n) * 24ULL)
#define TT_MONTH_FEB(n)  (TT_DAY(28) * (uint64_t)(n))   /* 28 days  */
#define TT_MONTH(n)      (TT_DAY(30) * (uint64_t)(n))   /* 30 days  */
#define TT_MONTH_LONG(n) (TT_DAY(31) * (uint64_t)(n))   /* 31 days  */
#define TT_YEAR(n)       (TT_DAY(365) * (uint64_t)(n))  /* 365 days */
#define TT_LEAP(span)    ((span) + TT_DAY(1))           /* +1 day   */

_Static_assert(TT_LEAP(TT_MONTH_FEB(1)) == TT_DAY(29), "leap February");
_Static_assert(TT_LEAP(TT_YEAR(1)) == TT_DAY(366), "leap year");

int main(void) {
    uint64_t leap_feb = TT_LEAP(TT_MONTH_FEB(1));
    uint64_t leap_yr  = TT_LEAP(TT_YEAR(1));

    printf("leap February = %" PRIu64 " days\n",
           (uint64_t)(leap_feb / TT_DAY(1)));
    printf("leap year     = %" PRIu64 " days\n",
           (uint64_t)(leap_yr / TT_DAY(1)));
    return 0;
}
