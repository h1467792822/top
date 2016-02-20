
#ifndef TB_STATE_H
#define TB_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

enum proxy_conn_state {
	PROXY_CONN_ST_INIT = 0,
	PROXY_CONN_ST_WAIT_R,
	PROXY_CONN_ST_OK,
	PROXY_CONN_ST_EOF,
	PROXY_CONN_ST_NULL
};

enum proxy_conn_event {
	PROXY_CONN_EV_R_OK = 0,
	PROXY_CONN_EV_R_FAIL,
	PROXY_CONN_EV_L_HUP,
	PROXY_CONN_EV_CLOSE,
	PROXY_CONN_EV_NULL,
};

enum proxy_data_state {
	PROXY_DATA_ST_WAIT_READ = 0,
	PROXY_DATA_ST_DOING ,
	PROXY_DATA_ST_WAIT_WRITE,
	PROXY_DATA_ST_EOF,
	PROXY_DATA_ST_NULL,
};

enum proxy_data_event {
	PROXY_DATA_EV_READ = 0,
	PROXY_DATA_EV_WRITE,
	PROXY_DATA_EV_R_HUP,
	PROXY_DATA_EV_W_HUP,
	PROXY_DATA_EV_CLOSE,
	PROXY_DATA_EV_NULL,
};


struct state_machine;
struct state{
	int st;
	int (*on_event)(struct state_machine* sm,int event,void* data);
	void (*on_enter)(struct state_machine* sm,int from_state);
	void (*on_leave)(struct state_machine* sm,int to_state);
};

struct state_machine {
	int cur_st;
	int running;
	struct state* states;
};

void proxy_conn_sm_init(struct state_machine* sm);
void proxy_data_sm_init(struct state_machine* sm);
int sm_notify(struct state_machine* sm,int event,void* data);

#ifdef __cplusplus
}
#endif

#endif

