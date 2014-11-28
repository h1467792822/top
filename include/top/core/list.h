

#ifndef TOP_CORE_LIST_H
#define TOP_CORE_LIST_H

#ifndef TOP_CORE_STDDEF_H
#include <top/core/stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_list_node {
	struct top_list_node* next;
	struct top_list_node* prev;
};

struct top_list {
	struct top_list_node* first;
	struct top_list_node* last;
};

#define TOP_LIST_INIT(name) {(struct top_list_node*)&name,(struct top_list_node)&name}

#define TOP_LSIT (name) top_list name = TOP_LIST_INIT(name)

static inline void top_list_init(struct top_list* list) {
	list->first = list->last = (struct top_list_node*)list;
}

static inline void top_list_node_insert(struct top_list_node* pos,struct top_list_node* newnode) {
	struct top_list_node* prev = pos->prev;
	newnode->prev = prev;
	prev->next = newnode;
	newnode->next = pos;
	pos->prev = newnode;
}

static inline void top_list_add(struct top_list* list,struct top_list_node* node) {
	top_list_node_insert(list->first,node);
}

static inline void top_list_add_tail(struct top_list* list,struct top_list_node* node) {
	top_list_node_insert((struct top_list_node*)list,node);
}

static inline void top_list_node_remove(struct top_list_node* node) {
	struct top_list_node* prev = node->prev;
	struct top_list_node* next = node->next;
	prev->next = next;
	next->prev = prev;
}

// 
static inline void top_list_range_remove(struct top_list_node* first, struct top_list_node* last) {
	struct top_list_node* prev = first->prev;
	struct top_list_node* next = last->next;
	prev->next = next;
	next->prev = prev;
}

static inline void top_list_node_move_at(struct top_list_node* node,struct top_list_node* pos) {
	top_list_node_remove(node);
	top_list_node_insert(pos,node);
}

static inline void top_list_range_move_at(struct top_list_node* first,struct top_list_node* last,struct top_list_node* pos) {
	struct top_list_node* prev = pos->prev;
	top_list_range_remove(first,last);
	prev->next = first;
	first->prev = prev;
	last->next = pos;
	pos->prev = last;
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
	return list->first == (struct top_list_node*)list;
}

static inline int top_list_singular(struct top_list* list) {
	return !top_list_empty(list) && list->first == list->last;
}


#define list_entry(node,entry,field) top_container_of(node,entry,field)

#define top_list_for_each(list,node) \
	for( node= list->first; node != (struct top_list_node*)list; node = node->next)

#define top_list_for_each_reverse(list,node) \
	for( node= list->last; node != (struct top_list_node*)list; node = node->prev)

#define top_list_for_each_from(list,node) \
	for( ; node != (struct top_list_node*)list; node = node->next)

#define top_list_for_each_reverse_from(list,node) \
	for( ; node != (struct top_list_node*)list; node = node->prev)

#define top_list_for_each_safe(list,node,tmp) \
	for( node = list->first,tmp = node->next; node != (struct top_list_node*)list; node = tmp,tmp = node->next)

#define top_list_for_each_reverse_safe(list,node,tmp) \
	for( node = list->last,tmp = node->prev; node != (struct top_list_node*)list; node = tmp,tmp = node->prev)

#define top_list_for_each_safe_from(list,node,tmp) \
	for( tmp = node->next; node != (struct top_list_node*)list; node = tmp,tmp = node->next)

#define top_list_for_each_reverse_safe_from(list,node,tmp) \
	for( tmp = node->prev; node != (struct top_list_node*)list; node = tmp,tmp = node->prev)

#define top_list_for_each_entry(list,entry,field) \
	for( entry = list_entry(list->first,typeof(*entry),field); &entry->field != (struct top_list_node*)list; node = list_entry(entry->field.next,typeof(*entry),field))

#define top_list_for_each_entry_reverse(list,entry,field) \
	for( entry = list_entry(list->last,typeof(*entry),field); &entry->field != (struct top_list_node*)list; node = list_entry(entry->field.prev,typeof(*entry),field))

#define top_list_for_each_entry_from(list,entry,field) \
	for( ; &entry->field != (struct top_list_node*)list; node = list_entry(entry->field.next,typeof(*entry),field))

#define top_list_for_each_entry_reverse_from(list,entry,field) \
	for( ; &entry->field != (struct top_list_node*)list; node = list_entry(entry->field.prev,typeof(*entry),field))

#define top_list_for_each_entry_safe(list,entry,field,tmp) \
	for( entry = list_entry(list->first,typeof(*entry),field),tmp = list_entry(entry->field.next,typeof(*entry),field); &entry->field != (struct top_list_node*)list; entry = tmp,tmp = list_entry(entry->field.next,typeof(*entry),field))

#define top_list_for_each_entry_reverse_safe(list,entry,field,tmp) \
	for( entry = list_entry(list->last,typeof(*entry),field),tmp = list_entry(entry->field.prev,typeof(*entry),field); &entry->field != (struct top_list_node*)list; entry = tmp,tmp = list_entry(entry->field.prev,typeof(*entry),field))

#define top_list_for_each_entry_safe_from(list,entry,field,tmp) \
	for( tmp = list_entry(entry->field.next,typeof(*entry),field); &entry->field != (struct top_list_node*)list; entry = tmp,tmp = list_entry(entry->field.next,typeof(*entry),field))

#define top_list_for_each_entry_reverse_safe_from(list,entry,field,tmp) \
	for( tmp = list_entry(entry->field.prev,typeof(*entry),field); &entry->field != (struct top_list_node*)list; entry = tmp,tmp = list_entry(entry->field.prev,typeof(*entry),field))


#ifdef __cplusplus
}
#endif

#endif

