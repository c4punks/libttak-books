#include <stdio.h>
#include <string.h>
#include <ttak/mem/mem.h>
#include <ttak/mem/owner.h>
#include <ttak/timing/timing.h>

static void show(void *ctx, void *args) {
    (void)args;
    printf("  owner sees: %s\n", ctx ? (const char *)ctx : "(nothing)");
}

int main(void) {
    uint64_t now = ttak_get_tick_count_ns();

    /* ttak_mem_freep: free and NULL the variable in one call. */
    char *buf = ttak_mem_alloc(64, TT_SECOND(1), now);
    if (!buf) return 1;
    strcpy(buf, "hello");
    ttak_mem_freep((void **)&buf);
    printf("after freep: buf = %p\n", (void *)buf);   /* (nil) */
    ttak_mem_freep((void **)&buf);                    /* harmless no-op */

    /* ttak_mem_unuse: detach a block from an owner without freeing it. */
    ttak_owner_t *owner = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    char *name = ttak_mem_alloc(32, TT_SECOND(1), now);
    if (!owner || !name) return 1;
    strcpy(name, "Younghee");

    ttak_owner_register_resource(owner, "name", name);
    ttak_owner_register_func(owner, "show", show);
    puts("before unuse:");
    ttak_owner_execute(owner, "show", "name", NULL);

    ttak_mem_unuse(name, owner);
    puts("after unuse:");
    ttak_owner_execute(owner, "show", "name", NULL);
    printf("  but the pointer is still ours: %s\n", name);

    ttak_owner_destroy(owner);
    ttak_mem_freep((void **)&name);     /* we still own it, so we free it */
    return 0;
}
