
#ifndef TOP_CORE_RBTREE_H
#define TOP_CORE_RBTREE_H

#ifndef TOP_CORE_STDDEF_H
#include <top/core/stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define top_rb_entry(ptr,type,member) top_container_of(ptr,type,member)

struct top_rbtree_node {
	union {
	struct top_rbtree_node* children[2];
	struct {
		struct top_rbtree_node* left;
		struct top_rbtree_node* right;
	};
	};
	unsigned long parent;
};

static inline struct top_rbtree_node* top_rbtree_node_parent(const struct top_rbtree_node* node)
{
	return (struct top_rbtree_node*)(node->parent & ~3);
}

struct top_rbtree {
	struct top_rbtree_node* root;
};

static inline void top_rbtree_init(struct top_rbtree* tree)
{
	tree->root = 0;
}

void top_rbtree_link_node(struct top_rbtree* tree,struct top_rbtree_node* node,struct top_rbtree_node* parent,struct top_rbtree_node** link) ;

void top_rbtree_erase(struct top_rbtree* tree, struct top_rbtree_node* node);

struct top_rbtree_node* top_rbtree_first(const struct top_rbtree* tree);

struct top_rbtree_node* top_rbtree_last(const struct top_rbtree* tree);

struct top_rbtree_node* top_rbtree_node_next(const struct top_rbtree_node* node);

struct top_rbtree_node* top_rbtree_node_prev(const struct top_rbtree_node* node);


#ifdef __cplusplus
}
#endif

#endif

