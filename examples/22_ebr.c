#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

#include <ttak/mem/epoch.h>
#include <ttak/mem/epoch_gc.h>
#include <ttak/mem/mem.h>
#include <ttak/shared/shared.h>
#include <ttak/timing/timing.h>

typedef struct {
    int  version;
    char data[64];
} my_config_t;

/* Generates ttak_shared_config_t and its typed helpers. */
TTAK_SHARED_DEFINE_WRAPPER(config, my_config_t)

static ttak_shared_config_t *g_config;
static atomic_bool g_running = true;
static atomic_int  g_reads   = 0;

/* Reader: lock-free, protected by the epoch it is pinned to. */
static void *reader_func(void *arg) {
    ttak_owner_t *owner = arg;
    ttak_epoch_register_thread();

    while (atomic_load(&g_running)) {
        ttak_shared_result_t res;
        const my_config_t *cfg = g_config->base.access_ebr(
            &g_config->base, owner, true, &res);     /* enter epoch */
        if (cfg) {
            volatile int v = cfg->version;  /* safe: cannot be freed now */
            (void)v;
            atomic_fetch_add(&g_reads, 1);
            g_config->base.release_ebr(&g_config->base);  /* leave epoch */
        }
        usleep(10);
    }

    ttak_epoch_deregister_thread();
    return NULL;
}
