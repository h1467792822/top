
#ifndef TB_CTX_H
#define TB_CTX_H

#include <inttypes.h>
#include <sys/uio.h>
#include <event2/event_struct.h>
#include "tb_list.h"
#include "state.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CTX_BUF_COUNT (4)

struct proxy_ctx {
	uint64_t req_flags;
	uint8_t rsp_idx;
	uint8_t rsp_flags;
	struct iovec req_vec[CTX_BUF_COUNT];
	struct iovec rsp_vec[CTX_BUF_COUNT];
	struct state_machine req_sm;
	struct state_machine rsp_sm;
	struct event req_fd;
	struct event rsp_fd;
	struct state_machine conn_sm;
	tb_list_node_t to_srv;
};

struct server_ctx {
	uint8_t flags;
	uint32_t max_conn_ps;
	tb_list_t conns;
	struct event fd;
};

struct proxy_ctx* proxy_ctx_alloc();

void proxy_ctx_free(struct proxy_ctx* ctx);

#ifdef __cplusplus
}
#endif

#endif

