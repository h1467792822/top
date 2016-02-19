
#include "ctx.h"
#include "mem.h"
#include <assert.h>
#include <string.h>


#define CTX_ALLOC_COUNT (40 * 1024)


static struct ctx** cached_ctx = 0;
static struct ctx* ctx_block_beg = 0;
static struct ctx* ctx_block_end = 0;


static inline struct ctx* new_ctx_from_block()
{
	struct ctx* ctx;
	if(ctx_block_beg == ctx_block_end) {
		ctx_block_beg = tb_malloc(sizeof(struct ctx) * CTX_ALLOC_COUNT);
		if (!ctx_block_beg) {
			ctx_block_end = 0;
			goto out;
		}
		memset(ctx,0,sizeof(struct ctx) * CTX_ALLOC_COUNT);
		ctx_block_end = ctx + CTX_ALLOC_COUNT;
	}
	ctx = ctx_block_beg++;
out:
	return ctx;
}


static inline struct ctx* new_ctx_from_cached()
{
	struct ctx* ctx;
	assert(cached_ctx);
	ctx = (struct ctx*)cached_ctx;
	cached_ctx = (struct ctx**)*cached_ctx;
	*(struct ctx**)ctx = 0;
	return ctx;
}


static inline void free_ctx(struct ctx* ctx)
{
	memset(ctx,0,sizeof(*ctx));
	*(struct ctx**)ctx = (struct ctx*)cached_ctx;
	cached_ctx = (struct ctx**)ctx;
}

struct ctx* ctx_alloc()
{
	if(cached_ctx)
		return new_ctx_from_cached();
	else
		return new_ctx_from_block();
}

void ctx_free(struct ctx* ctx)
{
	assert(ctx);
	free_ctx(ctx);
}
 
