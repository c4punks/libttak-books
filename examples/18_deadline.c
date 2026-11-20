#define _XOPEN_SOURCE 700
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <ttak/timing/deadline.h>
#include <ttak/timing/timing.h>

static void do_work_step(void) {
    struct timespec ts = { 0, 10 * 1000000L };   /* 10 ms of "work" */
    nanosleep(&ts, NULL);
}

int main(void) {
    /* The deadline API works in milliseconds. */
    ttak_deadline_t dl;
    ttak_deadline_set(&dl, 50);
    printf("deadline remaining ~%" PRIu64 " ms\n",
           ttak_deadline_remaining(&dl));

    const int total_steps = 100;
    int steps = 0;
    while (steps < total_steps) {
        /* Poll before every step: stop as soon as 50 ms have passed. */
        if (ttak_deadline_is_expired(&dl)) {
            printf("deadline expired after %d/%d steps; stopping early\n",
                   steps, total_steps);
            break;
        }
        do_work_step();
        steps++;
    }
    if (steps == total_steps)
        printf("finished all %d steps before the deadline\n", total_steps);

    printf("deadline remaining after loop = %" PRIu64 " ms\n",
           ttak_deadline_remaining(&dl));
    return 0;
}
