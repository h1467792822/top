
#ifndef TOP_CORE_PREFIX_TREE_H
#define TOP_CORE_PREFIX_TREE_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
* char_2_idx[c] :[ -1 ,0..(key_set_count - 1)]
*/
struct top_prefix_table {
	unsigned char key_2_idx[256];
	unsigned char key_set_count;
	unsigned char idx_2_key[1];
};

/**
* key: [0-9a-zA-Z.-_]*
*/
extern const struct top_prefix_table* const g_top_prefix_def_tabl;

struct top_prefix_tree_conf {
	const struct top_prefix_table* table; /** if null, instead of g_top_prefix_def_table */
	unsigned long max_capacity;
	unsigned long max_length; /** max length of key */
	/**
	* memory management, if pfmalloc or pffree is null, instead of malloc/free 
	*/
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

top_error_t top_prefix_tree_simple_insert(struct top_prefix_tree* tree, const char* key,void* data);

void* top_prefix_tree_simple_find(struct top_prefix_tree* tree,const char* key);

void* top_prefix_tree_simple_delete(struct top_prefix_tree* tree,const char* key);

typedef int (*pf_top_prefix_tree_visit)(void* visitor,struct top_prefix_tree* tree,const char* suffix, int suffix_len,void* data);

void top_prefix_tree_simple_visit(struct top_prefix_tree* tree, const char* prefix,int copy_suffix, pf_top_prefix_tree_visit pfvisit,void* visitor);

struct top_prefix_key_vec{
	const char* key;
	unsigned int len;
};

top_error_t top_prefix_tree_insert(struct top_prefix_tree* tree,const struct top_prefix_key_vec* key,int count,void* data);

void* top_prefix_tree_find(struct top_prefix_tree* tree, const struct top_prefix_key_vec* key,int count);

void* top_prefix_tree_delete(struct top_prefix_tree* tree, const struct top_prefix_key_vec* key,int count);

void top_prefix_tree_visit(struct top_prefix_tree* tree, const struct top_prefix_key_vec* prefix,int count,int copy_suffix, pf_top_prefix_tree_visit pfvisit,void* visitor);

#ifdef __cplusplus
}
#endif

#endif


