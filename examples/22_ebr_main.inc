int main(void) {
    ttak_epoch_gc_t gc;              /* macro lifecycle (EpochGC)  */
    ttak_epoch_gc_init(&gc);

    /* The container itself is retired through EBR at the end,
     * so it must come from the TTAK allocator. */
    g_config = ttak_mem_alloc(sizeof *g_config, __TTAK_UNSAFE_MEM_FOREVER__,
                              ttak_get_tick_count_ns());
    if (!g_config) return 1;
    ttak_shared_config_init(g_config);
    ttak_shared_config_allocate(g_config, TTAK_SHARED_LEVEL_1);

    my_config_t *initial = (my_config_t *)g_config->base.shared;
    initial->version = 1;
    snprintf(initial->data, sizeof initial->data, "initial config");

    ttak_owner_t *r1 = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    ttak_owner_t *r2 = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    g_config->base.add_owner(&g_config->base, r1);
    g_config->base.add_owner(&g_config->base, r2);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, reader_func, r1);
    pthread_create(&t2, NULL, reader_func, r2);

    /* Writer: build a new version, swap it in, retire the old one. */
    for (int i = 0; i < 5; i++) {
        usleep(20000);
        my_config_t next = { .version = i + 2 };
        snprintf(next.data, sizeof next.data, "config update %d", i + 2);

        /* swap_ebr copies `next`; the old payload is retired, not freed */
        ttak_shared_swap_ebr(&g_config->base, &next, sizeof next);
        printf("writer: now serving version %d\n", next.version);

        ttak_epoch_reclaim();          /* free what no reader can see */
        ttak_epoch_gc_rotate(&gc);
    }

    atomic_store(&g_running, false);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("readers finished after %d lock-free reads\n",
           atomic_load(&g_reads));

    /* Retire payload and container; a few reclaim passes drain them. */
    g_config->base.retire(&g_config->base);
    for (int i = 0; i < 3; i++) {
        ttak_epoch_reclaim();
        ttak_epoch_gc_rotate(&gc);
    }

    ttak_owner_destroy(r1);
    ttak_owner_destroy(r2);
    ttak_epoch_gc_destroy(&gc);
    return 0;
}
