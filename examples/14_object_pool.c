#include <stdio.h>
#include <ttak/container/pool.h>

/* One node fits exactly one pool slot. */
typedef struct tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
} tree_node_t;

/* Step 2: take a free slot. NULL when the pool is exhausted. */
static tree_node_t *pool_alloc_node(ttak_object_pool_t *pool, int val) {
    tree_node_t *node = ttak_object_pool_alloc(pool);
    if (!node) {
        fprintf(stderr, "Error: pool exhausted at node %d\n", val);
        return NULL;
    }
    node->value = val;
    node->left  = NULL;
    node->right = NULL;
    return node;
}

static void traverse_inorder(const tree_node_t *node) {
    if (!node) return;
    traverse_inorder(node->left);
    printf("Visited node: %d\n", node->value);
    traverse_inorder(node->right);
}

/* Step 3: post-order, so children go back before their parent. */
static void tree_destroy(ttak_object_pool_t *pool, tree_node_t *node) {
    if (!node) return;
    tree_destroy(pool, node->left);
    tree_destroy(pool, node->right);
    ttak_object_pool_free(pool, node);
}

int main(void) {
    /* Step 1: ten slots allocated once, no malloc() per node. */
    ttak_object_pool_t *pool =
        ttak_object_pool_create(10, sizeof(tree_node_t));
    if (!pool) return 1;

    tree_node_t *root = pool_alloc_node(pool, 10);
    if (root) {
        root->left  = pool_alloc_node(pool, 20);
        root->right = pool_alloc_node(pool, 30);
    }

    if (root && root->left && root->right) {
        puts("--- In-order traversal ---");
        traverse_inorder(root);
    }

    puts("--- Cleaning up ---");
    tree_destroy(pool, root);          /* safe even if a child is NULL */

    /* Step 4: release the whole slab at once. */
    ttak_object_pool_destroy(pool);
    return 0;
}
