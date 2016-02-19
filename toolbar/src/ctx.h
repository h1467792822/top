
#ifndef TB_CTX_H
#define TB_CTX_H

#include <event2/event_struct.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CTX_BUF_COUNT (4)

struct ctx {
	uint64_t req_flags;
	uint8_t rsp_idx;
	uint8_t rsp_flags;
	struct iovec req_vec[CTX_BUF_COUNT];
	struct iovec rsp_vec[CTX_BUF_COUNT];
	struct event req_ev;
	struct event rsp_ev;
};


struct ctx* ctx_alloc();

void ctx_free(struct ctx* ctx);

#ifdef __cplusplus
}
#endif

#endif

