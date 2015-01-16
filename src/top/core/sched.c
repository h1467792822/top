
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <assert.h>
#include <stdio.h>
#include <top/core/sched.h>
#include <top/core/pthread.h>
#include <top/core/atomic.h>

typedef struct top_join_cond_s {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} top_join_cond_t;

static inline void top_join_cond_init(top_join_cond_t* cond)
{
    pthread_mutex_init(&cond->mutex,0);
    pthread_cond_init(&cond->cond,0);
}

static inline void top_join_cond_fini(top_join_cond_t* cond)
{
    pthread_mutex_destroy(&cond->mutex);
    pthread_cond_destroy(&cond->cond);
}

#define TOP_JOIN_COND_WAIT( join_cond, express ) \
	do { \
		pthread_mutex_lock(&(join_cond)->mutex); \
		if ( express ) pthread_cond_wait(&(join_cond)->cond,&(join_cond)->mutex); \
		pthread_mutex_unlock(&(join_cond)->mutex); \
	}while(0)

#define TOP_JOIN_COND_SIGNAL( join_cond,express) \
	do { \
		pthread_mutex_lock(&(join_cond)->mutex); \
		express; \
		pthread_cond_signal(&(join_cond)->cond); \
		pthread_mutex_unlock(&(join_cond)->mutex); \
	}while(0)

#define TOP_SIG_ATTACH (SIGRTMIN) //将task挂接到running队列中去
#define TOP_SIG_RESUME (SIGRTMIN + 1) //将task挂接到running队列中去
#define TOP_SIG_EXIT SIGUSR1 //通知退出
#define TOP_SIG_SIGNAL SIGUSR2

static __thread struct top_sched_s* g_current_sched = 0;

top_sched_t* top_current_sched()
{
    return g_current_sched;
}

top_task_t* top_current_task()
{
    top_sched_t* sched = g_current_sched;
    return sched ? sched->current : 0;
}

void top_schedule(struct top_sched_s* sched) ;

static inline void top_sched_task_pending(top_sched_t* sched, top_task_t* task)
{
	if(0 == (task->priv_flags & TOP_TASK_PRIV_FL_PENDING)) {
		top_list_add_tail(&sched->running,&task->node);
		task->priv_flags |= TOP_TASK_PRIV_FL_PENDING;
	}
}

static inline void top_sched_task_signal(top_sched_t* sched,top_task_t* task)
{
	if(0 == (task->priv_flags & TOP_TASK_PRIV_FL_SIGNAL)) {
		top_list_add_tail(&sched->sig_running,&task->sig_node);
		task->priv_flags |= TOP_TASK_PRIV_FL_SIGNAL;
	}
}

static void top_sig_action(int signo,siginfo_t* info,void* ucontext)
{
    struct top_task_s* task = info->si_value.sival_ptr;
    //task->flags |= TOP_TASK_FL_PENDING;
    assert(task->sched == g_current_sched);
    switch(task->state) {
    case TOP_TASK_ST_LOCK_WAIT:
    case TOP_TASK_ST_COND_WAIT:
    case TOP_TASK_ST_SLEEP:
        top_list_node_insert(&task->node,task->sched->imm_pos->next);
        task->sched->imm_pos = &task->node;
        break;
    default:
        top_list_add_tail(&task->sched->running,&task->node);
        break;
    }
	int val = 0;
	(int)sem_getvalue((sem_t*)task->sched->sem,&val);
	if(val == 0) sem_post((sem_t*)task->sched->sem);
}

static void* top_idle_main_loop(struct top_task_s* task, void* data)
{
    top_sched_t * sched = task->sched;
    struct timeval tm = { 0, 1000 };
    while(sched->terminated == 0) {
		sem_wait((sem_t*)sched->sem);
        TOP_TASK_SUSPEND(task);
    }
    ++sched->terminated;
    top_longjmp(sched->main_context,1);
    return 0;
}

static void top_sig_ignore(int signo)
{
    (void)signo;
}

