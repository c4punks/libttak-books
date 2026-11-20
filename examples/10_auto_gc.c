#include <stdio.h>

#include <ttak/mem/mem.h>
#include <ttak/mem/epoch_gc.h>
#include <ttak/timing/timing.h>

int main(void) {
    ttak_epoch_gc_t gc;
    ttak_epoch_gc_init(&gc);

    /* ttak_fastalloc = ttak_mem_alloc + ttak_epoch_gc_register.
     * 300 microseconds, written as an integer (not 0.3 ms). */
    int *x = ttak_fastalloc(&gc, sizeof(int), TT_MICRO_SECOND(300),
                            ttak_get_tick_count_ns());
    if (x == NULL) {
        ttak_epoch_gc_destroy(&gc);
        return 1;
    }

    *x = 42;
    printf("x = %d\n", *x);

    /* No ttak_mem_free(x): the collector owns it now.
     * Freeing it here as well would be a double free. */
    ttak_epoch_gc_rotate(&gc);
    ttak_epoch_gc_destroy(&gc);
    return 0;
}
