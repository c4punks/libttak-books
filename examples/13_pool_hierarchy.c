#include <inttypes.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>

#include <ttak/async/future.h>
#include <ttak/mem/mem.h>
#include <ttak/thread/pool.h>
#include <ttak/timing/timing.h>

/** Context handed from a parent task to the child it spawns. */
typedef struct {
    int depth;
    int id;
} task_context_t;

/* Contexts cross threads, so keep them out of the thread-local
 * pocket tier (<= 512 B and < 1 s): give them a 1 s lifetime. */
#define CTX_LIFETIME TT_SECOND(1)

static atomic_int active_tasks = 0;
static ttak_thread_pool_t *global_pool = NULL;

static void *hierarchy_task_routine(void *arg);

/** Fire-and-forget submit: count first, release the unused future. */
static void spawn(task_context_t *ctx, uint64_t now) {
    atomic_fetch_add(&active_tasks, 1);
    ttak_future_t *f = ttak_thread_pool_submit_task(
        global_pool, hierarchy_task_routine, ctx, 0, now);
    if (f) {
        ttak_future_release(f);       /* we never read the result */
    } else {
        atomic_fetch_sub(&active_tasks, 1);
        ttak_mem_free(ctx);
    }
}

/** Each task may spawn one child: vertical ownership down the tree. */
static void *hierarchy_task_routine(void *arg) {
    task_context_t *ctx = arg;
    uint64_t now = ttak_get_tick_count_ns();
    printf("[task] id=%-3d depth=%d  t=%" PRIu64 " ns\n",
           ctx->id, ctx->depth, now);

    if (ctx->depth < 2) {
        task_context_t *child = ttak_mem_alloc(sizeof *child,
                                               CTX_LIFETIME, now);
        if (child) {
            child->depth = ctx->depth + 1;
            child->id    = ctx->id * 10 + 1;
            spawn(child, now);
        }
    }

    ttak_mem_free(ctx);                 /* the task owns its context */
    atomic_fetch_sub(&active_tasks, 1);
    return NULL;
}

int main(void) {
    uint64_t now = ttak_get_tick_count_ns();

    /* 1. Three workers; the pool owns its threads and queues. */
    global_pool = ttak_thread_pool_create(3, 0, now);
    if (!global_pool) {
        fprintf(stderr, "Failed to create thread pool\n");
        return 1;
    }

    /* 2. Parallel ownership: two independent root tasks. */
    for (int i = 1; i <= 2; i++) {
        task_context_t *ctx = ttak_mem_alloc(sizeof *ctx,
                                             CTX_LIFETIME, now);
        if (!ctx) continue;
        ctx->depth = 0;
        ctx->id    = i;
        spawn(ctx, now);
    }

    /* 3. Wait for the whole tree (portable: no x86-only pause). */
    while (atomic_load(&active_tasks) > 0)
        sched_yield();

    /* 4. Teardown. Every context was freed by the task that owned it. */
    ttak_thread_pool_destroy(global_pool);
    return 0;
}
