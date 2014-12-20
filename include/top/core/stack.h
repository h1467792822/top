
#ifndef TOP_CORE_STACK_H
#define TOP_CORE_STACK_H

#ifndef TOP_CORE_STDDEF_H
#include <top/core/stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_stack_node {
	struct top_stack_node* next;
};

struct top_stack {
	struct top_stack_node* top;
};

#define top_stack_entry(ptr,type,member) \
	top_container_of(ptr,type,member)

#define TOP_STACK_INIT(name) { 0 }
#define TOP_STACK(name) struct top_stack name = TOP_STACK_INIT(name)

static inline void top_stack_init(struct top_stack* stack)
{
	stack->top = 0;
}

static inline void top_stack_push(struct top_stack* stack,struct top_stack_node* node)
{
	node->next = stack->top;
	stack->top = node;
}

static inline struct top_stack_node* top_stack_pop(struct top_stack* stack)
{
	struct top_stack_node* node = stack->top;
	if(node) stack->top = node->next;
	return node;
}

static inline struct top_stack_node* top_stack_top(struct top_stack* stack)
{
	return stack->top;
}

static inline int top_stack_empty(struct top_stack* stack)
{
	return 0 == stack->top;
}

#ifdef __cplusplus
}
#endif

#endif

