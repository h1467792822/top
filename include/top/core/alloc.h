
#ifndef TOP_CORE_ALLOC_H
#define TOP_CORE_ALLOC_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_alloc {
    top_error_t (*malloc)(struct top_alloc* self,unsigned long size,void** palloc);
    top_error_t (*calloc)(struct top_alloc* self,unsigned long size,void** palloc);
    top_error_t (*memalign)(struct top_alloc* self,unsigned long alignment,unsigned long size,void** palloc);
    void (*free)(struct top_alloc* self,void* p);
};

extern struct top_alloc* g_top_glibc_alloc;

static inline top_error_t top_malloc(struct top_alloc* self,unsigned long size,void** palloc)
{
    return self->malloc(self,size,palloc);
}

static inline top_error_t top_calloc(struct top_alloc* self,unsigned long size,void** palloc)
{
    return self->calloc(self,size,palloc);
}

static inline top_error_t top_memalign(struct top_alloc* self,unsigned long alignment,unsigned long size,void** palloc)
{
    return self->memalign(self,alignment,size,palloc);
}

static inline void top_free(struct top_alloc* self,void* p)
{
    return self->free(self,p);
}


#ifdef __cplusplus
}
#endif

#endif

