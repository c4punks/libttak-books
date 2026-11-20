/**
 * @program finding-x
 * @file    main.c
 * @brief   Reads ten lines and reports whether each contains an 'x'.
 *          Short-lived log buffers are handed to EpochGC; the input
 *          buffer is owned and freed by the programmer.
 */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ttak/mem/mem.h>        /* allocator                  */
#include <ttak/mem/epoch_gc.h>   /* EpochGC                    */
#include <ttak/timing/timing.h>  /* TT_* units, monotonic clock */

#define INPUT_CAP 128
#define LOG_CAP   32

static void print_log(const char *s) {
    puts("Now the program has started.");
    puts(s);
}

/* Returns false on EOF so the loop can stop cleanly. */
static bool scanline(char *s, size_t cap) {
    if (!fgets(s, (int)cap, stdin)) return false;
    s[strcspn(s, "\n")] = '\0';
    return true;
}

int main(void) {
    ttak_epoch_gc_t gc;
    ttak_epoch_gc_init(&gc);

    /* Lifetimes are nanoseconds, so read the clock in nanoseconds too. */
    uint64_t now = ttak_get_tick_count_ns();

    /* GC-owned: printed once, then forgotten. Never free it by hand. */
    char *banner = ttak_mem_alloc(INPUT_CAP, TT_MILLI_SECOND(1), now);
    if (!banner) return 1;
    ttak_epoch_gc_register(&gc, banner, INPUT_CAP);
    snprintf(banner, INPUT_CAP, "#####Hello, LibTTAK!#####");
    print_log(banner);

    /* Programmer-owned: we cannot predict how long a human types. */
    char *input = ttak_mem_alloc(INPUT_CAP, __TTAK_UNSAFE_MEM_FOREVER__,
                                 ttak_get_tick_count_ns());
    if (!input) {
        ttak_epoch_gc_destroy(&gc);
        return 1;
    }

    for (int round = 0; round < 10; round++) {
        printf("Type a sentence: ");
        fflush(stdout);
        if (!scanline(input, INPUT_CAP)) break;
        printf("You typed %s\n", input);

        bool x_found = false;
        for (size_t i = 0; input[i] != '\0'; i++) {
            /* Showcase only: a real loop would reuse one buffer. */
            char *step = ttak_mem_alloc(LOG_CAP, TT_MILLI_SECOND(1),
                                        ttak_get_tick_count_ns());
            if (!step) break;
            ttak_epoch_gc_register(&gc, step, LOG_CAP);
            snprintf(step, LOG_CAP, "Iterate str[%zu]", i);
            puts(step);

            if (input[i] == 'x') {
                x_found = true;
                break;
            }
        }
        puts(x_found ? "x Found!!!" : input);

        /* The programmer decides when to collect: once per round. */
        ttak_epoch_gc_rotate(&gc);
    }

    ttak_mem_free(input);          /* ours: free it ourselves     */
    ttak_epoch_gc_destroy(&gc);    /* GC's: released all at once  */
    return 0;
}
