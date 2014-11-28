
#ifndef TOP_CORE_STDDEF_H
#define TOP_CORE_STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#define top_offset_of(type,member) ((size_t)(&((type*)0)->member))
#define top_container_of(ptr,type,member) ((type*)((const char*)ptr - top_offset_of(type,member)))

#ifdef __cplusplus
}
#endif


#endif

