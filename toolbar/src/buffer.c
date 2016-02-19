
#include "buffer.h"
#include <sys/uio.h>
#include "mem.h"


#define BUF_COUNT_IN_BLOCK (1024)
#define BLOCK_SIZE (BUF_LEN * BUF_COUNT_IN_BLOCK)

static void** cached_buff = 0;
static void* new_block_beg = 0;
static void* new_block_end = 0;


static inline void* new_buff_from_block()
{
	void* buf = 0;
	if (new_block_beg == new_block_end) {
		new_block_beg = tb_malloc(BLOCK_SIZE);
		if( 0 == new_block_beg) {
			new_block_end = 0;
			goto out;
		}
		new_block_end = (char*)new_block_end + BLOCK_SIZE;
	}
	buf = new_block_beg;
	new_block_beg = (char*)new_block_beg + BUF_LEN;
out:
	return buf;
}


static inline void* new_buff_from_cached()
{
	void* buf = cached_buff;
	cached_buff = (void**)*cached_buff;	
}


static inline void* new_buff()
{
	if(cached_buff)
		return new_buff_from_cached();
	else
		return new_buff_from_block();
}

int buff_vec_alloc(struct iovec* vec,int n)
{
	int i = 0;
	for(; i < n; ++i) {
		vec[i].iov_base = new_buff();
		if(!vec[i].iov_base) goto out;
		vec[i].iov_len = BUF_LEN;
	}
out:
	return i;
}


static inline void free_buff(void* buf)
{
	assert(buf);
	void** pbuf = (void**)buf;
	*pbuf = cached_buff;
	cached_buff = pbuf;
}

void buff_vec_free(struct iovec* vec,int n)
{
	int i = 0;
	for(; i < n; ++i) {
		free_buff(vec[i].iov_base);
	}
}




