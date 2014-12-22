
#include <top/core/pool.h>
#include <top/core/alloc.h>
#include <top/core/rbtree.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct top_pool_page {
    struct top_list_node node;
    unsigned int avail_size;
    unsigned int alloc_count: 28;
    unsigned int fail_count: 4;
};


struct top_pool_large {
    struct top_rbtree_node node;
    void* alloc;
    unsigned long size;
};

#define TOP_POOL_PAGE_MAX_FAIL_COUNT 4
#define TOP_POOL_PAGE_MAX_AVAIL_SIZE(pool) ((pool)->conf->page_size - sizeof(struct top_pool_page))
#define TOP_POOL_ALIGN_SIZE( size ) (((size) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))
#define TOP_POOL_PAGE_MAX_ALLOC_SIZE(pool) ((pool)->conf->page_size - sizeof(struct top_pool_page) - sizeof(void*))

static inline void top_pool_free_small(struct top_pool* pool,void* p)
{
    struct top_pool_page* page = (struct top_pool_page*)*((void**)p - 1);
    if(0 == --page->alloc_count) {
        page->avail_size = TOP_POOL_PAGE_MAX_AVAIL_SIZE(pool);
        page->fail_count = 0;
        if(pool->current == page) {
            pool->current = top_list_entry(page->node.next,struct top_pool_page,node);
        }
        top_list_node_move_tail(&page->node,&pool->pages);
        if(&pool->current->node == (struct top_list_node*)&pool->pages) {
            pool->current = page;
        }
        ++pool->page_reuse_count;
    }
    ++pool->free_count;
}

static inline top_error_t top_pool_alloc_low(struct top_pool* pool,unsigned long size,void** pp)
{
    if(pool->conf->max_capacity <= pool->capacity || pool->conf->max_capacity - pool->capacity < size) {
        return TOP_ERROR(-1);
    }
    top_error_t err = pool->conf->malloc(pool->conf->user_data,size,pp);
    if(top_errno(err) == 0) {
        pool->capacity += size;
    }
    return err;
}

static inline void top_pool_free_low(struct top_pool* pool,void* p)
{
    pool->conf->free(pool->conf->user_data,p);
}

static inline struct top_pool_large* top_pool_find_large(struct top_pool* pool,void* p)
{
    struct top_rbtree_node* parent = pool->large.root;
    struct top_pool_large* page;
    while(parent) {
        page = top_rb_entry(parent,struct top_pool_large,node);
        if(page->alloc == p) {
            return page;
        }
        if(page->alloc > p)
            parent = parent->left;
        else
            parent = parent->right;
    }
    return 0;
}

void top_pool_insert_large(struct top_pool* pool, struct top_pool_large* large)
{
    struct top_rbtree_node** p = &pool->large.root;
    struct top_rbtree_node* parent = *p;
    struct top_pool_large* page;
    while(*p) {
        parent = *p;
        page = top_rb_entry(parent,struct top_pool_large,node);
        assert(page->alloc != large->alloc);
        if(page->alloc > large->alloc) {
            p = &parent->left;
        } else {
            p = &parent->right;
        }
    }
    top_rbtree_link_node(&pool->large,&large->node,parent,p);
}

static inline void top_pool_free_large(struct top_pool* pool, struct top_pool_large* large)
{
    top_rbtree_erase(&pool->large,&large->node);
    pool->capacity -= large->size;
    top_pool_free_low(pool,large->alloc);
    top_pool_free_small(pool,large);
    --pool->large_count;
    ++pool->large_free_count;
}

void top_pool_free(struct top_pool* pool,void* p)
{
    struct top_pool_large* large = top_pool_find_large(pool,p);
    if(large) {
        top_pool_free_large(pool,large);
    } else {
        top_pool_free_small(pool,p);
    }
}

top_error_t top_pool_alloc_small(struct top_pool* pool,unsigned long size,void** pp)
{
    struct top_pool_page* page = pool->current;
    void** ppage;
    size = TOP_POOL_ALIGN_SIZE(size + sizeof(void*));
    top_list_for_each_entry_from(&pool->pages,page,node) {
        if(page->avail_size >= size) {
            ppage = (void**)((char*)page + pool->conf->page_size - page->avail_size);
            *ppage = page;
            *pp = ppage + 1;
            page->avail_size -= size;
            ++page->alloc_count;
            ++pool->alloc_count;
            return TOP_OK;
        } else {
            ++page->fail_count;
            if(page->fail_count == TOP_POOL_PAGE_MAX_FAIL_COUNT) {
                pool->current = top_list_entry(page->node.next,struct top_pool_page,node);
            }
        }
    }

    top_error_t err;
    err = top_pool_alloc_low(pool,pool->conf->page_size, (void**)&page);
    if(top_errno(err)) return err;

    ++pool->pages_count;
    ++pool->alloc_count;
    page->fail_count = 0;
    page->avail_size = TOP_POOL_PAGE_MAX_AVAIL_SIZE(pool) - size;
    page->alloc_count = 1;
    ppage = (void**)(page + 1);
    *ppage = page;
    *pp = ppage + 1;

    top_list_add_tail(&pool->pages,&page->node);
    if(&pool->current->node == (struct top_list_node*)&pool->pages) {
        pool->current = page;
    }
    return TOP_OK;
}

