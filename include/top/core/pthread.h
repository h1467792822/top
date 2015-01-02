
#ifndef TOP_CORE_PTHREAD_H
#define TOP_CORE_PTHREAD_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_t top_pthread_t;
#define top_pthread_kill pthread_kill

#ifdef __cplusplus
}
#endif

#endif

