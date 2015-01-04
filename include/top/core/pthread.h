
#ifndef TOP_CORE_PTHREAD_H
#define TOP_CORE_PTHREAD_H

#include <pthread.h>

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
    top_error_t (*create)(void* user_data,top_pthread_t* ptid,top_pthread_param_t* param);
    top_error_t (*rt_signal)(void* user_data,top_pthread_t tid, int signo,void* data);
    top_error_t (*signal)(void* user_data,top_pthread_t tid, int signo);
    top_error_t (*join)(void* user_data,top_pthread_t tid);
	void* user_data;
} top_pthread_conf_t;

extern struct top_pthread_conf_s* g_top_pthread_conf_linux;

static inline top_error_t top_pthread_create(struct top_thread_conf_s* conf,top_pthread_t* ptid,top_pthread_param_t* param)
{
	return conf->create(conf->user_data,ptid,param);
}

static inline top_error_t top_pthread_rt_signal(struct top_thread_conf_s* conf,top_pthread_t tid,int signo,void* data)
{
	return conf->rt_signal(conf->user_data,tid,signo,data);
}

static inline top_error_t top_pthread_signal(struct top_thread_conf_s* conf,top_pthread_t tid,int signo)
{
	return conf->signal(conf->user_data,tid,signo);
}

static inline top_error_t top_pthread_join(struct top_thread_conf_s* conf,top_pthread_t tid)
{
	return conf->join(conf->user_data,tid);
}




#ifdef __cplusplus
}
#endif

#endif

