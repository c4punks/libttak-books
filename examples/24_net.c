#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ttak/mem/owner.h>
#include <ttak/net/endpoint.h>
#include <ttak/net/session.h>
#include <ttak/net/view.h>
#include <ttak/timing/timing.h>

int main(void) {
    int rc = 1;
    int pair[2] = { -1, -1 };
    ttak_shared_net_endpoint_t *ep = NULL;
    ttak_net_session_mgr_t mgr;
    bool mgr_ready = false;

    ttak_owner_t *owner = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    if (!owner) return 1;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0) goto out;

    /* 1. Bind: the endpoint becomes the owner of pair[0]. */
    ep = ttak_net_endpoint_create(owner, ttak_get_tick_count_ns());
    if (!ep) goto out;
    const char tag[] = "book-socketpair";
    ttak_io_status_t st = ttak_net_endpoint_bind_fd(
        ep, owner, pair[0], AF_UNIX, SOCK_STREAM, 0,
        tag, (uint8_t)sizeof tag, TTAK_NET_ENDPOINT_UNIX,
        TT_SECOND(5),                      /* guard TTL in ns */
        ttak_get_tick_count_ns());
    if (st != TTAK_IO_SUCCESS) goto out;
    pair[0] = -1;                     /* closed by the endpoint from now on */

    /* 2. Session: the manager ticks, flushes and tears it down for us. */
    ttak_net_session_mgr_init(&mgr, true);
    mgr_ready = true;
    ttak_net_session_mgr_set_policy(&mgr, TTAK_SOCK_ALERT);
    ttak_net_session_t *s = ttak_net_session_mgr_create(
        &mgr, ep, NULL, owner, ttak_get_tick_count_ns());
    if (!s) goto out;

    /* 3. Map: read the payload through a view instead of copying it. */
    const char payload[] = "zero-copy hello";
    if (write(pair[1], payload, sizeof payload) < 0) goto out;

    ttak_net_view_t view;
    ttak_net_view_init(&view);
    st = ttak_net_view_from_endpoint(&view, ep, owner, sizeof payload, 0,
                                     ttak_get_tick_count_ns());
    if (st == TTAK_IO_SUCCESS)
        printf("received %zu bytes: \"%.*s\"\n",
               view.len, (int)view.len, (const char *)view.data);
    ttak_net_view_release(&view);

    ttak_net_session_mgr_tick(&mgr, ttak_get_tick_count_ns());
    ttak_net_session_mgr_close(&mgr, s, ttak_get_tick_count_ns());
    rc = 0;

out:  /* 4. Cleanup in reverse order of creation. */
    if (mgr_ready)
        ttak_net_session_mgr_destroy(&mgr, ttak_get_tick_count_ns());
    if (ep) ttak_net_endpoint_destroy(ep, owner, ttak_get_tick_count_ns());
    if (pair[0] >= 0) close(pair[0]);
    if (pair[1] >= 0) close(pair[1]);
    ttak_owner_destroy(owner);
    return rc;
}