void top_sched_main_loop(top_sched_t* sched)
{
	sem_t sem;
	sem_init(&sem,0,0);
	sem_t* psem = sched->sem;
	sched->sem = &sem;
    g_current_sched = sched;
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    if(SIG_ERR == signal(TOP_SIG_EXIT,top_sig_ignore)) {
        goto fail;
    }
    if(SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
        goto fail;
    }
    sa.sa_sigaction = top_sig_action;
    sa.sa_flags = SA_SIGINFO;
    if(0 != sigaction(TOP_SIG_ATTACH,&sa,0)) {
        goto fail;
    }
    if(0 != sigaction(TOP_SIG_RESUME,&sa,0)) {
        goto fail;
    }
    sem_post(psem);
    if(0 == top_setjmp(sched->main_context)) {
        top_schedule(sched);
    }

    if(!top_bool_cas(&sched->join_task,0,&sched->idle)) {
        //active join_task
    }
out:
    return ;
fail:
    printf("\nfailed to start thread: %d\n",errno);
    sem_post(psem);
    sched->retval = 1;
    goto out;
}
static void* top_sched_main(void* data)
{
    struct top_sched_s* sched = (struct top_sched_s*)data;
    top_sched_main_loop(sched);
    return 0;
}

top_error_t top_sched_init(struct top_sched_s* sched, const struct top_pthread_conf_s* conf)
{
    memset(sched,0,sizeof(*sched));
    sched->conf = (top_pthread_conf_t*)(conf ? conf : g_top_pthread_conf_linux);
    top_list_init(&sched->sig_running);
    top_list_init(&sched->running);
    sched->imm_pos = sched->running.first;
    top_list_init(&sched->waiting);
    top_list_init(&sched->sleeping);
    top_task_init(&sched->idle,0,top_idle_main_loop,sched);
    top_list_init((struct top_list*)&sched->idle.node);
    sched->idle.sched = sched;
    sched->retval = 0;
#if 1
    sem_t sem;
    (void)sem_init(&sem,0,0);
    sched->sem = &sem;
    top_pthread_param_t tp;
    memset(&tp,0,sizeof(tp));
    tp.main = top_sched_main;
    tp.main_data = sched;
    top_error_t retno =  top_pthread_create(sched->conf,&sched->tid,&tp);
    if(top_errno(retno) == 0) {
        sem_wait(&sem);
    }
    sem_destroy(&sem);
    return retno;
#endif
}

void top_sched_terminate(struct top_sched_s* sched)
{
    if(sched->terminated == 0) {
        sched->terminated = 1;
        (void)sem_post((sem_t*)sched->sem);
    }
}

void top_sched_fini(struct top_sched_s* sched)
{
}

static inline void top_sched_cleanup(struct top_sched_s* sched)
{
}

/**
 *
 */
top_error_t top_sched_join(struct top_sched_s* sched)
{
    struct top_sched_s* current = g_current_sched;
    if(current == 0) {
        top_pthread_join(sched->conf,sched->tid);
        top_sched_cleanup(sched);
        return TOP_OK;
    } else if(current != sched) {
        if(top_bool_cas(&sched->join_task,0, current->current)) {
            TOP_TASK_SUSPEND(current->current);
        }
        top_sched_cleanup(sched);
        return TOP_OK;
    }
    return TOP_ERROR(EPERM);
}

static void top_task_main(struct top_task_s* task)
{
    top_sched_t * sched = task->sched;
    task->state = TOP_TASK_ST_RUNNING;
    sched->current = task;
    task->retval = task->main(task,task->main_data);
	unsigned int flag = top_fetch_and_or(&task->flags,TOP_TASK_FL_EXIT);
	if(flag & TOP_TASK_FL_SIG) {
		printf("\n waiting signal \n");
		TOP_TASK_SUSPEND(task);
	}

    //if(task->exit_handler) task->exit_handler(task,TOP_TASK_SIG_EXIT,0);
    if(!top_bool_cas(&task->join_cond,0,(top_join_cond_t*)1)) {
        printf("\nnotify task join: %p\n",task);
        TOP_JOIN_COND_SIGNAL(task->join_cond, task->state = TOP_TASK_ST_EXIT);
    } else {
        printf("\ntask exit: %p\n",task);
        task->state = TOP_TASK_ST_EXIT;
        if(task->join_task) {
            printf("\ntask resume: %p\n",task->join_task);
            (void)top_task_resume(task->join_task);
        }
    }
    TOP_RESCHEDULE(sched);
	__builtin_unreachable();
}

static inline void top_task_restore_context(struct top_task_s* task)
{
    task->state = TOP_TASK_ST_RUNNING;
    task->sched->current = task;
    top_longjmp(task->context,1);
}

