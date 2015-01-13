
#ifndef TOP_CORE_SYNC_H
#define TOP_CORE_SYNC_H

#include <top/core/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct top_mutex_s {
	struct top_task_s* locked;
	top_list_t waiting;
	int waiting_count;
}top_mutex_t;

void top_mutex_init(top_mutex_t* mutex);
top_error_t top_mutex_fini(top_mutex_t* mutex);
void top_mutex_lock(top_mutex_t* mutex);
void top_mutex_unlock(top_mutex_t* mutex);

typedef struct top_cond_s {
	top_list_t waiting;
	int waiting_count;
	int sig_count;
}top_cond_t;

void top_cond_init(top_cond_t* cond);
top_error_t top_cond_fini(top_cond_t* cond);
void top_cond_wait(top_cond_t* cond);
void top_cond_signal(top_cond_t* cond);

typedef struct top_event_s {
	top_list_t waiting;
	int waiting_count;
	int evt_count;
}top_event_t;

void top_event_init(top_event_t* event);
top_error_t top_event_fini(top_event_t* event);
void top_event_wait(top_event_t* event);
void top_event_notify(top_event_t* event);

#ifdef __cplusplus
}
#endif

#endif

