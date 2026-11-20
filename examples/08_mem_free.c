/* Excerpt from src/mem/mem.c (embedded branches and tracing omitted) */
void TTAK_HOT_PATH ttak_mem_free(void *ptr) {
    if (!ptr) return;

    /* Keep a stable copy instead of working on the argument directly. */
    void *stable_ptr = ptr;
    ttak_mem_header_t *header = GET_HEADER(stable_ptr);

    /* Atomic exchange: a second free of the same block is a no-op. */
    if (ttak_mem_freed_exchange(header, 1)) return;

    V_HEADER(stable_ptr);   /* verify magic number and checksum */

    /* Roots are tracked by the global memory tree: unlink them first. */
    if (header->is_root &&
        (header->allocation_tier == TTAK_ALLOC_TIER_GENERAL ||
         header->allocation_tier == TTAK_ALLOC_TIER_BUDDY)) {
        pthread_mutex_lock(&global_map_lock);
        size_t node_val = 0;
        if (ttak_map_get_key(global_ptr_map, (uintptr_t)stable_ptr,
                             &node_val, 0))
            ttak_mem_tree_remove(&global_mem_tree,
                                 (ttak_mem_node_t *)node_val);
        ttak_delete_from_map(global_ptr_map, (uintptr_t)stable_ptr, 0);
        pthread_mutex_unlock(&global_map_lock);
    }

    ttak_atomic_sub64(&global_mem_usage, header->mapped_size);

    /* Hand the block back to the tier that produced it. */
    switch (header->allocation_tier) {
        case TTAK_ALLOC_TIER_POCKET:  _pocket_free_internal(header); break;
        case TTAK_ALLOC_TIER_VMA:     _vma_free_internal(header);    break;
        case TTAK_ALLOC_TIER_BUDDY:   /* fall through on hosted builds */
        case TTAK_ALLOC_TIER_GENERAL: _large_free_internal(header);  break;
        default:                      _large_free_internal(header);  break;
    }
}