static inline void top_task_process_rt_signal(struct top_task_s* task, int signo, top_task_rt_sig_t* rt_sig)
{
	int count = rt_sig->count;
	(void)top_fetch_and_sub(&rt_sig->count,count);
	unsigned int idx;
	void* data;
	for(; count; --count, ++rt_sig->low_idx) {
		idx = rt_sig->low_idx & rt_sig->idx_mask;
		data = rt_sig->datas[idx];
		if(data != rt_sig) {
			rt_sig->handler(task,signo,rt_sig->datas[idx]);
			rt_sig->datas[idx] = rt_sig;
		}else {
			//外部的数据还没有写入，需要等待下一次机会
			(void)top_fetch_and_add(&rt_sig->count,count);
			(void)top_fetch_and_or(&task->rt_sigmask, 1u << idx);
		}
	}
}

static inline void top_task_dispatch_rt_signal(struct top_task_s* task)
{
	unsigned int sigmask = top_fetch_and_and(&task->rt_sigmask,0) & task->sig_handler_mask;
	if(!sigmask) return;
	int sig_count = __builtin_popcount(sigmask);	
	int offset;
	for(; sig_count; --sig_count) {
		offset = __builtin_ctz(sig_count);
		sig_count &= ~(1ul << offset);
		top_task_process_rt_signo(task,offset + 32,&task->rt_sigs[offset]);
	}
}

static inline void top_task_dispatch_signal(struct top_task_s* task)
{
	unsigned int sigmask = top_fetch_and_and(&task->sigmask,0) & task->sig_handler_mask;
	if(!sigmask) return;
	int sig_count = __builtin_popcount(sigmask);	
	int offset;
	for(; sig_count; --sig_count) {
		offset = __builtin_ctz(sig_count);
		sig_count &= ~(1ul << offset);
		task->sigs[offset].handler(task,offset,0);
	}
}

static void top_schedule_pending(struct top_sched_s* sched)
{
	if(!top_list_empty(&sched->sig_running)) {
		top_task_t* task;
        task = top_list_entry(top_list_remove_first(&sched->sig_running),top_task_t,node);
		task->priv_flags &= ~TOP_TASK_PRIV_FL_SIGNAL;
		top_task_dispatch_signal(task);
		top_task_dispatch_rt_signal(task);
	}
}

static void top_schedule_pending(struct top_sched_s* sched)
{
    top_task_t * task;
	int count = 0;
    if(!top_list_empty(&sched->running)) {
        task = top_list_entry(top_list_remove_first(&sched->running),top_task_t,node);
        if(sched->imm_pos == &task->node) sched->imm_pos = (struct top_list_node*)&sched->running;
		task->priv_flags &= ~TOP_TASK_PRIV_FL_PENDING;
        printf("\ntop_schedule: %p\n",task);
    } else {
        printf("\ntop_schedule: idle_task\n");
        task = &sched->idle;
    }
    sched->current = task;
    switch(task->state) {
    case TOP_TASK_ST_INIT:
        printf("\n init :%p\n",task);
        top_task_main(task);
        break;
    default:
        printf("\n long jmp: %p \n",task);
        top_task_restore_context(task);
        break;
    }

	printf("\n **** shouldn't reach here! *** \n");
	__builtin_unreachable();
}

void top_schedule(struct top_sched_s* sched)
{
	top_schedule_signal(sched);
	top_schedule_pending(sched);
}

void top_task_init(struct top_task_s* task,top_task_conf_t* conf,top_task_main_loop main,void* main_data)
{
    memset(task,0,sizeof(*task));
    task->main = main;
    task->main_data = main_data;
}

top_error_t top_task_active(struct top_task_s* task,struct top_sched_s* sched)
{
    if(task->state == TOP_TASK_ST_INIT) {
        task->sched = sched;
        if(sched == g_current_sched) {
			top_sched_task_pending(sched,task);
            return TOP_OK;
        } else {
            return top_pthread_rt_signal(sched->conf,sched->tid,TOP_SIG_ATTACH,task);
        }
    }
    return TOP_ERROR(-1);
}

top_error_t top_task_restart(struct top_task_s* task,struct top_sched_s* sched)
{
    task->state = TOP_TASK_ST_INIT;
    if(task->sched == sched) {
		top_sched_task_pending(sched,task);
        return TOP_OK;
    } else {
        task->sched = sched;
        return top_pthread_rt_signal(sched->conf,sched->tid,TOP_SIG_ATTACH,task);
    }
}

void top_task_suspend(struct top_task_s* task)
{
    assert(task->sched == g_current_sched);
    task->state = TOP_TASK_ST_SUSPEND;
	//unsigned int flags = top_fetch_and_and(task->flags,~TOP_TASK_FL_RESUME);
	//if(!(flags & TOP_TASK_FL_RESUME)) {
		TOP_TASK_SUSPEND(task);
	//}
}

