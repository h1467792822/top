
#ifndef TOP_CORE_SCHED_H
#define TOP_CORE_SCHED_H

#include <inttypes.h>

#ifndef TOP_CORE_SETJMP_H
#include <top/core/setjmp.h>
#endif

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

struct top_task_s;
struct top_sched_s;

typedef void* (*top_task_main_loop)(struct top_task_s* task,void* main_data);

typedef enum top_task_state_enum {
	TOP_TASK_ST_INIT = 0,
	TOP_TASK_ST_RUNNING,
	TOP_TASK_ST_SIGNAL,
	TOP_TASK_ST_WAIT_EXIT,
	TOP_TASK_ST_EXIT,
}top_task_state_e;

typedef enum top_task_flag_enum {
	TOP_TASK_FL_LOCK_WAIT = 1ul << 1,
	TOP_TASK_FL_COND_WAIT = 1ul << 2,
	TOP_TASK_FL_SEM_WAIT = 1ul << 3,
	TOP_TASK_FL_SLEEP = 1ul << 4,
	TOP_TASK_FL_ASYN = 1ul << 5,
	TOP_TASK_FL_EXIT = 1ul << 15,
}top_task_flag_e;

#define TOP_TASK_SIG_EXIT 31  //task 退出是最后发送给自己的信号，用户可以注册在这个信号处理中完成资源清理动作, 保证只会被调用一次
#define TOP_TASK_SIG_TERM 30  //sched 调用terminate的时候发送给所有的task的信号，task收到这个信号后应该尽快完成退出动作, 保证只会被调用一次
#define TOP_TASK_SIG_MIN 0
#define TOP_TASK_SIG_MAX 15 
#define TOP_TASK_RT_SIG_MIN 32
#define TOP_TASK_RT_SIG_MAX 47 

#define TOP_TASK_PRIV_FL_PENDING 1u

#define TOP_TASK_RT_SIG_EXIT (1u << 31)

typedef void (*top_task_sig_handler)(struct top_task_s* task,void* task_data,int sig,void* data);

typedef struct top_task_rt_sig_s {
	top_task_sig_handler handler;
	void** datas;
	unsigned int max_count;
	unsigned int count;
	unsigned int exit_count;
	uint64_t pop_idx;
	uint64_t push_idx;
}top_task_rt_sig_t;

typedef struct top_task_sig_s {
	top_task_sig_handler handler;
}top_task_sig_t;

// sizeof(empty struct) == sizeof(size_t) in c++
// sizeof(empty struct) == 0 in c
typedef struct top_task_stat_s {
	unsigned long sched_times;
} top_task_stat_t;

typedef struct top_task_conf_s {
}top_task_conf_t;

typedef struct top_task_s {
	unsigned int state;
	unsigned int priv_flags;
	struct top_sched_s* sched;
	struct top_list_node node;
	struct top_stack_node asyn_node;
	struct top_list_node to_sched;
	top_task_stat_t stat;
	top_jmp_buf main_context;
	top_jmp_buf sig_context;
	volatile unsigned int flags;
	volatile uint64_t sigmask;
	uint64_t exit_sigmask;
	uint64_t sig_handler_mask;
	top_task_sig_t sigs[32];
	top_task_rt_sig_t rt_sigs[32];
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
top_error_t top_task_signal(struct top_task_s* task, int sig);
top_error_t top_task_rt_signal(struct top_task_s* task, int sig,void* data);
top_error_t top_task_rt_signal_wait(struct top_task_s* task, int sig,void* data);


/**
  * only called in task
  */
top_error_t top_task_restart(struct top_task_s* task,struct top_sched_s* sch);
top_error_t top_task_sigaction(struct top_task_s* task, int sig,top_task_sig_handler handler);
top_error_t top_task_set_rt_signal_buf(struct top_task_s* task,int sig,void** datas,int count);
top_error_t top_task_rt_sigaction(struct top_task_s* task, int sig,top_task_sig_handler handler);
void top_task_suspend(struct top_task_s* task);
void top_task_yield(struct top_task_s* task);
void top_task_sleep(struct top_task_s* task,unsigned long secs);
void top_task_usleep(struct top_task_s* task,unsigned long usecs);

typedef struct top_sched_stat_s {
	unsigned long sched_times;
}top_sched_stat_t;

#define TOP_SCHED_FL_ASYN (1u << 1)
#define TOP_SCHED_FL_EXIT (1u << 2)

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
	top_jmp_buf main_context;
	top_jmp_buf sched_context;
}top_sched_t;

struct top_sched_s* top_current_sched();
struct top_task_s* top_current_task();

top_error_t top_sched_init(struct top_sched_s* sched,const struct top_pthread_conf_s* conf);
void top_sched_terminate(struct top_sched_s* sched);
top_error_t top_sched_join(struct top_sched_s* sched);
void top_sched_fini(struct top_sched_s* sched);
void top_sched_lock(struct top_sched_s* sched);
void top_sched_unlock(struct top_sched_s* sched);

#define TOP_RESCHEDULE(sched) \
	top_schedule(sched)

#define TOP_TASK_SUSPEND(task) \
	do { \
		if((task)->state == TOP_TASK_ST_SIGNAL){ \
	if(0 == top_setjmp((task)->sig_context)) { \
		printf("\n suspend in signal: %p\n",(task)); \
		TOP_RESCHEDULE((task)->sched); \
	}\
		}else {\
	if(0 == top_setjmp((task)->main_context)) { \
		printf("\n suspend in running: %p\n",(task)); \
		TOP_RESCHEDULE((task)->sched); \
	}\
		}\
	}while(0)

#define TOP_TASK_RESUME(task) \
		top_task_resume(task)
	

#ifdef __cplusplus
}
#endif

#endif

