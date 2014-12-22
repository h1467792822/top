
#include <top/core/alloc.h>
#include <malloc.h>


top_error_t top_glibc_malloc(struct top_alloc* self,unsigned long size,void** palloc)
{
    void* p = malloc(size);
    if(p) {
        *palloc = p;
        return TOP_OK;
    }
    return TOP_ERROR(-1);
}

top_error_t top_glibc_calloc(struct top_alloc* self,unsigned long size,void** palloc)
{
    void* p = calloc(1,size);
    if(p) {
        *palloc = p;
        return TOP_OK;
    }
    return TOP_ERROR(-1);
}

top_error_t top_glibc_memalign(struct top_alloc* self,unsigned long alignment,unsigned long size,void** palloc)
{
    void* p = memalign(alignment,size);
    if(p) {
        *palloc = p;
        return TOP_OK;
    }
    return TOP_ERROR(-1);
}

void top_glibc_free(struct top_alloc* self,void* p)
{
    free(p);
}

static struct top_alloc g_top_glibc_alloc_policy = {
    .malloc = top_glibc_malloc,
    .memalign = top_glibc_memalign,
    .calloc = top_glibc_calloc,
    .free = top_glibc_free,
};

struct top_alloc* g_top_glibc_alloc = &g_top_glibc_alloc_policy;

