
#include <top/core/alloc.h>
#include <malloc.h>


top_error_t top_glibc_malloc(void* user_data,unsigned long size,void** palloc)
{
    void* p = malloc(size);
    if(p) {
        *palloc = p;
        return TOP_OK;
    }
    return TOP_ERROR(-1);
}

top_error_t top_glibc_calloc(void* user_data,unsigned long size,void** palloc)
{
    void* p = calloc(1,size);
    if(p) {
        *palloc = p;
        return TOP_OK;
    }
    return TOP_ERROR(-1);
}

top_error_t top_glibc_memalign(void* user_data,unsigned long alignment,unsigned long size,void** palloc)
{
    void* p = memalign(alignment,size);
    if(p) {
        *palloc = p;
        return TOP_OK;
    }
    return TOP_ERROR(-1);
}

void top_glibc_free(void* user_data,void* p)
{
    free(p);
}
