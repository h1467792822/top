



#include "buffer.h"
#include "ctx.h"
#include <sys/uio.h>
#include <stdio.h>


int main(int argc,char* argv[])
{
	int i;
	for(i = 0; i < 10; ++i){
	struct iovec vec;
	int n = buff_vec_alloc(&vec,1);
	printf("buffer_alloc: %d\n",n);	

	struct proxy_ctx* ctx;
	ctx = proxy_ctx_alloc();
	printf("proxy_ctx_alloc:%p\n",ctx);

	buff_vec_free(&vec,1);
	proxy_ctx_free(ctx);
	}
	return 1;
}

