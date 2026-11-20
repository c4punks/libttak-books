#include <stdio.h>
#include <ttak/mem/owner.h>
#include <ttak/unsafe/context.h>

/* Runs with the shared buffer borrowed under one owner's authority. */
static void bump(void *shared, size_t size, void *who) {
    (void)size;
    int *counter = shared;
    (*counter)++;
    printf("%-6s owner ran: counter = %d\n", (const char *)who, *counter);
}

int main(void) {
    ttak_owner_t *first  = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    ttak_owner_t *second = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    if (!first || !second) return 1;

    /* A context bridges two owners over memory neither of them owns. */
    int counter = 0;
    ttak_context_t ctx;
    if (ttak_context_init(&ctx, first, second, &counter, sizeof counter,
                          __TTAK_CTX_USE_FIRST__)) {
        ttak_context_run(&ctx, __TTAK_CTX_USE_FIRST__,  bump, "first");
        ttak_context_run(&ctx, __TTAK_CTX_USE_SECOND__, bump, "second");
        ttak_context_destroy(&ctx);
    }

    ttak_owner_destroy(second);
    ttak_owner_destroy(first);
    return 0;
}
