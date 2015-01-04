
#ifndef TOP_CORE_TASK_H
#define TOP_CORE_TASK_H

#include <setjmp.h>

/**
  * Cache调用的使用方式:
  * Task的处理函数中:
  * top_chain_s chain; //form stack ,not heap
  * ...
  * // 如果cache模块是独立的task，那么其接口应该是纯粹的信号发送接口
  * // 如果是一般的函数接口应该理解为不存在task切换
  * top_cache_insert(chain); //写入chain，chain写入之后才能返回，但要求不能堵塞当前业务
  * top_task_event_t sig_evt = { insert, chain, 0 };
  * top_task_sig_action_return(cache_task,sig_evt);
  * top_chain_dtor(chain); // 释放chain的资源，chain写入返回后会自动激活当前task，从这里开始执行
  * ...
*/
#ifdef __cplusplus
extern "C" {
#endif

/**
  *EVTSTD 机制类似posix的standard signals，特点是同一个信号发送的时候如果上一次发送的信号还没有被处理，则会覆盖上一次信号信息，只处理一次
  *EVT RT 机制类似posix的realtime signals, 特点是多次发送的信号不会被覆盖，而是根据发送顺序依次被处理, pending状态的信号有最大数量限制，这是每个task根据自己的处理能力限制的
  *       这个实际是一种流控反压机制,类似socket接收缓冲区，完全避免task内部未处理的某个信号的数量超过最大预期
*/
#define TOP_TASK_EVT_STD_MIN 0 
#define TOP_TASK_EVT_STD_MAX 31 
#define TOP_TASK_EVT_RT_MIN 32 
#define TOP_TASK_EVT_RT_MAX 63

typedef void (*pf_top_std_evt_handler)(struct top_task_s* task,int evtno,unsigned int dup_times);
typedef void (*pf_top_rt_evt_handler)(struct top_task_s* task,int evtno,void* data[],unsigned int len);

/**
  * @handler 发送信号者指定信号处理逻辑，这个逻辑在task的上下文中执行，相当于注入了信号的处理函数，如果handler为NULL，则使用内部注册的handler处理。
*/
struct top_task_event_s {
	int evtno;
	void* data;
	union {
		pf_top_std_evt_handler std_handler;
		pf_top_rt_evt_handler rt_handler;
	};
	struct top_task_s* 
};

struct top_task_s;
struct top_task_operation_s;
struct top_task_operation_s {
	top_error_t (*signal)(struct top_task_s* task,struct top_task_event_s* evt);
	top_error_t (*sigaction)(struct top_task_s* task,struct top_task_event_s* evt);
	top_error_t (*sigaction_return)(struct top_task_s* task,struct top_task_event_s* evt);
	top_error_t (*set_max_pending)(struct top_task_s* task,int evtno,int max,int* pold_max);
	int (*get_max_pending)(struct top_task_s* task, int evtno);

	top_error_t (*register_std_evt_handler)(struct top_task_s* task,int evtno,pf_top_std_evt_handler handler);
	top_error_t (*register_rt_evt_handler)(struct top_task_s* task,int evtno,pf_top_rt_evt_handler handler);

	top_error_t (*)(struct top_task_s* task);
	top_error_t (*suspend)(struct top_task_s* task);

};

typedef struct top_task_s {
	jmp_buf context;	
	void* events[TOP_TASK_EVT_MAX + 1];
	top_scheduler_t * sch;
	top_list_node to_sch;
	to_list friends;
} top_task_t;


#ifdef __cplusplus
}
#endif

#endif

