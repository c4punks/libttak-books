#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ttak/mem/arena_helper.h>

/* Called once per chunk of the generation. Return false to stop. */
static bool fill_chunk(void *chunk, size_t bytes, size_t index, void *ctx) {
    unsigned gen = *(const unsigned *)ctx;
    memset(chunk, (int)((gen * 13u + index) & 0xFF), bytes);
    return true;
}

int main(void) {
    ttak_arena_env_config_t cfg;
    ttak_arena_env_config_init(&cfg);
    cfg.generation_bytes = 4096;   /* one generation = 4 KiB          */
    cfg.chunk_bytes      = 128;    /* carved into 128-byte chunks     */

    ttak_arena_env_t env;
    if (!ttak_arena_env_init(&env, &cfg)) return 1;

    for (unsigned gen = 0; gen < 3; gen++) {
        ttak_arena_generation_t g = {0};
        if (!ttak_arena_generation_begin(&env, &g, gen)) break;

        /* A one-off claim, then fill the rest chunk by chunk. */
        char *header = ttak_arena_generation_claim(&env, &g, 64);
        if (header) snprintf(header, 64, "generation %u", gen);

        size_t chunks = ttak_arena_generation_for_each(&env, &g, 0,
                                                       fill_chunk, &gen);
        printf("%s: %zu chunks, used %zu / %zu bytes\n",
               header ? header : "?", chunks, g.used, g.capacity);

        /* Retire the whole generation at once: no per-object free(). */
        ttak_arena_generation_retire(&env, &g);
        ttak_arena_env_rotate(&env);
    }

    ttak_arena_env_destroy(&env);
    return 0;
}
