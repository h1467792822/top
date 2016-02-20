
#include "state.h"
#include <assert.h>

int sm_notify(struct state_machine* sm,int event,void* data)
{
	assert(sm);
	assert(0 == sm->running);
	sm->running = 1;
	int cur_st = sm->cur_st;
	int new_st = sm->states[cur_st].on_event(sm,event,data);
	if(new_st != cur_st) {
		if(sm->states[cur_st].on_leave){
			sm->states[cur_st].on_leave(sm,new_st);
		}
		sm->cur_st = new_st;
		if(sm->states[new_st].on_enter) {
			sm->states[new_st].on_enter(sm,cur_st);
		}
	}
	sm->running = 0;
}