top_error_t top_task_resume(struct top_task_s* task)
{
    if(task->sched == g_current_sched) {
        assert(task->sched);
		top_sched_task_pending(task->sched,task);
        return TOP_OK;
    } else {
        return top_pthread_rt_signal(task->sched->conf,task->sched->tid,TOP_SIG_RESUME,task);
    }
}

void top_task_yield(struct top_task_s* task)
{
    assert(task->sched == g_current_sched);
	top_sched_task_pending(task->sched,task);
    task->state = TOP_TASK_ST_PENDING;
    TOP_TASK_SUSPEND(task);
}

top_error_t top_task_join(struct top_task_s* task)
{
    top_task_t* current = top_current_task();
    if(current == 0) {
        top_join_cond_t join;
        top_join_cond_init(&join);
        if(top_bool_cas(&task->join_cond,0,&join)) {
            TOP_JOIN_COND_WAIT( &join, task->state != TOP_TASK_ST_EXIT );
        }
        top_join_cond_fini(&join);
        return TOP_OK;
    } else if(task != current) {
        task->join_task = current;
        TOP_TASK_SUSPEND(current);
        return TOP_OK;
    }
    return TOP_ERROR(EPERM);
}

top_error_t top_task_sigaction(top_task_t* task,int sig,top_task_sig_handler handler)
{
	if(sig < TOP_TASK_SIG_MIN || sig > TOP_TASK_SIG_MAX) return TOP_ERROR(EINVAL);
	if(handler) {
		task->sig_handler_mask |= 1u << sig;
		task->sigs[sig].handler = handler;
	}else {
		task->sig_handler_mask &= ~(1ul << sig);
		task->sigs[sig].handler = 0;
	}
	return TOP_OK;
}

top_error_t top_task_rt_sigaction(top_task_t* task,int sig,top_task_sig_handler handler)
{
	if(sig < TOP_TASK_RT_SIG_MIN || sig > TOP_TASK_RT_SIG_MAX) return TOP_ERROR(EINVAL);
	if(handler) {
		task->rt_sig_handler_mask |= 1u << sig;
		task->rt_sigs[sig].handler = handler;
	}else {
		task->rt_sig_handler_mask &= ~(1ul << sig);
		task->rt_sigs[sig].handler = 0;
	}
	return TOP_OK;
}

top_error_t top_task_signal(struct top_task_s* task, int sig_no)
{
	assert(sig_no >= 0 && sig_no < 32);
	top_sched_t* sched = top_current_sched();
	unsigned int sigmask = 1u << sig_no;
	unsigned int old_sigmask = top_fetch_and_or(&task->sigmask,sigmask);

	if(old_sigmask & TOP_TASK_PRIV_FL_EXIT){
		return TOP_ERROR(EEXIST);
	}
	
	if(old_sigmask) return TOP_OK;

	if(sched == task->sched) {
		top_sched_task_signal(sched,task);
		return TOP_OK;
	}else {
		return top_pthread_signal(sched->conf,sched->tid,TOP_SIG_SIGNAL);
	}
}

static inline top_error_t top_task_push_back_rt_sig(top_task_t* task, int sig, void* data)
{
#if 0
	assert(sig >= 32 && sig <= 63);
	sig -= 32;
	f(top_add_and_fetch(&task->rt_sigs[sig].count,1));
#endif
	return TOP_OK;
}

top_error_t top_task_rt_signal(struct top_task_s* task, int sig_no,void* data)
{
	assert(sig_no >= 0 && sig_no < 32);
	top_sched_t* sched = top_current_sched();
	unsigned int sigmask = 1u << sig_no;
	unsigned int old_sigmask = top_fetch_and_or(&task->rt_sigmask,sigmask);
	top_error_t err;

	if(old_sigmask & TOP_TASK_PRIV_FL_EXIT){
		return TOP_ERROR(EEXIST);
	}
	
	if(old_sigmask) return TOP_OK;

	/** 即便出错，也需要发送信号 */
	err = top_task_push_back_rt_sig(task,sig_no,data);	
	if(sched == task->sched) {
		top_sched_task_signal(sched,task);
	}else {
		top_error_t sig_err;
		sig_err = top_pthread_signal(sched->conf,sched->tid,TOP_SIG_SIGNAL);
		if(top_errno(sig_err)) return sig_err;
	}
	return err;
}

