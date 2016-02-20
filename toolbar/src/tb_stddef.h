
#ifndef TB_STDDEF_H
#define TB_STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#define tb_offset_of(type,member) ((unsigned long)(&((type*)0)->member))
#define tb_container_of(ptr,type,member) ((type*)((const char*)(ptr) - tb_offset_of(type,member)))

#ifdef __cplusplus
}
#endif


#endif

