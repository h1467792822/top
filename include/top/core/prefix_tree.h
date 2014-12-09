
#ifndef TOP_CORE_PREFIX_TREE_H
#define TOP_CORE_PREFIX_TREE_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct top_prefix_tree_conf {
	unsigned long max_capacity;
	top_error_t (*pfmalloc)(void* user_data,unsigned long size,void** pallocated);
	void (*pffree)(void* user_data,void* pallocated,unsigned long size);
	void* user_data;
};

struct top_prefix_tree_node;
struct top_prefix_tree {
	unsigned long capacity;
	struct top_prefix_tree_node* root;
	struct top_prefix_tree_node* cached;
	struct top_prefix_tree_node* bulk_alloc;
	struct top_prefix_tree_conf conf;
};


void top_prefix_tree_init(struct top_prefix_tree* tree, const struct top_prefix_tree_conf * conf);

void top_prefix_tree_fini(struct top_prefix_tree* tree);

top_error_t top_prefix_tree_insert(struct top_prefix_tree* tree, unsigned long key,void* data);

void* top_prefix_tree_find(struct top_prefix_tree* tree,unsigned long key);

void* top_prefix_tree_delete(struct top_prefix_tree* tree,unsigned long key);

#ifdef __cplusplus
}
#endif

#endif


