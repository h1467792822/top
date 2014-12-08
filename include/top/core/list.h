

#ifndef TOP_CORE_LIST_H
#define TOP_CORE_LIST_H

#ifndef TOP_CORE_STDDEF_H
#include <top/core/stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define top_list_entry(ptr,type,member) top_container_of(ptr,type,member)

struct top_list_node {
	struct top_list_node* next;
	struct top_list_node* prev;
};

struct top_list {
	struct top_list_node* first;
	struct top_list_node* last;
};

#define TOP_LIST_INIT(name) {(struct top_list_node*)&name,(struct top_list_node*)&name}

#define TOP_LIST(name) struct top_list name = TOP_LIST_INIT(name)

static inline void top_list_init(struct top_list* list) {
	list->first = list->last = (struct top_list_node*)(list);
}

static inline void __top_list_node_link(struct top_list_node* prev,struct top_list_node* next){
	prev->next = next;
	next->prev = prev;
}

static inline void top_list_node_insert(struct top_list_node* newnode,struct top_list_node* pos) {
	__top_list_node_link(pos->prev,newnode);
	__top_list_node_link(newnode,pos);
}

static inline void top_list_add(struct top_list* list,struct top_list_node* node) {
	__top_list_node_link(node,list->first);
	__top_list_node_link((struct top_list_node*)list,node);
}

static inline void top_list_add_tail(struct top_list* list,struct top_list_node* node) {
	__top_list_node_link(list->last,node);
	__top_list_node_link(node,(struct top_list_node*)list);
}

static inline void top_list_node_del(struct top_list_node* node) {
	__top_list_node_link(node->prev,node->next);
}

// 
static inline void top_list_range_del(struct top_list_node* first, struct top_list_node* last) {
	__top_list_node_link(first->prev,last->next);
}

static inline void top_list_node_move_at(struct top_list_node* node,struct top_list_node* pos) {
	top_list_node_del(node);
	top_list_node_insert(node,pos);
}

static inline void top_list_range_move_at(struct top_list_node* first,struct top_list_node* last,struct top_list_node* pos) {
	struct top_list_node* prev = pos->prev;
	top_list_range_del(first,last);
	__top_list_node_link(pos->prev,first);
	__top_list_node_link(last,pos);
}

static inline void top_list_node_move(struct top_list_node* node,struct top_list* list) {
	top_list_node_move_at(node,list->first);
}

static inline void top_list_node_move_tail(struct top_list_node* node,struct top_list* list) {
	top_list_node_move_at(node,(struct top_list_node*)list);
}

static inline void top_list_range_move(struct top_list_node* first,struct top_list_node* last,struct top_list* list) {
	top_list_range_move_at(first,last,list->first);
}

static inline void top_list_range_move_tail(struct top_list_node* first,struct top_list_node* last,struct top_list* list) {
	top_list_range_move_at(first,last,(struct top_list_node*)list);
}

static inline void top_list_move(struct top_list* src, struct top_list* dest) {
	top_list_range_move(src->first,src->last,dest);
}

static inline void top_list_move_tail(struct top_list* src,struct top_list* dest) {
	top_list_range_move_tail(src->first,src->last,dest);
}

static inline int top_list_empty(struct top_list* list) {
	return list->first == (struct top_list_node*)(list);
}

static inline int top_list_singular(struct top_list* list) {
	return !top_list_empty(list) && list->first == list->last;
}

#define top_list_for_each(list,node) \
	for( (node) = (list)->first; (node) != (struct top_list_node*)(list); (node) = (node)->next)

#define top_list_for_each_reverse(list,node) \
	for( (node)= (list)->last; (node) != (struct top_list_node*)(list); (node) = (node)->prev)

#define top_list_for_each_from(list,node) \
	for( ; (node) != (struct top_list_node*)(list); (node) = (node)->next)

#define top_list_for_each_reverse_from(list,node) \
	for( ; (node) != (struct top_list_node*)(list); (node) = (node)->prev)

#define top_list_for_each_safe(list,node,tmp) \
	for( (node) = (list)->first,(tmp) = (node)->next; (node) != (struct top_list_node*)(list); (node) = (tmp),(tmp) = (node)->next)

#define top_list_for_each_safe_reverse(list,node,tmp) \
	for( (node) = (list)->last,(tmp) = (node)->prev; (node) != (struct top_list_node*)(list); (node) = (tmp),(tmp) = (node)->prev)

#define top_list_for_each_safe_from(list,node,tmp) \
	for( (tmp) = (node)->next; (node) != (struct top_list_node*)(list); (node) = (tmp),(tmp) = (node)->next)

#define top_list_for_each_safe_reverse_from(list,node,tmp) \
	for( (tmp) = (node)->prev; (node) != (struct top_list_node*)(list); (node) = (tmp),(tmp) = (node)->prev)

#define top_list_for_each_entry(list,entry,field) \
	for( (entry) = top_list_entry((list)->first,typeof(*(entry)),field); &(entry)->field != (struct top_list_node*)(list); (entry) = top_list_entry((entry)->field.next,typeof(*(entry)),field))

#define top_list_for_each_entry_reverse(list,entry,field) \
	for( (entry) = top_list_entry((list)->last,typeof(*(entry)),field); &(entry)->field != (struct top_list_node*)(list); (entry) = top_list_entry((entry)->field.prev,typeof(*(entry)),field))

#define top_list_for_each_entry_from(list,entry,field) \
	for( ; &(entry)->field != (struct top_list_node*)(list); (entry) = top_list_entry((entry)->field.next,typeof(*(entry)),field))

#define top_list_for_each_entry_reverse_from(list,entry,field) \
	for( ; &(entry)->field != (struct top_list_node*)(list); (entry) = top_list_entry((entry)->field.prev,typeof(*(entry)),field))

#define top_list_for_each_entry_safe(list,entry,field,tmp) \
	for( (entry) = top_list_entry((list)->first,typeof(*(entry)),field),(tmp) = top_list_entry((entry)->field.next,typeof(*(entry)),field); &(entry)->field != (struct top_list_node*)(list); (entry) = (tmp),(tmp) = top_list_entry((entry)->field.next,typeof(*(entry)),field))

#define top_list_for_each_entry_safe_reverse(list,entry,field,tmp) \
	for( (entry) = top_list_entry((list)->last,typeof(*(entry)),field),(tmp) = top_list_entry((entry)->field.prev,typeof(*(entry)),field); &(entry)->field != (struct top_list_node*)(list); (entry) = (tmp),(tmp) = top_list_entry((entry)->field.prev,typeof(*(entry)),field))

#define top_list_for_each_entry_safe_from(list,entry,field,tmp) \
	for( (tmp) = top_list_entry((entry)->field.next,typeof(*(entry)),field); &(entry)->field != (struct top_list_node*)(list); (entry) = (tmp),(tmp) = top_list_entry((entry)->field.next,typeof(*(entry)),field))

#define top_list_for_each_entry_safe_reverse_from(list,entry,field,tmp) \
	for( (tmp) = top_list_entry((entry)->field.prev,typeof(*(entry)),field); &(entry)->field != (struct top_list_node*)(list); (entry) = (tmp),(tmp) = top_list_entry((entry)->field.prev,typeof(*(entry)),field))


#ifdef __cplusplus
}
#endif

#endif

