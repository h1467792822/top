
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
        printf("\n add pending: %p\n", task);
        top_list_add_tail(&sched->running,&task->node);
        task->priv_flags |= TOP_TASK_PRIV_FL_PENDING;
    }
}

static inline void top_sched_task_signal(top_sched_t* sched,top_task_t* task)
{
    top_sched_task_pending(sched,task);
}

#if 0
static void top_sig_action(int signo,siginfo_t* info,void* ucontext)
{
    struct top_task_s* task = info->si_value.sival_ptr;
    top_sched_t * sched = g_current_sched;
    if(task->sched == sched) {
        top_sched_task_pending(sched,task);
    } else {
        //
    }
    int val = 0;
    (int)sem_getvalue((sem_t*)task->sched->sem,&val);
    if(val == 0) sem_post((sem_t*)task->sched->sem);
}
#endif

static void* top_idle_main_loop(struct top_task_s* task, void* data)
{
    top_sched_t * sched = task->sched;
    struct timeval tm = { 0, 1000 };
    while(sched->task_count > 1 || 0 == (sched->flags & TOP_SCHED_FL_EXIT)) {
        sem_wait((sem_t*)sched->sem);
        (void)top_fetch_and_and(&sched->flags ,~TOP_SCHED_FL_ASYN);
        TOP_TASK_SUSPEND(task);
    }
    top_longjmp(sched->main_context,1);
    return 0;
}

#if 0
static void top_sig_ignore(int signo)
{
    (void)signo;
}
#endif

static void top_sched_main_loop(top_sched_t* sched)
{
    sem_t sem;
    (void)sem_init(&sem,0,0);
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    sched->mutex = &mutex;
    sem_t* psem = sched->sem;
    sched->sem = &sem;
    g_current_sched = sched;
#if 0
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
#endif
    sem_post(psem);
    if(0 == top_setjmp(sched->main_context)) {
        top_schedule(sched);
    }

    if(!top_bool_cas(&sched->join_task,0,&sched->idle)) {
        top_task_resume(sched->join_task);
    }
out:
    (void)pthread_mutex_destroy(&mutex);
    (void)sem_destroy(&sem);
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
    top_list_init(&sched->running);
    sched->imm_pos = sched->running.first;
    top_task_init(&sched->idle,0,top_idle_main_loop,sched);
    top_list_init((struct top_list*)&sched->idle.node);
    sched->idle.sched = sched;
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

static inline top_error_t top_sched_signal(struct top_sched_s* sched);

void top_sched_terminate(struct top_sched_s* sched)
{
    unsigned int flags = top_fetch_and_or(&sched->flags,TOP_SCHED_FL_EXIT);
    if(0 == (flags & TOP_SCHED_FL_EXIT)) {
        top_sched_signal(sched);
    }
}

void top_sched_fini(struct top_sched_s* sched)
{
}

static inline void top_sched_cleanup(struct top_sched_s* sched)
{
}

void top_sched_lock(struct top_sched_s* sched)
{
    (void)pthread_mutex_lock(sched->mutex);
}

void top_sched_unlock(struct top_sched_s* sched)
{
    (void)pthread_mutex_unlock(sched->mutex);
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
            top_sched_cleanup(sched);
        }
        return TOP_OK;
    }
    return TOP_ERROR(EDEADLK);
}

static inline void top_task_mark_rt_signal_exit(struct top_task_s* task)
{
    top_task_rt_sig_t* rt_sig;
    int i;
    for(i = 32; i < 64; ++i) {
        rt_sig = &task->rt_sigs[i - 32];
        rt_sig->exit_count = top_fetch_and_or(&rt_sig->count,TOP_TASK_RT_SIG_EXIT);
    }
}

