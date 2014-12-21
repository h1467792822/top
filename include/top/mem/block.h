
#ifndef TOP_MEM_BLOCK_H
#define TOP_MEM_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

top_error_t top_block_alloc(struct top_block_pool* pool,void** pallocated);
void top_block_free(struct top_block_pool* pool,void* p)

#ifdef __cplusplus
}
#endif

#endif

