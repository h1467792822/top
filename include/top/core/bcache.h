
#ifndef TOP_CORE_BCACHE_H
#define TOP_CORE_BCACHE_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifndef TOP_CORE_STACK_H
#include <top/core/stack.h>
#endif

#ifndef TOP_CORE_HLIST_H
#include <top/core/hlist.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_bcache_conf
{
	top_error_t (*alloc_page)(void* user_data,unsigned long size,void** ppage);
	void (*free_page)(void* user_data,void* page,unsigned long size);
	void* user_data;
	unsigned long max_capacity;
	unsigned long page_size;
	unsigned short block_size;
};

struct top_bcache
{
	struct top_hlist full_cached;
	struct top_hlist partial_cached;
	unsigned int full_cached_count;
	unsigned int partial_cached_count;
	unsigned int pages_count;
	unsigned short block_size;
	struct top_hlist pages;
	unsigned long capacity;
	unsigned long block_count_per_page;
	struct top_bcache_conf conf;
};

void top_bcache_init(struct top_bcache* cache,struct top_bcache_conf* conf);

void top_bcache_fini(struct top_bcache* cache);

top_error_t top_bcache_alloc(struct top_bcache* cache,void** pallocated);

void top_bcache_free(struct top_bcache* cache,void* allocated);

void top_bcache_free_cached(struct top_bcache* cache);

#ifdef __cplusplus
}
#endif

#endif

