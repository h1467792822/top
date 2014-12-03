
#ifndef TOP_CORE_AVLTREE_H
#define TOP_CORE_AVLTREE_H

#include <top/core/stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct top_avltree_node {
	struct top_avltree_node* children[2];
	unsigned long parent;
	int bf;
};

static inline struct top_avltree_node* top_avltree_node_parent(const struct top_avltree_node* node)
{
	return (struct top_avltree_node*)(node->parent & ~3);
}

struct top_avltree {
	struct top_avltree_node* root;
};

void top_avltree_link_node(struct top_avltree* tree,struct top_avltree_node* node,struct top_avltree_node* parent,struct top_avltree_node** link) ;

void top_avltree_erase(struct top_avltree* tree, struct top_avltree_node* node);

struct top_avltree_node* top_avltree_first(const struct top_avltree* tree);

struct top_avltree_node* top_avltree_last(const struct top_avltree* tree);

struct top_avltree_node* top_avltree_node_next(const struct top_avltree_node* node);

struct top_avltree_node* top_avltree_node_prev(const struct top_avltree_node* node);


#ifdef __cplusplus
}
#endif

#endif