top_error_t top_pool_alloc_add_large(struct top_pool* pool,void* data,unsigned long size,void** pp)
{
    struct top_pool_large* large;
    top_error_t err;
    err = top_pool_alloc_small(pool,sizeof(*large),(void**)&large);
    if(top_errno(err)) {
        pool->capacity -= size;
        top_pool_free_low(pool,data);
        return err;
    }

    ++pool->large_count;
    ++pool->large_alloc_count;
    large->alloc = data;
    large->size = size;
    top_pool_insert_large(pool,large);
    *pp = data;
    return TOP_OK;
}

top_error_t top_pool_alloc_large(struct top_pool* pool,unsigned long size,void ** pp)
{
    top_error_t err;
    void* alloc;
    err = top_pool_alloc_low(pool,size,&alloc);
    if(top_errno(err)) return err;

    return top_pool_alloc_add_large(pool,alloc,size,pp);
}

top_error_t top_pool_malloc(struct top_pool* pool,unsigned long size,void ** pp)
{
    if(size > TOP_POOL_PAGE_MAX_ALLOC_SIZE(pool)) {
        return top_pool_alloc_large(pool,size,pp);
    } else {
        return top_pool_alloc_small(pool,size,pp);
    }
}

top_error_t top_pool_calloc(struct top_pool* pool,unsigned long size,void** pp)
{
    top_error_t err;
    if(size > TOP_POOL_PAGE_MAX_AVAIL_SIZE(pool)) {
        err = top_pool_alloc_large(pool,size,pp);
    } else {
        err = top_pool_alloc_small(pool,size,pp);
    }
    if(top_errno(err) == 0) {
        memset(*pp,0,size);
    }
    return err;
}

top_error_t top_pool_memalign(struct top_pool* pool,unsigned long alignment,unsigned long size,void** pp)
{
    if(alignment <= sizeof(void*)) {
        return top_pool_malloc(pool,size,pp);
    } else {
        top_error_t err;
        void* alloc;
        err = pool->conf->memalign(pool->conf->user_data,alignment,size,pp);
        if(top_errno(err)) return err;

        return top_pool_alloc_add_large(pool,alloc,size,pp);
    }
}

void top_pool_free_cached(struct top_pool* pool)
{
    struct top_pool_page* page,*tmp;
    top_list_for_each_entry_safe_reverse(&pool->pages,page,node,tmp) {
        if(page->alloc_count == 0) {
            if(pool->current == page) {
                pool->current = top_list_entry(&pool->pages,struct top_pool_page,node);
            }
            top_list_node_del(&page->node);
            top_pool_free_low(pool,page);
            --pool->pages_count;
            pool->capacity -= pool->conf->page_size;
        }
    }
}

static const struct top_pool_conf g_top_pool_def_conf = {
    .malloc = top_glibc_malloc,
    .memalign = top_glibc_memalign,
    .free = top_glibc_free,
    .max_capacity = (unsigned long)-1,
    .page_size = 4 * 1024,
};

void top_pool_init(struct top_pool* pool,const struct top_pool_conf* conf)
{
    assert(pool);
    memset(pool,0,sizeof(*pool));
    top_list_init(&pool->pages);
    pool->current = top_list_entry(&pool->pages,struct top_pool_page,node);
    pool->conf = conf ? conf : &g_top_pool_def_conf;
}

void top_pool_fini(struct top_pool* pool)
{
    struct top_rbtree_node* node,*next;
    for(node = top_rbtree_first(&pool->large); node; node = top_rbtree_node_next(node)) {
        top_pool_free_low(pool, top_rb_entry(node,struct top_pool_large,node)->alloc);
    }

    struct top_pool_page* page,*tmp;
    top_list_for_each_entry_safe_reverse(&pool->pages,page,node,tmp) {
        top_list_node_del(&page->node);
        top_pool_free_low(pool,page);
    }
}

