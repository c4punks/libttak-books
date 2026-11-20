#include <stdio.h>
#include <string.h>
#include <ttak/timing/timing.h>
#include <ttak/tree/bplus.h>

static int cmp_str(const void *a, const void *b) { return strcmp(a, b); }

int main(void) {
    uint64_t now = ttak_get_tick_count_ns();

    /* Order 3: small nodes, so a handful of keys already splits them. */
    ttak_bplus_tree_t tree;
    ttak_bplus_init(&tree, 3, cmp_str, NULL, NULL);   /* no key/value free */

    const char *keys[] = { "delta", "alpha", "echo", "charlie", "bravo" };
    const char *vals[] = { "4", "1", "5", "3", "2" };
    for (size_t i = 0; i < 5; i++)
        ttak_bplus_insert(&tree, (void *)keys[i], (void *)vals[i], now);

    const char *probe[] = { "alpha", "charlie", "zulu" };
    for (size_t i = 0; i < 3; i++) {
        const char *v = ttak_bplus_get(&tree, probe[i], now);
        printf("%-8s -> %s\n", probe[i], v ? v : "(missing)");
    }

    ttak_bplus_destroy(&tree, now);
    return 0;
}
