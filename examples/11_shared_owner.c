/**
 * @file  main.c
 * @brief Set up shared data, release the lock, then let the owner run
 *        registered functions on it (no double locking).
 */
#include <stdio.h>
#include <string.h>

#include <ttak/mem/owner.h>
#include <ttak/shared/shared.h>

struct child_data_s {
    int  age;
    char name[32];
};

/* ctx = the resource, args = what the caller passes to execute() */
static void perform_greeting(void *ctx, void *args) {
    const struct child_data_s *child = ctx;
    const char *intro = args;
    if (child)
        printf("[%s] %s I am %d years old.\n",
               child->name, intro, child->age);
}

int main(void) {
    ttak_shared_t shared;
    ttak_shared_init(&shared);
    if (shared.allocate_typed(&shared, sizeof(struct child_data_s) * 2,
                              "children", TTAK_SHARED_LEVEL_3)
        != TTAK_OWNER_SUCCESS) {
        ttak_shared_destroy(&shared);
        return 1;
    }

    ttak_owner_t *parent = ttak_owner_create(TTAK_OWNER_SAFE_DEFAULT);
    if (!parent) {
        ttak_shared_destroy(&shared);
        return 1;
    }
    shared.add_owner(&shared, parent);

    /* 1. Setup: lock, write, register, RELEASE. */
    ttak_shared_result_t res;
    struct child_data_s *kids =
        (struct child_data_s *)shared.access(&shared, parent, &res);
    if (kids && res == TTAK_OWNER_SUCCESS) {
        kids[0].age = 7;
        snprintf(kids[0].name, sizeof kids[0].name, "Cheolsoo");
        kids[1].age = 5;
        snprintf(kids[1].name, sizeof kids[1].name, "Younghee");

        ttak_owner_register_func(parent, "greet", perform_greeting);
        ttak_owner_register_resource(parent, "child_1", &kids[0]);
        ttak_owner_register_resource(parent, "child_2", &kids[1]);

        shared.release(&shared);   /* open the door */
    }

    /* 2. Execution: execute() takes its own lock internally. */
    puts("--- Parent: \"Say hello to your grandparents.\" ---");
    ttak_owner_execute(parent, "greet", "child_1", "Hello!");
    ttak_owner_execute(parent, "greet", "child_2", "Hi there!");

    /* 3. Teardown: the owner first, then the data it pointed into. */
    ttak_owner_destroy(parent);
    ttak_shared_destroy(&shared);
    return 0;
}
