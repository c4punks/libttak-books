#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <ttak/io/io.h>
#include <ttak/io/sync.h>
#include <ttak/mem/mem.h>
#include <ttak/mem/owner.h>
#include <ttak/timing/timing.h>

int main(void) {
    ttak_owner_t *owner = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    int p[2];
    if (!owner || pipe(p) != 0) return 1;

    /* A guard owns one descriptor and expires after its TTL.
     * TTL and timestamps share one clock: nanoseconds here. */
    ttak_io_guard_t rd, wr;
    ttak_io_guard_init(&rd, p[0], owner, TT_MILLI_SECOND(300),
                       ttak_get_tick_count_ns());
    ttak_io_guard_init(&wr, p[1], owner, TT_SECOND(2),
                       ttak_get_tick_count_ns());

    /* I/O buffers must come from the TTAK allocator: the guard
     * validates them through their header before every transfer. */
    char *buf = ttak_mem_alloc(64, TT_SECOND(5), ttak_get_tick_count_ns());
    char *out = ttak_mem_alloc(64, TT_SECOND(5), ttak_get_tick_count_ns());
    if (!buf || !out) return 1;
    size_t n = 0;
    size_t len = (size_t)snprintf(out, 64, "guarded chunk 01");  /* 16 B */
    ttak_io_sync_write(&wr, out, len, &n, ttak_get_tick_count_ns());
    ttak_io_sync_read(&rd, buf, len, &n, ttak_get_tick_count_ns());
    buf[n] = '\0';
    printf("read %zu bytes: \"%s\"\n", n, buf);

    /* Let the reader's TTL run out: the guard now refuses to read. */
    usleep(350 * 1000);
    ttak_io_sync_write(&wr, out, len, &n, ttak_get_tick_count_ns());
    ttak_io_status_t st = ttak_io_sync_read(&rd, buf, len, &n,
                                            ttak_get_tick_count_ns());
    printf("after 350 ms: %s\n", st == TTAK_IO_SUCCESS ? "read ok"
                                                        : "guard expired");

    /* refresh() would have extended it; close() releases the fd. */
    ttak_io_guard_close(&rd, ttak_get_tick_count_ns());
    ttak_io_guard_close(&wr, ttak_get_tick_count_ns());
    ttak_mem_free(buf);
    ttak_mem_free(out);
    ttak_owner_destroy(owner);
    return 0;
}
