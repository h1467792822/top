
#include <pthread.h>
#include <signal.h>
#include <top/core/pthread.h>

static top_error_t top_pthread_conf_linux_create(top_pthread_conf_t* conf,top_pthread_t* ptid,top_pthread_param_t* param)
{
    pthread_attr_t attr;
    int retno;

    retno = pthread_attr_init(&attr);
    if(retno) goto fail;

    if(param) {
        (void)pthread_attr_setdetachstate(&attr,param->daemon ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
        if(param->stack_size) (void)pthread_attr_setstacksize(&attr,param->stack_size);
        if(param->priority) {
            struct sched_param sp = { param->priority };
            (void)pthread_attr_setschedparam(&attr,&sp);
        }
    }

    retno = pthread_create(ptid,&attr,param->main,param->main_data);
    pthread_attr_destroy(&attr);
fail:
    return TOP_ERROR(retno);
}

static top_error_t top_pthread_conf_linux_join(top_pthread_conf_t* conf,top_pthread_t tid)
{
    int retno = pthread_join(tid,0);
    return TOP_ERROR(retno);
}

static top_error_t top_pthread_conf_linux_signal(top_pthread_conf_t* conf,top_pthread_t tid,int signo)
{
    int retno = pthread_kill(tid,signo);
    return TOP_ERROR(retno);
}

static top_error_t top_pthread_conf_linux_rt_signal(top_pthread_conf_t* conf,top_pthread_t tid,int signo,void* data)
{
    union sigval value;
    value.sival_ptr = data;
    int retno = pthread_sigqueue(tid,signo,value);
    return TOP_ERROR(retno);
}

static struct top_pthread_conf_s g_top_pthread_conf_linux_v = {
    .create = top_pthread_conf_linux_create,
    .join = top_pthread_conf_linux_join,
    .signal = top_pthread_conf_linux_signal,
    .rt_signal = top_pthread_conf_linux_rt_signal,
};

const struct top_pthread_conf_s* g_top_pthread_conf_linux = &g_top_pthread_conf_linux_v;
