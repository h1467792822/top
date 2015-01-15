
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
	TOP_TASK_ST_SEM_WAIT,
	TOP_TASK_ST_SLEEP,
	TOP_TASK_ST_EXIT,
}top_task_state_e;

typedef enum top_task_flag_enum {
	TOP_TASK_FL_RUN = 1ul,
	TOP_TASK_FL_LOCK_WAIT = 1ul << 1,
	TOP_TASK_FL_COND_WAIT = 1ul << 2,
	TOP_TASK_FL_SEM_WAIT = 1ul << 3,
	TOP_TASK_FL_SLEEP = 1ul << 4,
	TOP_TASK_FL_RESUME = 1ul << 5,
	TOP_TASK_FL_SIG = 1ul << 6,
	TOP_TASK_FL_RT_SIG = 1ul << 7,
	TOP_TASK_FL_EXIT = 1ul << 15,
}top_task_flag_e;

#define TOP_TASK_SIG_EXIT 31  //task 退出是最后发送给自己的信号，用户可以注册在这个信号处理中完成资源清理动作, 保证只会被调用一次
#define TOP_TASK_SIG_TERM 30  //sched 调用terminate的时候发送给所有的task的信号，task收到这个信号后应该尽快完成退出动作, 保证只会被调用一次
#define TOP_TASK_SIG_MIN 0
#define TOP_TASK_SIG_MAX 15 
#define TOP_TASK_RT_SIG_MIN 32
#define TOP_TASK_RT_SIG_MAX 47 

typedef void (*top_task_sig_handler)(struct top_task_s* task,int evt,void* data);

typedef struct top_task_conf_s {
	int sig_max;
	int rt_sig_max;
	int rt_sig_max_pending;
}top_task_conf_t;

typedef struct top_task_rt_sig_s {
	void* sigs;
	int count;
	top_task_sig_handler handler;
}top_task_rt_sig_t;

typedef struct top_task_sig_s {
	top_task_sig_handler handler;
}top_task_sig_t;

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
	top_task_rt_sig_queue_t* rt_sigs;
	unsigned int sigmask;
	unsigned int sig_handler_mask;
	unsigned int rt_sigmask;
	unsigned int rt_sig_handler_mask;
	top_task_sig_t* sigs;
	top_task_rt_sig_t* rt_sigs;
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
	top_jmp_buf main_context;
	top_jmp_buf sched_context;
}top_sched_t;

void top_sched_main_loop(struct top_sched_s* sched);

struct top_sched_s* top_current_sched();
struct top_task_s* top_current_task();

top_error_t top_sched_init(struct top_sched_s* sch,const struct top_pthread_conf_s* conf);
void top_sched_terminate(struct top_sched_s* sch);
top_error_t top_sched_join(struct top_sched_s* sch);
void top_sched_fini(struct top_sched_s* sch);

#define TOP_RESCHEDULE(sched) \
	top_schedule(sched)

#define TOP_TASK_SUSPEND(task) \
	do { \
		printf("\n suspend task: %p\n", (task)); \
	if(0 == top_setjmp((task)->context)) { \
		TOP_RESCHEDULE((task)->sched); \
	}\
	}while(0)

#define TOP_TASK_RESUME(task) \
		top_list_add_tail(&(task)->sched->running,&(task)->node)
	

#ifdef __cplusplus
}
#endif

#endif

