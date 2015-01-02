
#ifndef TOP_CORE_SCH_H
#define TOP_COre-SCH_H

#include <top/core/setjmp.h>
#include <top/core/pthread.h>
#include <top/core/list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct top_runable_s {
	struct top_sch_s* sch;
	struct top_list_node to_sch;
	top_jmp_buf context;
};

struct top_sch_s {
	struct top_list running;
	struct top_list runnables;
	top_pthread_t tid;
};

static inline void top_runnable_pending(struct top_runnable_s* s)
{
	top_list_add_tail(&s->sch->running,&s->to_sch);
}

static inline void top_runnable_suspend(struct top_runnable_s* s)
{
	if (top_setjmp(&s->context) == 0){
		top_schedule(s->sch);
	}
}

static inline void top_runnable_active(struct top_runnable_s* s) {
	top_longjmp(&s->context);
}

#ifdef __cplusplus
}
#endif

#endif

