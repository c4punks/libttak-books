#include <stdio.h>
#include <string.h>

#include <ttak/mem/mem.h>
#include <ttak/mem/epoch_gc.h>
#include <ttak/timing/timing.h>

/* Allocate and hand ownership to the collector in one step. */
static void *gc_alloc(ttak_epoch_gc_t *gc, size_t sz, uint64_t lifetime) {
    void *p = ttak_mem_alloc(sz, lifetime, ttak_get_tick_count_ns());
    if (p) ttak_epoch_gc_register(gc, p, sz);
    return p;
}

int main(void) {
    ttak_epoch_gc_t gc;
    ttak_epoch_gc_init(&gc);

    for (int frame = 0; frame < 3; frame++) {
        /* Scratch buffers for this frame only. */
        char *title = gc_alloc(&gc, 64, TT_MILLI_SECOND(10));
        int  *score = gc_alloc(&gc, sizeof *score, TT_MILLI_SECOND(10));
        if (!title || !score) break;

        snprintf(title, 64, "frame #%d", frame);
        *score = frame * 100;
        printf("%s -> score %d\n", title, *score);

        /* Manual collection point: everything registered in this frame
         * is released here, never in the middle of the frame. */
        ttak_epoch_gc_rotate(&gc);
    }

    ttak_epoch_gc_destroy(&gc);   /* sweeps whatever is left */
    return 0;
}
