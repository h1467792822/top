
#ifndef TOP_CORE_SYNC_H
#define TOP_CORE_SYNC_H

#include <pthread.h>

#ifndef TOP_CORE_LIST_H
#include <top/core/list.h>
#endif

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct top_mutex_s {
    struct top_task_s* locked;
    struct top_list waiting;
    int waiting_count;
} top_mutex_t;

void top_mutex_init(top_mutex_t* mutex);
top_error_t top_mutex_fini(top_mutex_t* mutex);
void top_mutex_lock(top_mutex_t* mutex);
void top_mutex_unlock(top_mutex_t* mutex);

typedef struct top_cond_s {
    struct top_list waiting;
    int waiting_count;
    int sig_count;
} top_cond_t;

void top_cond_init(top_cond_t* cond);
top_error_t top_cond_fini(top_cond_t* cond);
void top_cond_wait(top_cond_t* cond);
void top_cond_signal(top_cond_t* cond);

typedef struct top_sem_s {
    struct top_list waiting;
    int waiting_count;
    int value;
} top_sem_t;

void top_sem_init(top_sem_t* sem);
top_error_t top_sem_fini(top_sem_t* sem);
void top_sem_wait(top_sem_t* sem);
void top_sem_post(top_sem_t* sem);

#ifdef __cplusplus
}
#endif

#endif

