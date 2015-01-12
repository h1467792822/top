
#include <top/core/sched.h>

#define TOP_SIG_AWAKE (SIGRTMIN) //只有一个信号，将task挂接到running队列中去

static __thread struct top_sched_s* g_current_sched = 0;

void top_schedule(struct top_sched_s* sch) ;

static void top_sig_action(int signo,siginfo_t* info,void* ucontext)
{
	struct top_task_s* task = info->si_value.sival_ptr;
	task->flag |= TOP_TASK_FL_PENDING;
	assert(task->sched == g_current_sched);
	switch(task->state){
		case TOP_TASK_ST_LOCK_WAIT:
		case TOP_TASK_ST_COND_WAIT:
		case TOP_TASK_ST_SLEEP:
	top_list_add_tail(&task->sched->imm_running,&task->to_sch);
	break;
		default:
	top_list_add_tail(&task->sched->running,&task->to_sch);
	break;
	}
	++task->sched->running_count;
}

static void* top_idle_main(struct top_task_s* task, void* data) {
	struct top_sched_s* sch = (struct top_sched_s*)data;
	return sch;
}

static void* top_sched_main(void* data) 
{
	struct top_sched_s* sch = (struct top_sched_s*)data;
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_action = top_sig_action;
	sa.sa_flags = SA_SIGINFO;
	if(0 == sigaction(TOP_SIG_AWAKE,&sa,0)) {
		top_schedule(sch);
	}else {
		sch->retval = 1;
	}
	return 0;
}

top_error_t top_sched_init(struct top_sched_s* sch,const struct top_pthread_conf_s* conf)
{
	sch->conf = conf;
	top_list_init(&sch->imm_running);
	top_list_init(&sch->running);
	top_list_init(&sch->waiting);
	top_list_init(&sch->sleeping);
	top_task_init(&sch->idle,top_idle_main,sch);
	sch->idle.next = sch->idle.prev = &sch->idle;
	sch->retval = 0;
	return conf->create(&sch->tid,conf->user_data,top_sched_main,sch,0,0);
}

/**
 * only invoked by sched-manager
 */
void top_sched_join(struct top_sched_s* sch,void* data)
{
	struct top_sched_s* current = g_current_sched;
	if(current == 0) {
		sch->conf->join(sch->tid);
		return;
	}else if(current != sch) {
		TOP_TASK_SWITCH_CONTEXT(current->current);
	}
}

static inline void top_task_main(struct top_task_s* task) 
{
	task->sch->current = task;
	void* retval = task->main(task,task->main_data);
}

static inline void top_task_retore_context(struct top_task_s* task) {
	task->sch->current = task;
	top_long_jmp(&task->context,1);
}

void top_schedule(struct top_sched_s* sch) 
{
	top_task_t * task;
	if(!top_list_empty(&sch->imm_running)) {
		task = top_list_entry(sch->imm_running.first,top_task_t,to_sch);
		top_list_node_del(&task->to_sch);
	}else if(!top_list_empty(&sch->running)) {
		task = top_list_entry(sch->running.first,top_task_t,to_sch);
		top_list_node_del(&task->to_sch);
		task = &sch->idle;
	}else {
		task = &sch->idle;
	}
	sch->current = task;
	switch(task->state) {
	case TOP_TASK_ST_INIT:
		top_task_main(task);
		break;
	default:
		top_task_restore_context(task);
		break;
	}
}

void top_task_init(struct top_task_s* task,top_task_main main,void* main_data) {
	task->flag = 0;
	task->state = 0;
	task->main = main;
	task->main_data = main_data;
}

top_error_t top_task_attach(struct top_task_s* task,struct top_sched_s* sch) {
	if(task->state == TOP_TASK_ST_INIT) {
		task->state = TOP_TASK_ST_PENDING;
	if(sch == g_current_sched) {
		top_list_add_tail(&sch->running,&task->to_sch);
		++sch->running_count;
		return TOP_OK;
	}else {
		return top_pthread_rt_signal(sch->conf,sch->tid,TOP_SIG_AWAKE,task);
	}
	}
	return TOP_ERROR(-1);
}

void top_task_yield(struct top_task_s* task) {
	assert(task->sched == g_current_sched);
	top_list_add_tail(&task->sched->running,&task->to_sch);
	TOP_TASK_SWITCH_CONTEXT(task);
}

void top_task_join(struct top_task_s* task)
{
	struct top_sched_s* current = g_current_sched;
	if(current == 0) {
		sch->conf->join(sch->tid);
		return;
	}else if(current != task->sched) {
		TOP_TASK_SWITCH_CONTEXT(task);
	}
}

