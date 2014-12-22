
#include <top/core/bcache.h>
#include <top/core/alloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct top_bcache_page {
    struct top_hlist_node cached;
    struct top_stack blocks;
    unsigned short avail_count;
    unsigned short block_count;
};

void top_bcache_init(struct top_bcache* cache,const struct top_bcache_conf* conf)
{
    assert(conf);
    memset(cache,0,sizeof(*cache));
    cache->conf = *conf;
    if(0 == cache->conf.alloc) cache->conf.alloc = g_top_glibc_alloc;
    if(0 == cache->conf.max_capacity) cache->conf.max_capacity = (unsigned long)-1;
    if(0 == cache->conf.page_size) cache->conf.page_size = 4 * 1024;
    cache->block_size = ((cache->conf.block_size + sizeof(void*) - 1) & ~(sizeof(void*) - 1)) + sizeof(void*);
    cache->block_count_per_page = (cache->conf.page_size - sizeof(struct top_bcache_page)) / cache->block_size;
}

void top_bcache_fini(struct top_bcache* cache)
{
    struct top_bcache_page* page;
    struct top_hlist_node* tmp1,*tmp2;
    top_hlist_for_each_entry_safe(&cache->full_cached,page,cached,tmp1,tmp2) {
        top_hlist_node_del(&page->cached);
        top_free(cache->conf.alloc,page);
    }
    top_hlist_for_each_entry_safe(&cache->partial_cached,page,cached,tmp1,tmp2) {
        top_hlist_node_del(&page->cached);
        top_free(cache->conf.alloc,page);
    }
    top_hlist_for_each_entry_safe(&cache->pages,page,cached,tmp1,tmp2) {
        top_hlist_node_del(&page->cached);
        top_free(cache->conf.alloc,page);
    }
}

static inline void top_bcache_init_page(struct top_bcache* cache,struct top_bcache_page* page)
{
    top_stack_init(&page->blocks);
    page->avail_count = cache->block_count_per_page;
    page->block_count = 0;
    cache->capacity += cache->conf.page_size;
    top_hlist_add_head(&cache->partial_cached,&page->cached);
}

static inline void* top_bcache_page_pop_last(struct top_bcache* cache,struct top_bcache_page* page)
{
    void** p = (void**)((char*)(page + 1) + cache->block_size * --page->avail_count);
    *p = page;
    return p + 1;
}

top_error_t top_bcache_alloc(struct top_bcache* cache,void** pallocated)
{
    void* block;
    struct top_bcache_page* page;
    top_error_t err;
    if(top_hlist_empty(&cache->partial_cached)) {
        if(!top_hlist_empty(&cache->full_cached)) {
            top_hlist_node_move(cache->full_cached.first,&cache->partial_cached);
            --cache->full_cached_count;
            ++cache->partial_cached_count;
        } else {
            if(cache->conf.max_capacity <= cache->capacity || cache->conf.max_capacity - cache->capacity < cache->conf.page_size) {
                return TOP_ERROR(-1);
            }
            err = top_malloc(cache->conf.alloc,cache->conf.page_size,(void**)&page);
            if(top_errno(err)) return err;
            ++cache->pages_count;
            ++cache->partial_cached_count;
            top_bcache_init_page(cache,page);
        }
    }
    page = top_hlist_entry(cache->partial_cached.first,struct top_bcache_page,cached);
    if(page->avail_count) {
        block = top_bcache_page_pop_last(cache,page);
    } else {
        block = top_stack_pop(&page->blocks);
        --page->block_count;
    }
    if(0 == page->avail_count && top_stack_empty(&page->blocks)) {
        top_hlist_node_move(&page->cached,&cache->pages);
        --cache->partial_cached_count;
    }
    *pallocated = block;
    return TOP_OK;
}

void top_bcache_free(struct top_bcache* cache,void* p)
{
    struct top_bcache_page* page = (struct top_bcache_page*)*((void**)p - 1);
    struct top_stack_node* node = (struct top_stack_node*)p;
    top_stack_push(&page->blocks,node);
    ++page->block_count;
    if(0 == page->avail_count) {
        if(1 == page->block_count) {
            top_hlist_node_move(&page->cached,&cache->partial_cached);
            ++cache->partial_cached_count;
        } else if(cache->block_count_per_page == page->block_count) {
            top_hlist_node_move(&page->cached,&cache->full_cached);
            ++cache->full_cached_count;
            --cache->partial_cached_count;
        };
    }
}

void top_bcache_free_cached(struct top_bcache* cache)
{
    struct top_bcache_page* page;
    struct top_hlist_node* tmp1,*tmp2;
    top_hlist_for_each_entry_safe(&cache->full_cached,page,cached,tmp1,tmp2) {
        top_hlist_node_del(&page->cached);
        top_free(cache->conf.alloc,page);
        cache->capacity -= cache->conf.page_size;
    }
    cache->pages_count -= cache->full_cached_count;
    cache->full_cached_count = 0;
}