static void top_task_main(struct top_task_s* task)
{
    top_sched_t * sched = task->sched;
    task->state = TOP_TASK_ST_RUNNING;
    ++sched->task_count;
    task->retval = task->main(task,task->main_data);
    top_task_mark_rt_signal_exit(task);
    task->exit_sigmask = top_fetch_and_or(&task->sigmask,TOP_TASK_SIG_EXIT);
    printf("\n %p check sigmask: %dll\n",task,task->exit_sigmask);
    if(task->exit_sigmask) {
        task->state = TOP_TASK_ST_WAIT_EXIT;
        printf("\n suspend before exit: %dll",task->exit_sigmask);
        TOP_TASK_SUSPEND(task);// task_resume错误的调用可能导致问题
    }
    task->state = TOP_TASK_ST_EXIT;
    printf("\n %p check asyn running: \n",task);
    unsigned int flags = top_fetch_and_or(&task->flags,TOP_TASK_FL_EXIT);
    if((flags & TOP_TASK_FL_ASYN) || (task->priv_flags & TOP_TASK_PRIV_FL_PENDING) ) {
        TOP_TASK_SUSPEND(task);
    }

    top_task_sig_handler handler = task->sigs[TOP_TASK_SIG_EXIT].handler;
    if(handler) handler(task,task->main_data,TOP_TASK_SIG_EXIT,0);
    if(!top_bool_cas(&task->join_cond,0,(top_join_cond_t*)1)) {
        TOP_JOIN_COND_SIGNAL(task->join_cond, task->state = TOP_TASK_ST_EXIT);
    } else {
        if(task->join_task) {
            (void)top_task_resume(task->join_task);
        }
    }
    --sched->task_count;
    TOP_RESCHEDULE(sched);
    __builtin_unreachable();
}

static inline void top_task_process_rt_signal(struct top_task_s* task, int signo, top_task_rt_sig_t* rt_sig)
{
    void* data;
    unsigned int idx;
    unsigned int count = rt_sig->count; //snapshot
    if(count & TOP_TASK_RT_SIG_EXIT) {
        count = rt_sig->exit_count;
    }
    for(; count; --count, top_atomic_inc(&rt_sig->pop_idx),top_atomic_dec(&rt_sig->count)) {
        idx = rt_sig->pop_idx % rt_sig->max_count;
retry:
        data = rt_sig->datas[idx];
        if(data != rt_sig) {
            if(rt_sig->handler) rt_sig->handler(task,task->main_data,signo,rt_sig->datas[idx]);
            rt_sig->datas[idx] = rt_sig;
        } else {
            top_task_yield(task);
            goto retry;
        }
    }
}

static inline void top_task_process_rt_sigmask(struct top_task_s* task,unsigned int sigmask)
{
    int sig_count = __builtin_popcount(sigmask);
    int offset;
    for(; sig_count; --sig_count) {
        offset = __builtin_ctz(sigmask);
        sigmask &= ~(1u << offset);
        top_task_process_rt_signal(task,offset + 32,&task->rt_sigs[offset]);
    }
}

static inline void top_task_process_sigmask(struct top_task_s* task,unsigned int sigmask)
{
    top_task_sig_handler handler;
    int sig_count = __builtin_popcount(sigmask);
    int offset;
    for(; sig_count; --sig_count) {
        offset = __builtin_ctz(sigmask);
        sigmask &= ~(1u << offset);
        handler = task->sigs[offset].handler;
        if(handler) handler(task,task->main_data,offset,0);
    }
}

static inline void top_task_dispatch_signal(struct top_task_s* task,uint64_t sigmask,int state)
{
    if(!sigmask) return;
    printf("\n sigmask: %xull \n",sigmask);
    task->state = TOP_TASK_ST_SIGNAL;
    top_task_process_sigmask(task,sigmask);
    top_task_process_rt_sigmask(task,sigmask >> 32);
    task->state = state;
}

static inline void top_sched_merge_asyn_running(struct top_sched_s* sched)
{
    struct top_stack_node* top = *(struct top_stack_node* volatile*)&sched->asyn_running.top;
    if(top) {
        struct top_list_node* pos = (struct top_list_node*)&sched->running;
        struct top_list_node* imm_pos = (struct top_list_node*)&sched->running;
        top_task_t * task;
        pthread_mutex_lock(sched->mutex);
        top = sched->asyn_running.top;
        sched->asyn_running.top = 0;
        pthread_mutex_unlock(sched->mutex);
        do {
            task = top_stack_entry(top,top_task_t,asyn_node);
            if(0 == (task->priv_flags & TOP_TASK_PRIV_FL_PENDING)) {
                printf("\n insert an async task: %p\n", task);
                task->priv_flags |= TOP_TASK_PRIV_FL_PENDING;
                switch(task->state) {
                case TOP_TASK_ST_WAIT_EXIT:
                case TOP_TASK_ST_EXIT:
                    top_list_node_insert(&task->node,imm_pos->next);
                    imm_pos = &task->node;
                    break;
                default:
                    top_list_node_insert(&task->node,pos);
                    pos = pos->prev;
                    break;
                }
            } else {
                printf("\n an async task already inserted: %p\n", task);
            }
            top = top->next;
        } while(top);
    }
}

