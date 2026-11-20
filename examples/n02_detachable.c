#include <stdio.h>
#include <string.h>
#include <ttak/mem/detachable.h>

int main(void) {
    /* Epoch reclamation + default EpochGC + locked access. */
    ttak_detachable_context_t ctx;
    ttak_detachable_context_init(&ctx,
        TTAK_ARENA_HAS_EPOCH_RECLAMATION |
        TTAK_ARENA_HAS_DEFAULT_EPOCH_GC  |
        TTAK_ARENA_USE_LOCKED_ACCESS);

    for (int i = 0; i < 8; i++) {
        size_t size = (i % 2) ? 16 : 64;  /* 16-byte blocks may be cached */
        ttak_detachable_allocation_t a =
            ttak_detachable_mem_alloc(&ctx, size, (uint64_t)i);
        if (!a.data) break;

        bool cached = a.detach_status.bits & TTAK_DETACHABLE_PARTIAL_CACHE;
        printf("alloc %d: %2zu bytes from %s\n",
               i, a.size, cached ? "cache" : "arena");

        memset(a.data, i, a.size);
        ttak_detachable_mem_free(&ctx, &a);   /* back to cache or arena */
    }

    printf("cache hits=%llu misses=%llu\n",
           (unsigned long long)ctx.small_cache.hits,
           (unsigned long long)ctx.small_cache.misses);

    ttak_detachable_context_destroy(&ctx);
    return 0;
}
