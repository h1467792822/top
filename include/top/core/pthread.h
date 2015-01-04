
#ifndef TOP_CORE_PTHREAD_H
#define TOP_CORE_PTHREAD_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_t top_pthread_t;

typedef struct top_pthread_conf_s {
    top_error_t (*create)(top_pthread_t* ptid,void* (*main)(void* data),void* data, int daemon, unsigned long stack_size, int priority);
    top_error_t (*rt_signal)(top_pthread_t tid, int signo,void* data);
    top_error_t (*signal)(top_pthread_t tid, int signo);
    top_error_t (*join)(top_pthread_t tid);
} top_pthread_conf_t;

extern const struct top_pthread_conf_s* const g_top_pthread_conf_linux;

#ifdef __cplusplus
}
#endif

#endif