void top_schedule(struct top_sched_s* sched)
{
    uint64_t sigmask;
    top_task_t * task;
retry:
    top_sched_merge_asyn_running(sched);
    if(!top_list_empty(&sched->running)) {
        task = top_list_entry(top_list_remove_first(&sched->running),top_task_t,node);
        if(sched->imm_pos == &task->node) sched->imm_pos = (struct top_list_node*)&sched->running;
        task->priv_flags &= ~TOP_TASK_PRIV_FL_PENDING;
        task->flags &= ~TOP_TASK_FL_ASYN ;
        printf("\ntop_schedule: %p\n",task);
    } else {
        printf("\ntop_schedule: idle_task\n");
        task = &sched->idle;
    }
    sched->current = task;
    switch(task->state) {
    case TOP_TASK_ST_INIT:
        printf("\n init :%p\n",task);
        if(task->sigmask) {
            top_list_add(&sched->running,&task->node);
        }
        top_task_main(task);
        break;
    case TOP_TASK_ST_RUNNING:
        sigmask = top_fetch_and_and(&task->sigmask,0);
        printf("\n top_schedule running,sigmask: %xull \n",sigmask);
        top_task_dispatch_signal(task,sigmask,TOP_TASK_ST_RUNNING);
        if(task->flags == 0 || task->flags == TOP_TASK_FL_EXIT) {
            top_longjmp(task->main_context,1);
        } else {
            goto retry;
        }
        break;
    case TOP_TASK_ST_SIGNAL:
        printf("\n ST_SIGNAL\n");
        top_longjmp(task->sig_context,1);
        break;
    case TOP_TASK_ST_WAIT_EXIT:
        printf("\n ST_WAIT_EXIT: %p\n",task);
        top_task_dispatch_signal(task,task->exit_sigmask,TOP_TASK_ST_EXIT);
        top_longjmp(task->main_context,1);
        break;
    case TOP_TASK_ST_EXIT:
        printf("\n ST_EXIT: %p\n",task);
        task->priv_flags |= TOP_TASK_PRIV_FL_PENDING;
        top_longjmp(task->main_context,1);
        break;
    default:
        assert(0);
        break;
    }

    printf("\n **** shouldn't reach here! *** \n");
    __builtin_unreachable();
}

static inline top_error_t top_sched_signal(struct top_sched_s* sched)
{
    unsigned int flags = top_fetch_and_or(&sched->flags,TOP_SCHED_FL_ASYN);
    printf("\n sched signal: %d \n",flags);
    if(0 == (TOP_SCHED_FL_ASYN & flags)) {
        printf("\n sem post \n");
        sem_post(sched->sem);
    } else if(TOP_SCHED_FL_EXIT & flags) {
        return TOP_ERROR(EINVAL);
    }
    return TOP_OK;
}

static inline top_error_t top_sched_asyn_task(struct top_sched_s* sched,struct top_task_s* task)
{
    unsigned int flags = top_fetch_and_or(&task->flags,TOP_TASK_FL_ASYN);
    if(flags & TOP_TASK_FL_EXIT) return TOP_ERROR(EINVAL);
    if(0 == (flags & TOP_TASK_FL_ASYN)) {
        pthread_mutex_lock(sched->mutex);
        top_stack_push(&sched->asyn_running,&task->asyn_node);
        pthread_mutex_unlock(sched->mutex);
        return top_sched_signal(sched);
    }
    return TOP_OK;
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
            return top_sched_asyn_task(sched,task);
        }
    }
    return TOP_ERROR(EINVAL);
}

top_error_t top_task_restart(struct top_task_s* task,struct top_sched_s* sched)
{
    assert(task == top_current_task());
    task->state = TOP_TASK_ST_INIT;
    if(task->sched == sched) {
        top_sched_task_pending(sched,task);
        return TOP_OK;
    } else {
        task->sched = sched;
        return top_sched_asyn_task(sched,task);
    }
}

void top_task_suspend(struct top_task_s* task)
{
    assert(task == top_current_task());
    TOP_TASK_SUSPEND(task);
}

top_error_t top_task_resume(struct top_task_s* task)
{
    if(task->sched == g_current_sched) {
        printf("\n resume in same sched: %p\n",task);
        assert(task->sched);
        top_sched_task_pending(task->sched,task);
        return TOP_OK;
    } else {
        printf("\n resume in different sched: %p\n",task);
        return top_sched_asyn_task(task->sched,task);
    }
}

