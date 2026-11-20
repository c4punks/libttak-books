#include <inttypes.h>
#include <stdio.h>
#include <ttak/timing/timing.h>

int main(void) {
    /* 01:02:03 written the way a human reads it */
    uint64_t ttl = TT_HOUR(1)
                 + TT_MINUTE(2)
                 + TT_SECOND(3);

    printf("ttl = %" PRIu64 " ns\n", ttl);   /* 3723000000000 */
    return 0;
}
