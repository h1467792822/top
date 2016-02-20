

#ifndef TB_LIST_H
#define TB_LIST_H

#ifndef TB_STDDEF_H
#include "tb_stddef.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define tb_list_entry(ptr,type,member) container_of(ptr,type,member)

typedef struct tb_list_node {
    struct tb_list_node* next;
    struct tb_list_node* prev;
} tb_list_node_t;

typedef struct tb_list {
    struct tb_list_node* first;
    struct tb_list_node* last;
} tb_list_t;

#define TOP_LIST_INIT(name) {(struct tb_list_node*)&name,(struct tb_list_node*)&name}

#define TOP_LIST(name) struct tb_list name = TOP_LIST_INIT(name)

static inline void tb_list_init(struct tb_list* list)
{
    list->first = list->last = (struct tb_list_node*)(list);
}

static inline void __tb_list_node_link(struct tb_list_node* prev,struct tb_list_node* next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void tb_list_node_insert(struct tb_list_node* newnode,struct tb_list_node* pos)
{
    __tb_list_node_link(pos->prev,newnode);
    __tb_list_node_link(newnode,pos);
}

static inline void tb_list_add(struct tb_list* list,struct tb_list_node* node)
{
    __tb_list_node_link(node,list->first);
    __tb_list_node_link((struct tb_list_node*)list,node);
}

static inline void tb_list_add_tail(struct tb_list* list,struct tb_list_node* node)
{
    __tb_list_node_link(list->last,node);
    __tb_list_node_link(node,(struct tb_list_node*)list);
}

static inline void tb_list_node_del(struct tb_list_node* node)
{
    __tb_list_node_link(node->prev,node->next);
}

//
static inline void tb_list_range_del(struct tb_list_node* first, struct tb_list_node* last)
{
    __tb_list_node_link(first->prev,last->next);
}

static inline void tb_list_node_move_at(struct tb_list_node* node,struct tb_list_node* pos)
{
    tb_list_node_del(node);
    tb_list_node_insert(node,pos);
}

static inline void tb_list_range_move_at(struct tb_list_node* first,struct tb_list_node* last,struct tb_list_node* pos)
{
    struct tb_list_node* prev = pos->prev;
    tb_list_range_del(first,last);
    __tb_list_node_link(pos->prev,first);
    __tb_list_node_link(last,pos);
}

static inline void tb_list_node_move(struct tb_list_node* node,struct tb_list* list)
{
    tb_list_node_move_at(node,list->first);
}

static inline void tb_list_node_move_tail(struct tb_list_node* node,struct tb_list* list)
{
    tb_list_node_move_at(node,(struct tb_list_node*)list);
}

static inline void tb_list_range_move(struct tb_list_node* first,struct tb_list_node* last,struct tb_list* list)
{
    tb_list_range_move_at(first,last,list->first);
}

static inline void tb_list_range_move_tail(struct tb_list_node* first,struct tb_list_node* last,struct tb_list* list)
{
    tb_list_range_move_at(first,last,(struct tb_list_node*)list);
}

static inline void tb_list_move(struct tb_list* src, struct tb_list* dest)
{
    tb_list_range_move(src->first,src->last,dest);
}

static inline void tb_list_move_tail(struct tb_list* src,struct tb_list* dest)
{
    tb_list_range_move_tail(src->first,src->last,dest);
}

static inline int tb_list_empty(struct tb_list* list)
{
    return list->first == (struct tb_list_node*)(list);
}

static inline int tb_list_singular(struct tb_list* list)
{
    return !tb_list_empty(list) && list->first == list->last;
}

static inline struct tb_list_node* tb_list_remove_first(struct tb_list* list)
{
    if(!tb_list_empty(list)) {
        struct tb_list_node* node = list->first;
        tb_list_node_del(node);
        return node;
    }
    return 0;
}

static inline struct tb_list_node* tb_list_remove_last(struct tb_list* list)
{
    if(!tb_list_empty(list)) {
        struct tb_list_node* node = list->last;
        tb_list_node_del(node);
        return node;
    }
    return 0;
}



#define tb_list_for_each(list,node) \
	for( (node) = (list)->first; (node) != (struct tb_list_node*)(list); (node) = (node)->next)

#define tb_list_for_each_reverse(list,node) \
	for( (node)= (list)->last; (node) != (struct tb_list_node*)(list); (node) = (node)->prev)

#define tb_list_for_each_from(list,node) \
	for( ; (node) != (struct tb_list_node*)(list); (node) = (node)->next)

#define tb_list_for_each_reverse_from(list,node) \
	for( ; (node) != (struct tb_list_node*)(list); (node) = (node)->prev)

#define tb_list_for_each_safe(list,node,tmp) \
	for( (node) = (list)->first,(tmp) = (node)->next; (node) != (struct tb_list_node*)(list); (node) = (tmp),(tmp) = (node)->next)

#define tb_list_for_each_safe_reverse(list,node,tmp) \
	for( (node) = (list)->last,(tmp) = (node)->prev; (node) != (struct tb_list_node*)(list); (node) = (tmp),(tmp) = (node)->prev)

#define tb_list_for_each_safe_from(list,node,tmp) \
	for( (tmp) = (node)->next; (node) != (struct tb_list_node*)(list); (node) = (tmp),(tmp) = (node)->next)

#define tb_list_for_each_safe_reverse_from(list,node,tmp) \
	for( (tmp) = (node)->prev; (node) != (struct tb_list_node*)(list); (node) = (tmp),(tmp) = (node)->prev)

#define tb_list_for_each_entry(list,entry,field) \
	for( (entry) = tb_list_entry((list)->first,typeof(*(entry)),field); &(entry)->field != (struct tb_list_node*)(list); (entry) = tb_list_entry((entry)->field.next,typeof(*(entry)),field))

#define tb_list_for_each_entry_reverse(list,entry,field) \
	for( (entry) = tb_list_entry((list)->last,typeof(*(entry)),field); &(entry)->field != (struct tb_list_node*)(list); (entry) = tb_list_entry((entry)->field.prev,typeof(*(entry)),field))

#define tb_list_for_each_entry_from(list,entry,field) \
	for( ; &(entry)->field != (struct tb_list_node*)(list); (entry) = tb_list_entry((entry)->field.next,typeof(*(entry)),field))

#define tb_list_for_each_entry_reverse_from(list,entry,field) \
	for( ; &(entry)->field != (struct tb_list_node*)(list); (entry) = tb_list_entry((entry)->field.prev,typeof(*(entry)),field))

#define tb_list_for_each_entry_safe(list,entry,field,tmp) \
	for( (entry) = tb_list_entry((list)->first,typeof(*(entry)),field),(tmp) = tb_list_entry((entry)->field.next,typeof(*(entry)),field); &(entry)->field != (struct tb_list_node*)(list); (entry) = (tmp),(tmp) = tb_list_entry((entry)->field.next,typeof(*(entry)),field))

#define tb_list_for_each_entry_safe_reverse(list,entry,field,tmp) \
	for( (entry) = tb_list_entry((list)->last,typeof(*(entry)),field),(tmp) = tb_list_entry((entry)->field.prev,typeof(*(entry)),field); &(entry)->field != (struct tb_list_node*)(list); (entry) = (tmp),(tmp) = tb_list_entry((entry)->field.prev,typeof(*(entry)),field))

#define tb_list_for_each_entry_safe_from(list,entry,field,tmp) \
	for( (tmp) = tb_list_entry((entry)->field.next,typeof(*(entry)),field); &(entry)->field != (struct tb_list_node*)(list); (entry) = (tmp),(tmp) = tb_list_entry((entry)->field.next,typeof(*(entry)),field))

#define tb_list_for_each_entry_safe_reverse_from(list,entry,field,tmp) \
	for( (tmp) = tb_list_entry((entry)->field.prev,typeof(*(entry)),field); &(entry)->field != (struct tb_list_node*)(list); (entry) = (tmp),(tmp) = tb_list_entry((entry)->field.prev,typeof(*(entry)),field))


#ifdef __cplusplus
}
#endif

#endif

