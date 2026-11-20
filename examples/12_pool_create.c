/* Excerpt from src/thread/pool.c (error paths shortened) */
ttak_thread_pool_t *ttak_thread_pool_create(size_t num_threads,
                                            int default_nice,
                                            uint64_t now) {
    ttak_scheduler_init();                    /* smart scheduler ready */

    ttak_thread_pool_t *pool = ttak_mem_alloc_raw(
        sizeof *pool, __TTAK_UNSAFE_MEM_FOREVER__, now);
    if (!pool) return NULL;

    pool->num_threads    = num_threads;
    pool->creation_ts    = now;
    pool->is_shutdown    = false;
    pool->force_shutdown = pool_force_shutdown;   /* "permission to kill" */

    /* One coarse lock for shutdown, plus one queue/lock per shard.
     * Tasks are routed to shards through a Latin-square table, so
     * submitters rarely contend on the same lock. */
    pthread_mutex_init(&pool->pool_lock, NULL);
    pthread_cond_init(&pool->task_cond, NULL);
    for (size_t s = 0; s < TTAK_THREAD_POOL_SHARDS; s++) {
        ttak_priority_queue_init(&pool->shards[s].queue);
        pthread_mutex_init(&pool->shards[s].lock, NULL);
        pthread_cond_init(&pool->shards[s].cond, NULL);
    }

    pool->workers = ttak_mem_alloc_raw(
        sizeof(ttak_worker_t *) * num_threads,
        __TTAK_UNSAFE_MEM_FOREVER__, now);
    if (!pool->workers) { /* destroy locks, free pool */ return NULL; }

    for (size_t i = 0; i < num_threads; i++) {
        ttak_worker_t *w = ttak_mem_alloc_raw(
            sizeof *w, __TTAK_UNSAFE_MEM_FOREVER__, now);
        if (!w) {
            pool->num_threads = i;
            pool_force_shutdown(pool);
            return NULL;
        }

        w->pool            = pool;
        w->should_stop     = false;
        w->exit_code       = 0;
        w->preferred_shard = ttak_shard_for_worker(i);  /* spread evenly */

        w->wrapper = ttak_mem_alloc_raw(sizeof *w->wrapper,
                                        __TTAK_UNSAFE_MEM_FOREVER__, now);
        /* (allocation check omitted) */
        w->wrapper->nice_val  = default_nice;
        w->wrapper->ts        = now;
        w->wrapper->jmp_magic = 0;   /* abort checks are safe before setjmp */
        w->wrapper->jmp_tid   = 0;

        pool->workers[i] = w;
        if (pthread_create(&w->thread, NULL, ttak_worker_routine, w) != 0) {
            pool->num_threads = i;
            pool_force_shutdown(pool);
            return NULL;
        }
    }
    return pool;
}
