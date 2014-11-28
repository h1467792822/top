
#ifndef TOP_MEM_POOL_H
#define TOP_MEM_POOL_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct top_mem_pool;

typedef top_error_t (*top_mem_obj_ctor)(void* self,va_list args);
typedef void (*top_mem_obj_dtor)(void* self);
struct top_mem_pool_operations {
	void (*destroy)(struct top_mem_pool * self);
	top_error_t (*malloc)(struct top_mem_pool* self,size_t size,void** pallocated);		
	top_error_t (*calloc)(struct top_mem_pool* self,size_t size,void** pallocated);		
	top_error_t (*realloc)(struct top_mem_pool* self,void* old,size_t size,void** pallocated);
	void (*free)struct top_mem_pool* self,void* allocated);
	top_error_t (*alloc_object)(struct top_mem_pool* self,top_mem_obj_dtor,top_mem_obj_ctor,...);

	// pool management
	size_t (*get_allocated)(struct top_mem_pool * self);
};

typedef struct top_mem_pool {
	struct top_mem_pool_operations* ops;
} top_mem_pool_t;



#ifdef __cplusplus
}
#endif

#endif

