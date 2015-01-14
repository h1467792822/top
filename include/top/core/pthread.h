
#ifndef TOP_CORE_PTHREAD_H
#define TOP_CORE_PTHREAD_H

#include <pthread.h>

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_t top_pthread_t;

typedef struct top_pthread_param_s {
	void* (*main)(void* data);
	void* main_data;
	int daemon;
   	unsigned long stack_size;
   	int priority;
}top_pthread_param_t;

typedef struct top_pthread_conf_s {
    top_error_t (*create)(struct top_pthread_conf_s* conf,top_pthread_t* ptid,top_pthread_param_t* param);
    top_error_t (*rt_signal)(struct top_pthread_conf_s* conf,top_pthread_t tid, int signo,void* data);
    top_error_t (*signal)(struct top_pthread_conf_s* conf,top_pthread_t tid, int signo);
    top_error_t (*join)(struct top_pthread_conf_s* conf,top_pthread_t tid);
} top_pthread_conf_t;

extern const struct top_pthread_conf_s* g_top_pthread_conf_linux;

static inline top_error_t top_pthread_create(struct top_pthread_conf_s* conf,top_pthread_t* ptid,top_pthread_param_t* param)
{
	return conf->create(conf,ptid,param);
}

static inline top_error_t top_pthread_rt_signal(struct top_pthread_conf_s* conf,top_pthread_t tid,int signo,void* data)
{
	return conf->rt_signal(conf,tid,signo,data);
}

static inline top_error_t top_pthread_signal(struct top_pthread_conf_s* conf,top_pthread_t tid,int signo)
{
	return conf->signal(conf,tid,signo);
}

static inline top_error_t top_pthread_join(struct top_pthread_conf_s* conf,top_pthread_t tid)
{
	return conf->join(conf,tid);
}




#ifdef __cplusplus
}
#endif

#endif

