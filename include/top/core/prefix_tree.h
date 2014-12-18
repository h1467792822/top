
#ifndef TOP_CORE_PREFIX_TREE_H
#define TOP_CORE_PREFIX_TREE_H

#ifndef TOP_CORE_ERROR_H
#include <top/core/error.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef const char* top_prefix_tree_key_map_const;
typedef unsigned char top_prefix_tree_slot_map[256];
typedef const unsigned char top_prefix_tree_slot_map_const[256];

void top_prefix_tree_slot_map_init(top_prefix_tree_slot_map slot_map, top_prefix_tree_key_map_const key_map, unsigned int key_map_size);

/**
  * default is BASE64 CODEC:
  * if slot_map == 0 || key_map == 0 then 
  *    key_map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
  *    slot_map = init with key_map
  * end
  */
struct top_prefix_tree_conf {
	const unsigned char* slot_map; /** if null, instead of g_top_prefix_def_table */
	const char* key_map;
	unsigned char key_map_size;
	unsigned char bulk; /** 一次申请的node数量, 申请的内存 = bulk * tree->node_size ,if bulk == 0 then bulk = 10*/
	unsigned long max_capacity; /** if max_capacity == 0 then max_capacity = (unsigned long)-1; */
	unsigned long max_length; /** max length of key */
	/**
	* memory management, if pfmalloc or pffree is null, instead of malloc/free 
	*/
	top_error_t (*pfmalloc)(void* user_data,unsigned long size,void** pallocated);
	void (*pffree)(void* user_data,void* pallocated,unsigned long size);
	void* user_data;
};

struct top_prefix_tree_key;
struct top_prefix_tree_slots;
struct top_prefix_tree {
	unsigned long capacity;
	unsigned short bulk_size;
	unsigned short node_size;
	unsigned short max_key_size;
	unsigned long root;
	struct top_prefix_tree_key* cached_key;
	struct top_prefix_tree_slots* cached_slots;
	struct top_prefix_tree_slots* bulk_alloc;
	struct top_prefix_tree_conf conf;
};


void top_prefix_tree_init(struct top_prefix_tree* tree, const struct top_prefix_tree_conf * conf);

void top_prefix_tree_fini(struct top_prefix_tree* tree);

top_error_t top_prefix_tree_simple_insert(struct top_prefix_tree* tree, const char* key,void* data,void** pold_data);

void* top_prefix_tree_simple_find(struct top_prefix_tree* tree,const char* key);

void* top_prefix_tree_simple_delete(struct top_prefix_tree* tree,const char* key);

typedef int (*pf_top_prefix_tree_visit)(void* visitor,struct top_prefix_tree* tree,const char* suffix, int suffix_len,void* data);

void top_prefix_tree_simple_visit(struct top_prefix_tree* tree, const char* prefix,int copy_suffix, pf_top_prefix_tree_visit pfvisit,void* visitor);

struct top_prefix_tree_key_vec{
	const char* key;
	const char* key_end;
};

top_error_t top_prefix_tree_insert(struct top_prefix_tree* tree,const struct top_prefix_tree_key_vec* key,int count,void* data);

void* top_prefix_tree_find(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key,int count);

void* top_prefix_tree_delete(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key,int count);

void top_prefix_tree_visit(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* prefix,int count,int copy_suffix, pf_top_prefix_tree_visit pfvisit,void* visitor);

#ifdef __cplusplus
}
#endif

#endif


