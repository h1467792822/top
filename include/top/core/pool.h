
#ifndef TOP_CORE_POOL_H
#define TOP_CORE_POOL_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifndef TOP_CORE_STACK_H
#include <top/core/stack.h>
#endif

#ifndef TOP_CORE_LIST_H
#include <top/core/hlist.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_pool_conf
{
	top_error_t (*alloc_page)(void* user_data,unsigned long size,void** ppage);
	top_error_t (*memalign)(void* user_data,unsigned long alignment,unsigned long size,void** ppage);
	void (*free_page)(void* user_data,void* page);
	void* user_data;
	unsigned long max_capacity;
	unsigned long page_size;
};

struct top_pool
{
	struct top_hlist full_cached;
	struct top_hlist partial_cached;
	unsigned int full_cached_count;
	unsigned int partial_cached_count;
	unsigned int pages_count;
	struct top_hlist pages;
	unsigned long capacity;
	unsigned long block_count_per_page;
	struct top_pool_conf conf;
};

void top_pool_init(struct top_pool* cache,struct top_pool_conf* conf);

void top_pool_fini(struct top_pool* cache);

top_error_t top_pool_alloc(struct top_pool* cache,void** pallocated);

void top_pool_free(struct top_pool* cache,void* allocated);

void top_pool_free_cached(struct top_pool* cache);

#ifdef __cplusplus
}
#endif

#endif

