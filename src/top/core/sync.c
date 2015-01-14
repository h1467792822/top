
#include <assert.h>
#include <errno.h>
#include <top/core/sync.h>
#include <top/core/sched.h>

void top_mutex_init(top_mutex_t* mutex)
{
    mutex->locked = 0;
    top_list_init(&mutex->waiting);
    mutex->waiting_count = 0;
}

top_error_t top_mutex_fini(top_mutex_t* mutex)
{
    if(mutex->locked) return TOP_ERROR(EBUSY);
    return TOP_OK;
}

void top_mutex_lock(top_mutex_t* mutex)
{
    top_task_t* task = top_current_task();
    if(mutex->locked) {
        top_list_add_tail(&mutex->waiting,&task->node);
        ++mutex->waiting_count;
        task->state = TOP_TASK_ST_LOCK_WAIT;
        TOP_TASK_SUSPEND(task);
    } else {
        mutex->locked = top_current_task();
    }
}

void top_mutex_unlock(top_mutex_t* mutex)
{
    assert(mutex->locked == top_current_task());
    if(mutex->waiting_count) {
        --mutex->waiting_count;
        mutex->locked = top_list_entry(top_list_remove_first(&mutex->waiting),top_task_t,node);
        TOP_TASK_RESUME(mutex->locked);
    } else {
        mutex->locked = 0;
    }
}

void top_cond_init(top_cond_t* cond)
{
    top_list_init(&cond->waiting);
    cond->sig_count = cond->waiting_count = 0;
}

top_error_t top_cond_fini(top_cond_t* cond)
{
    if(cond->sig_count) return TOP_ERROR(EBUSY);
    return TOP_OK;
}

void top_cond_wait(top_cond_t* cond)
{
    top_task_t* task = top_current_task();
    if(cond->sig_count == 0) {
        ++cond->waiting_count;
        top_list_add_tail(&cond->waiting,&task->node);
        TOP_TASK_SUSPEND(task);
    }
    cond->sig_count = 0;
}

void top_cond_signal(top_cond_t* cond)
{
    if(cond->waiting_count) {
        top_task_t* task = top_list_entry(top_list_remove_first(&cond->waiting),top_task_t,node);
        --cond->waiting_count;
        task->state = TOP_TASK_ST_COND_WAIT;
        TOP_TASK_RESUME(task);
    } else {
        cond->sig_count = 1;
    }
}

void top_sem_init(top_sem_t* sem)
{
    top_list_init(&sem->waiting);
    sem->value = sem->waiting_count = 0;
}

top_error_t top_sem_fini(top_sem_t* sem)
{
    if(sem->value) return TOP_ERROR(EBUSY);
    return TOP_OK;
}

void top_sem_wait(top_sem_t* sem)
{
    top_task_t* task = top_current_task();
    if(sem->value == 0) {
        ++sem->waiting_count;
        top_list_add_tail(&sem->waiting,&task->node);
        task->state = TOP_TASK_ST_EVENT_WAIT;
        TOP_TASK_SUSPEND(task);
    }
    --sem->value ;
}

void top_sem_post(top_sem_t* sem)
{
    if(sem->waiting_count) {
        top_task_t* task;
        task = top_list_entry(top_list_remove_first(&sem->waiting),top_task_t,node);
        --sem->waiting_count;
        TOP_TASK_RESUME(task);
    } else {
        ++sem->value;
    }
}

