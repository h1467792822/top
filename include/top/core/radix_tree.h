
#ifndef TOP_CORE_RADIX_TREE_H
#define TOP_CORE_RADIX_TREE_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef top_error_t (*pfmalloc)(unsigned long size,void** pallocated);
typedef void (*pffree)(void* pallocated,unsigned long size);


struct top_radix_tree_conf {
	unsigned long max_capacity;
	top_error_t (*pfmalloc)(void* user_data,unsigned long size,void** pallocated);
	void (*pffree)(void* user_data,void* pallocated,unsigned long size);
	void* user_data;
};

struct top_radix_tree_node;
struct top_radix_tree {
	int height;
	unsigned long capacity;
	struct top_radix_tree_node* root;
	struct top_radix_tree_node* cached;
	struct top_radix_tree_node* bulk_alloc;
	struct top_radix_tree_conf conf;
};


void top_radix_tree_init(struct top_radix_tree* tree, const struct top_radix_tree_conf * conf);

void top_radix_tree_fini(struct top_radix_tree* tree);

top_error_t top_radix_tree_insert(struct top_radix_tree* tree, unsigned long key,void* data);

void* top_radix_tree_find(struct top_radix_tree* tree,unsigned long key);

void* top_radix_tree_delete(struct top_radix_tree* tree,unsigned long key);

#ifdef __cplusplus
}
#endif

#endif


