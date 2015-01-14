
#ifndef TOP_CORE_SCHED_H
#define TOP_CORE_SCHED_H

#ifndef TOP_CORE_SETJMP_H
#include <top/core/setjmp.h>
#endif

#ifndef TOP_CORE_PTHREAD_H
#include <top/core/pthread.h>
#endif

#ifndef TOP_CORE_LIST_H
#include <top/core/list.h>
#endif

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifndef TOP_CORE_POOL_H
#include <top/core/pool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_task_s;
struct top_sched_s;

typedef void* (*top_task_main_loop)(struct top_task_s* task,void* main_data);

typedef enum top_task_state_enum {
	TOP_TASK_ST_INIT = 0,
	TOP_TASK_ST_PENDING,
	TOP_TASK_ST_RUNNING,
	TOP_TASK_ST_SUSPEND,
	TOP_TASK_ST_COND_WAIT,
	TOP_TASK_ST_LOCK_WAIT,
	TOP_TASK_ST_EVENT_WAIT,
	TOP_TASK_ST_SLEEP,
	TOP_TASK_ST_EXIT,
}top_task_state_e;

typedef void (*top_task_sig_handler)(struct top_task_s* task,int evt,void* data);

typedef struct top_task_conf_s {
	int evt_max;
	int rt_evt_max;
	int rt_evt_max_pending;
}top_task_conf_t;

typedef struct top_task_rt_event_queue_s {
	void* events;
	int count;
	top_task_sig_handler handler;
}top_task_rt_event_queue_t;

typedef struct top_task_event_s {
	top_task_sig_handler handler;
}top_task_event_t;

// sizeof(empty struct) == sizeof(size_t) in c++
// sizeof(empty struct) == 0 in c
typedef struct top_task_stat_s {
	unsigned long sched_times;
} top_task_stat_t;

typedef struct top_task_s {
	int state;
	int flags;
	struct top_sched_s* sched;
	struct top_list_node node;
	top_task_stat_t stat;
	top_jmp_buf context;
	top_task_rt_event_queue_t* rt_events;
	top_task_event_t* events;
	top_task_conf_t conf;
	top_pool_t pool;
	void* main_data;
	top_task_main_loop main;
	void* volatile retval;
	struct top_task_s * join_task;
	struct top_join_cond_s* join_cond;
}top_task_t;

/**
  * can be called anywhere
  */
void top_task_init(struct top_task_s* task, top_task_conf_t* conf,top_task_main_loop main,void* main_data);
top_error_t top_task_active(struct top_task_s* task,struct top_sched_s* sch);
top_error_t top_task_resume(struct top_task_s* task);
top_error_t top_task_join(struct top_task_s* task);
top_error_t top_task_signal(struct top_task_s* task, int sig_no);
top_error_t top_task_rt_signal(struct top_task_s* task, int sig_no,void* data);

/**
  * only called in task
  */
top_error_t top_task_restart(struct top_task_s* task,struct top_sched_s* sch);
top_error_t top_task_sigaction(struct top_task_s* task, int evt,top_task_sig_handler handler);
top_error_t top_task_rt_sigaction(struct top_task_s* task, int evt,top_task_sig_handler handler);
void top_task_suspend(struct top_task_s* task);
void top_task_yield(struct top_task_s* task);
void top_task_sleep(struct top_task_s* task,unsigned long secs);
void top_task_usleep(struct top_task_s* task,unsigned long usecs);

typedef struct top_sched_stat_s {
	unsigned long sched_times;
}top_sched_stat_t;

typedef struct top_sched_s {
	struct top_list running;
	struct top_list_node* imm_pos;
	struct top_list waiting;
	struct top_list sleeping;
	struct top_task_s idle;
	top_pthread_t tid;
	struct top_task_s* current;
	struct top_pthread_conf_s* conf;
	struct top_task_s * join_task;
	top_sched_stat_t stat;
	int retval;
	volatile int terminated;
	void* sem;
	top_jmp_buf context;
}top_sched_t;

void top_sched_main_loop(struct top_sched_s* sched);

struct top_sched_s* top_current_sched();
struct top_task_s* top_current_task();

top_error_t top_sched_init(struct top_sched_s* sch,const struct top_pthread_conf_s* conf);
void top_sched_terminate(struct top_sched_s* sch);
top_error_t top_sched_join(struct top_sched_s* sch);
void top_sched_fini(struct top_sched_s* sch);

#define TOP_TASK_SUSPEND(task) \
	do { \
	if(0 == top_setjmp((task)->context)) { \
		top_schedule((task)->sched); \
	}\
	}while(0)

#define TOP_TASK_RESUME(task) \
		top_list_add_tail(&(task)->sched->running,&(task)->node)
	

#ifdef __cplusplus
}
#endif

#endif

