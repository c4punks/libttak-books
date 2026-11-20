/* A BBS door game that admits at most 5 launches per second,
 * with a burst of 3. Everything else is turned away at the door. */
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <time.h>
#include <ttak/limit/limit.h>

/* <ttak/limit/limit.h> in short:
 *   ttak_token_bucket_t  tokens, max_tokens, refill_rate, spinlock
 *   ttak_ratelimit_init(rl, rate_per_sec, burst)
 *   ttak_ratelimit_allow(rl)   consumes 1.0 token, true if allowed  */

static void sleep_ms(long ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

int main(void) {
    ttak_ratelimit_t door;
    ttak_ratelimit_init(&door, 5.0, 3.0);

    int admitted = 0, refused = 0;
    for (int caller = 1; caller <= 12; caller++) {
        if (ttak_ratelimit_allow(&door)) {
            printf("caller %2d: launching door game\n", caller);
            admitted++;
        } else {
            printf("caller %2d: busy, try again later\n", caller);
            refused++;
        }
        sleep_ms(50);   /* callers arrive every 50 ms (20 per second) */
    }
    printf("admitted=%d refused=%d\n", admitted, refused);
    return 0;
}
