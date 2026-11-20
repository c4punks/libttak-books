#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ttak/container/ringbuf.h>
#include <ttak/container/set.h>
#include <ttak/ht/map.h>
#include <ttak/timing/timing.h>

static int cmp_str(const void *a, const void *b) { return strcmp(a, b); }

int main(void) {
    uint64_t now = ttak_get_tick_count_ns();

    /* Ring buffer: fixed capacity, push fails instead of growing. */
    ttak_ringbuf_t *rb = ttak_ringbuf_create(3, sizeof(int));
    if (!rb) return 1;
    for (int i = 0; i < 4; i++)
        if (!ttak_ringbuf_push(rb, &i)) printf("ring full, dropped %d\n", i);
    for (int v; ttak_ringbuf_pop(rb, &v);) printf("ring pop %d\n", v);
    ttak_ringbuf_destroy(rb);

    /* Set: membership by key bytes + comparator. */
    ttak_set_t set;
    ttak_set_init(&set, 16, NULL, cmp_str, NULL);
    const char *words[] = { "alpha", "beta", "gamma" };
    for (size_t i = 0; i < 3; i++)
        ttak_set_add(&set, (void *)words[i], strlen(words[i]) + 1, now);
    ttak_set_remove(&set, "alpha", sizeof "alpha", now);
    bool has_beta  = ttak_set_contains(&set, "beta", sizeof "beta", now);
    bool has_alpha = ttak_set_contains(&set, "alpha", sizeof "alpha", now);
    printf("set: beta=%s alpha=%s\n",
           has_beta ? "yes" : "no", has_alpha ? "yes" : "no");
    ttak_set_destroy(&set, now);

    /* Map: uintptr_t key -> size_t value. */
    tt_map_t *map = ttak_create_map(16, now);
    if (!map) return 1;
    ttak_insert_to_map(map, 0xABCDEF, 99, now);
    size_t value = 0;
    if (ttak_map_get_key(map, 0xABCDEF, &value, now))
        printf("map: 0xABCDEF -> %zu\n", value);
    ttak_delete_from_map(map, 0xABCDEF, now);
    ttak_destroy_map(map);
    return 0;
}
