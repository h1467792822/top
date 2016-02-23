
#ifndef TOP_CORE_COROUTINE_H
#define TOP_CORE_COROUTINE_H

#ifdef __cplusplus
extern "C" {
#endif

struct top_sched;
struct top_coroutine;
typedef struct top_coroutine_conf{
	void* (*main)(struct top_coroutine* current,void* data);
	void* data;
	void* ret;
}top_coroutine_conf;

typedef struct top_coroutine {
	ucontext_t context;
	top_sched* sched;
	top_list_node to_sched;
	top_coroutine_conf conf;
	unsigned int flag;
	unsigned int state;
}top_coroutine;

top_coroutine* top_coroutine_self(void);
void top_coroutine_yield(top_coroutine* co);
void top_coroutine_suspend(top_coroutine* co);
void top_coroutine_resume(top_coroutine* co);

#ifdef __cplusplus
}
#endif

#endif

