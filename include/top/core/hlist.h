
#ifndef TOP_CORE_HLIST_H
#define TOP_CORE_HLIST_H

#include <top/core/stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct top_hlist_node
{
	struct top_hlist_node* next;
	struct top_hlist_node** pprev;
};


/**
 * struct HashTable { struct top_hlist_head buckets[count]; }
 * 
 */

struct top_hlist_head
{
	struct top_hlist_node* first;
};

#define top_hlist_entry(ptr,type,member) top_container_of(ptr,type,member)

static inline void top_hlist_init(struct top_hlist_head* head) 
{
	head->first = 0;
}

static inline void top_hlist_node_init(struct top_hlist_node* node)
{
	node->next = 0;
	node->pprev = 0;
}

static inline void top_hlist_add_head(struct top_hlist_head* head,struct top_hlist_node* node)
{
	struct top_hlist_node* first = head->first;
	node->next = first;
	if(first) first->pprev = &node->next;
	node->pprev = &head->first;
	head->first = node;
}

static inline void top_hlist_add_before(struct top_hlist_node* pos,struct top_hlist_node* node)
{
	node->pprev = pos->pprev;
	*node->pprev = node;
	node->next = pos;
	pos->pprev = &node->next;
}

static inline void top_hlist_add_after(struct top_hlist_node* pos,struct top_hlist_node* node)
{
	struct top_hlist_node* next = pos->next;
	node->next = next;
	node->pprev = &pos->next;
	if(next) next->pprev = &node->next;
	pos->next = node;
}

static inline void top_hlist_node_del(struct top_hlist_node* node)
{
	struct top_hlist_node* next;
	*node->pprev = next;
	if(next) next->pprev = node->pprev;
}

static inline void top_hlist_move_to(struct top_hlist_head* old,struct top_hlist_head* dest)
{
	dest->first = old->first;
	if(dest->first) dest->first->pprev = &dest->first;
	old->first = 0;
}

#define top_hlist_for_each(head,pos) \
	for(pos = head->first; pos; pos = pos->next)

#define top_hlist_for_each_safe(head,pos,tmp) \
	for(pos = head->first; pos && (tmp = pos->next, 1); pos = tmp)

#define top_hlist_for_each_continue(pos) \
	for(; pos ; pos = pos->next)

#define top_hlist_for_each_safe_continue(pos,tmp) \
	for(; pos && (tmp = pos, 1) ; pos = tmp)

#define top_hlist_for_each_entry(head,entry,member,tmp) \
	for(tmp = head->first; tmp && (entry = top_hlist_entry(tmp,typeof(*entry),member),1); tmp = tmp->next)

#define top_hlist_for_each_entry_continue(entry,member,tmp) \
	for(tmp = entry->member.next; tmp && (entry = top_hlist_entry(tmp,typeof(*entry),member),1); tmp = tmp->next)

#define top_hlist_for_each_entry_safe(head,entry,member,tmp1,tmp2) \
	for(tmp1 = head->first; tmp1 && (entry = top_hlist_entry(tmp1,typeof(*entry),member),tmp2 = tmp1->next,1); tmp1 = tmp2)

#define top_hlist_for_each_entry_safe_continue(entry,member,tmp1,tmp2) \
	for(tmp1 = entry->member.next; tmp1 && (entry = top_hlist_entry(tmp1,typeof(*entry),member),tmp2 = tmp1->next,1); tmp1 = tmp2)

#ifdef __cplusplus
}
#endif

#endif

