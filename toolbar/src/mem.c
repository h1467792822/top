
#include "mem.h"
#include <stdlib.h>
#include <assert.h>


struct mem {
	void* (*malloc)(size_t size);
	void (*free)(void* p);
	void* (*realloc)(void* p, size_t size);
	void* (*calloc)(size_t nmemb,size_t size);
};


static struct mem g_mem = {
	malloc, free, realloc,calloc
};

void mem_init(struct mem* mem)
{
	assert(mem);
	if(mem->malloc) g_mem.malloc = mem->malloc;
	if(mem->realloc) g_mem.realloc = mem->realloc;
	if(mem->calloc) g_mem.calloc = mem->calloc;
	if(mem->free) g_mem.free = mem->free;	
}

void* tb_malloc(size_t size){
	return g_mem.malloc(size);
}

void tb_free(void* p){
	g_mem.free(p);
}

void* realloc(void* p,size_t size){
	return g_mem.realloc(p,size);
}

void* calloc(size_t nmemb,size_t size){
	return g_mem.calloc(nmemb,size);
}

#ifdef __cplusplus
}
#endif

#endif

