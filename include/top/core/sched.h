
#ifndef TOP_CORE_SCHED_H
#define TOP_COre-SCHED_H

#include <top/core/setjmp.h>
#include <top/core/pthread.h>
#include <top/core/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*top_task_main)(struct top_task_s* task,void* main_data);

typedef enum top_task_state_enum {
	TOP_TASK_INIT = 0,
	TOP_TASK_PENDING,
	TOP_TASK_RUNNING,
	TOP_TASK_COND_WAIT,
	TOP_TASK_LOCK_WAIT,
	TOP_TASK_SLEEP,
	TOP_TASK_EXIT,
}top_task_state_e;

typedef struct top_task_s {
	int state;
	int flags;
	struct top_sch_s* sch;
	struct top_list_node to_sch;
	top_jmp_buf context;
	void* main_data;
	top_task_main main;
}top_task_t;

typedef struct top_task_stat_s {
} top_task_stat_t;

void top_task_init(struct top_task_s* task, top_task_main main,void* main_data);
void* top_task_join(struct top_task_s* task,struct top_task_stat_s* stat);
top_error_t top_task_attach(struct top_task_s* task,struct top_sched_s* sch);

void top_task_yield(struct top_task_s* task);

typedef struct top_sched_s {
	struct top_list running;
	struct top_list waiting;
	struct top_list sleeping;
	struct top_task_s idle;
	top_pthread_t tid;
	struct top_task_s* current;
	const struct top_pthread_conf_s* conf;
}top_sched_t;

struct top_sched_s* top_current_sched();
struct top_task_s* top_current_task();

typedef struct top_sched_stat_s {
}top_sched_stat_t;

top_error_t top_sched_init(struct top_sched_s* sch,const struct top_pthread_conf_s* conf);
void top_sched_join(struct top_sched_s* sch,struct top_sched_stat_s* stat);



#ifdef __cplusplus
}
#endif

#endif

