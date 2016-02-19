
#ifndef TB_MEM_H
#define TB_MEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


struct mem {
	void* (*malloc)(size_t size);
	void (*free)(void* p);
	void* (*realloc)(void* p, size_t size);
	void* (*calloc)(size_t nmemb,size_t size);
};

void mem_init(struct mem* mem);

void* tb_malloc(size_t size);

void tb_free(void* p);

void* realloc(void* p,size_t size);

void* calloc(size_t nmemb,size_t size);

#ifdef __cplusplus
}
#endif

#endif

