
#ifndef TOP_CORE_ALLOC_H
#define TOP_CORE_ALLOC_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


	top_error_t top_glibc_malloc(void* user_data,unsigned long size,void** palloc);

	top_error_t top_glibc_calloc(void* user_data,unsigned long size,void** palloc);

	top_error_t top_glibc_memalign(void* user_data,unsigned long alignment,unsigned long size,void** palloc);

	void top_glibc_free(void* user_data,void* p);

#ifdef __cplusplus
}
#endif

#endif

