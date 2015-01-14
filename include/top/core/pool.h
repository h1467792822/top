
#ifndef TOP_CORE_POOL_H
#define TOP_CORE_POOL_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifndef TOP_CORE_STACK_H
#include <top/core/stack.h>
#endif

#ifndef TOP_CORE_LIST_H
#include <top/core/list.h>
#endif

#ifndef TOP_CORE_RBTREE_H
#include <top/core/rbtree.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_alloc;
typedef struct top_pool_conf
{
	struct top_alloc* alloc;
	unsigned long max_capacity;
	unsigned int page_size; 
}top_pool_conf_t;

typedef struct top_pool
{
	struct top_list pages;
	struct top_pool_page* current;
	struct top_rbtree large;
	unsigned long capacity;
	unsigned int pages_count;
	unsigned int large_count;
	unsigned long page_reuse_count;
	unsigned long alloc_count;
	unsigned long free_count;
	unsigned long large_alloc_count;
	unsigned long large_free_count;
	struct top_pool_conf conf;
}top_pool_t;

void top_pool_init(struct top_pool* pool,const struct top_pool_conf* conf);

void top_pool_fini(struct top_pool* pool);

top_error_t top_pool_malloc(struct top_pool* pool,unsigned long size,void** pallocated);
top_error_t top_pool_calloc(struct top_pool* pool,unsigned long size,void** pallocated);
top_error_t top_pool_memalign(struct top_pool* pool,unsigned long alignment,unsigned long size,void** pallocated);

void top_pool_free(struct top_pool* pool,void* allocated);

void top_pool_free_cached(struct top_pool* pool);

#ifdef __cplusplus
}
#endif

#endif

