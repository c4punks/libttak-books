#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <ttak/async/function.h>
#include <ttak/async/sched.h>

static void *scheduled_task(void *arg) {
    puts((const char *)arg);
    return arg;
}

int main(void) {
    /* 1. Start the scheduler (nice value 0). */
    ttak_async_init(0);

    /* 2. Submit: function, context, cleanup callback, priority. */
    ttak_async_future_t future =
        ttak_async_call(scheduled_task, (void *)"scheduler fired", NULL, 0);
    if (!future.future) {
        fputs("async schedule failed\n", stderr);
        ttak_async_shutdown();
        return 1;
    }

    /* 3. Give workers a chance, then wait for the result. */
    ttak_async_yield();
    const char *result = ttak_async_await(&future);
    printf("await returned: %s\n", result ? result : "(null)");

    /* 4. Release the future, then stop the scheduler. */
    ttak_async_future_cleanup(&future);
    ttak_async_shutdown();
    return 0;
}
