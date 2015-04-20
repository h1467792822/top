
#ifndef TOP_CO_CORE_SCHED_H
#define TOP_CO_CORE_SCHED_H

#include <inttypes.h>
#include <ucontext.h>

#ifndef TOP_CORE_PTHREAD_H
#include <top/core/pthread.h>
#endif

#ifndef TOP_CORE_LIST_H
#include <top/core/list.h>
#endif

#ifndef TOP_CORE_STACK_H
#include <top/core/stack.h>
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

struct top_co_task_s;
struct top_co_sched_s;

typedef enum top_co_task_state_e {
	TOP_CO_TASK_ST_INIT = 0,
	TOP_CO_TASK_ST_RUNNING,
	TOP_CO_TASK_ST_PENDING,
	TOP_CO_TASK_ST_SUSPEND,
	TOP_CO_TASK_ST_EXITED,
}top_co_task_state_e;

typedef enum top_co_task_flag_e {
	TOP_CO_TASK_FL_INIT = 0,
	TOP_CO_TASK_FL_RUN = 1u,
	TOP_CO_TASK_FL_EXIT = 1u << 1,
}top_co_task_flag_e;

typedef struct top_co_task_stat_s {
    unsigned long sched_times;
} top_task_stat_t;

typedef struct top_co_task_conf_s {
	void* (*main_loop)(struct top_co_task_s* task,void* main_data);
	void* main_data;
} top_task_conf_t;

typedef struct top_co_task_s {
	top_task_conf_t conf;
	top_task_stat_t stat;
	top_co_task_state_e state;
	unsigned int flags __top_cache_aligned;
	top_list_node_t to_sched;
	ucontext_t context;
	void* sp;
	unsigned int sp_size;
}top_co_task_t;

/**
  * can be called anywhere
  */
void top_co_task_init(struct top_co_task_s* task, top_co_task_conf_t* conf);
top_error_t top_co_task_active(struct top_co_task_s* task,struct top_co_sched_s* sch);
top_error_t top_co_task_resume_r(struct top_co_task_s* task);
top_error_t top_co_task_join_r(struct top_co_task_s* task);


/**
  * only called in task
  */
top_error_t top_co_task_restart(struct top_co_task_s* task,struct top_sched_s* sch);
void top_task_suspend(struct top_co_task_s* task);
void top_task_yield(struct top_co_task_s* task);
void top_task_sleep(struct top_co_task_s* task,unsigned long secs);
void top_task_usleep(struct top_co_task_s* task,unsigned long usecs);

typedef struct top_sched_stat_s {
    unsigned long sched_times;
} top_sched_stat_t;

#define TOP_SCHED_FL_ASYN (1u << 1)
#define TOP_SCHED_FL_EXIT (1u << 2)
#define TOP_SCHED_FL_EXITED (1u << 3)

typedef struct top_sched_s {
    struct top_list all_tasks;
    struct top_list running;
    struct top_stack asyn_running;
    struct top_list_node* imm_pos;
    struct top_task_s idle;
    top_pthread_t tid;
    struct top_task_s* current;
    struct top_pthread_conf_s* conf;
    struct top_task_s * join_task;
    top_sched_stat_t stat;
    volatile unsigned int flags;
    int retval;
    unsigned int task_count;
    void* sem;
    void* mutex;
    //top_jmp_buf main_context;
    //top_jmp_buf sched_context;
	ucontext_t main_context;
	ucontext_t sched_context;
} top_sched_t;

struct top_sched_s* top_current_sched();
struct top_task_s* top_current_task();

top_error_t top_sched_init(struct top_sched_s* sched,const struct top_pthread_conf_s* conf);
void top_sched_terminate(struct top_sched_s* sched);
top_error_t top_sched_join(struct top_sched_s* sched);
void top_sched_fini(struct top_sched_s* sched);
void top_sched_lock(struct top_sched_s* sched);
void top_sched_unlock(struct top_sched_s* sched);

#define TOP_RESCHEDULE(sched) \
	top_longjmp((sched)->sched_context,1)
	//top_schedule(sched)
	//top_longjmp((sched)->sched_context,1)
	//top_schedule(sched)

#define TOP_TASK_SUSPEND(task) \
	do { \
		if((task)->state == TOP_TASK_ST_SIGNAL){ \
			printf("\n signal set_jmp: task: %p,jmp_buf: %p\n",(task),(task)->sig_context); \
			swapcontext(&(task)->sig_context,&(task)->sched->sched_context);\
		}else {\
			printf("\n running set_jmp: task: %p, jmp_buf:%p\n",(task),(task)->main_context); \
			swapcontext(&(task)->main_context,&(task)->sched->sched_context);\
		}\
	}while(0)

#define TOP_TASK_RESUME(task) \
		top_task_resume(task)


#ifdef __cplusplus
}
#endif

#endif