void top_task_yield(struct top_task_s* task)
{
    assert(task == top_current_task());
    top_sched_task_pending(task->sched,task);
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
    if(sig < TOP_TASK_SIG_MIN || sig >= TOP_TASK_RT_SIG_MAX) return TOP_ERROR(EINVAL);
    if(handler) {
        task->sigs[sig].handler = handler;
        task->sig_handler_mask |= 1ull << sig;
    } else {
        task->sigs[sig].handler = 0;
        task->sig_handler_mask &= ~(1ull << sig);
    }
    return TOP_OK;
}

top_error_t top_task_rt_sigaction(top_task_t* task,int sig,top_task_sig_handler handler)
{
    if(sig < TOP_TASK_RT_SIG_MIN || sig > TOP_TASK_RT_SIG_MAX) return TOP_ERROR(EINVAL);
    if(handler) {
        task->rt_sigs[sig - 32].handler = handler;
        task->sig_handler_mask |= 1ull << sig;
    } else {
        task->rt_sigs[sig - 32].handler = 0;
        task->sig_handler_mask &= ~(1ull << sig);
    }
    return TOP_OK;
}

static top_error_t top_task_asyn_signal(struct top_sched_s* sched,struct top_task_s* task, int sig_no)
{
    uint64_t sigmask = 1ull << sig_no;
    //if(!(sigmask & task->sig_handler_mask)) return TOP_OK;

    printf("\n 1. asyn_signal: %d\n",sig_no);
    unsigned int old_sigmask = top_fetch_and_or(&task->sigmask,sigmask);
    if(old_sigmask & TOP_TASK_SIG_EXIT) {
        return TOP_ERROR(EINVAL);
    }

    printf("\n 2. asyn_signal: %d\n",sig_no);
    if(old_sigmask) return TOP_OK;

    printf("\n 3. asyn_signal: %d\n",sig_no);
    if(sched == task->sched) {
        top_sched_task_signal(sched,task);
        return TOP_OK;
    } else {
        return top_sched_asyn_task(task->sched,task);
    }
}

top_error_t top_task_signal(struct top_task_s* task, int sig_no)
{
    assert(sig_no >= 0 && sig_no < TOP_TASK_RT_SIG_MIN);
    top_sched_t* sched = top_current_sched();
    if(sched && sched->current == task) {
        printf("\n ++synch signal :%d\n",sig_no);
        top_task_sig_handler handler = task->sigs[sig_no].handler;
        if(handler) handler(task,task->main_data,sig_no,0);
        return TOP_OK;
    } else {
        printf("\n --asynch signal :%d\n",sig_no);
        return top_task_asyn_signal(sched,task,sig_no);
    }
}

static inline top_error_t top_task_push_back_rt_sig(top_task_t* task, int sig, void* data)
{
    top_task_rt_sig_t* rt_sig = &task->rt_sigs[sig - 32];
    unsigned int count = top_add_and_fetch(&rt_sig->count,1);
    if(count > rt_sig->max_count) {
        (void)top_sub_and_fetch(&rt_sig->count,1);
        return TOP_ERROR(ENOMEM);
    }
    unsigned int push_idx = top_add_and_fetch(&rt_sig->push_idx,1) % rt_sig->max_count;
    rt_sig->datas[push_idx] = data;
    return TOP_OK;
}

top_error_t top_task_async_rt_signal(struct top_sched_s* sched,struct top_task_s* task,int pos,int sig_no,void* data)
{
    unsigned int sigmask = 1u << sig_no;

    unsigned int old_sigmask = top_fetch_and_or(&task->sigmask,sigmask);
    top_error_t err;

    if(old_sigmask & TOP_TASK_SIG_EXIT) {
        return TOP_ERROR(EINVAL);
    }

    err = top_task_push_back_rt_sig(task,sig_no,data);
    if(top_errno(err)) return err;

    if(sched == task->sched) {
        top_sched_task_signal(sched,task);
        return TOP_OK;
    } else {
        err = top_sched_asyn_task(task->sched,task);
        return err;
    }
}

top_error_t top_task_rt_signal(struct top_task_s* task, int sig_no,void* data)
{
    assert(sig_no >= TOP_TASK_RT_SIG_MIN && sig_no <= TOP_TASK_RT_SIG_MAX);
    int pos = sig_no - 32;
    top_sched_t* sched = top_current_sched();
    if(sched && sched->current == task) {
        top_task_sig_handler handler = task->rt_sigs[pos].handler;
        if(handler) handler(task,task->main_data,sig_no,data);
        return TOP_OK;
    } else {
        return top_task_async_rt_signal(sched,task,pos,sig_no,data);
    }
}

