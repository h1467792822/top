
#include "ctx.h"
#include "mem.h"
#include <assert.h>
#include <string.h>


#define CTX_ALLOC_COUNT (40 * 1024)


static struct proxy_ctx** cached_proxy_ctx = 0;
static struct proxy_ctx* proxy_ctx_block_beg = 0;
static struct proxy_ctx* proxy_ctx_block_end = 0;


static inline struct proxy_ctx* new_proxy_ctx_from_block()
{
	struct proxy_ctx* proxy_ctx = 0;
	if(proxy_ctx_block_beg == proxy_ctx_block_end) {
		proxy_ctx_block_beg = tb_malloc(sizeof(struct proxy_ctx) * CTX_ALLOC_COUNT);
		if (!proxy_ctx_block_beg) {
			proxy_ctx_block_end = 0;
			goto out;
		}
		memset(proxy_ctx_block_beg,0,sizeof(struct proxy_ctx) * CTX_ALLOC_COUNT);
		proxy_ctx_block_end = proxy_ctx_block_beg + CTX_ALLOC_COUNT;
	}
	proxy_ctx = proxy_ctx_block_beg++;
out:
	return proxy_ctx;
}


static inline struct proxy_ctx* new_proxy_ctx_from_cached()
{
	struct proxy_ctx* proxy_ctx;
	assert(cached_proxy_ctx);
	proxy_ctx = (struct proxy_ctx*)cached_proxy_ctx;
	cached_proxy_ctx = (struct proxy_ctx**)*cached_proxy_ctx;
	*(struct proxy_ctx**)proxy_ctx = 0;
	return proxy_ctx;
}


static inline void free_proxy_ctx(struct proxy_ctx* proxy_ctx)
{
	memset(proxy_ctx,0,sizeof(*proxy_ctx));
	*(struct proxy_ctx**)proxy_ctx = (struct proxy_ctx*)cached_proxy_ctx;
	cached_proxy_ctx = (struct proxy_ctx**)proxy_ctx;
}

struct proxy_ctx* proxy_ctx_alloc()
{
	if(cached_proxy_ctx)
		return new_proxy_ctx_from_cached();
	else
		return new_proxy_ctx_from_block();
}

void proxy_ctx_free(struct proxy_ctx* proxy_ctx)
{
	assert(proxy_ctx);
	free_proxy_ctx(proxy_ctx);
}
 
