
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

/**
  * 如果只想支持小写字母，但输入可能有大写，不想事先遍历进行预处理，则可以如下:
  * top_prefix_tree_slot_map_init(slot_map,"abcdefghijklmnopqrstuvwxyz",-1);
  * top_prefix_tree_slot_map_init_more_key(slot_map,'a',"ABCDEFGHIJKLMNOPQRSTUVWXYZ",-1);
  * 内部比较的时候则会将小写和大写字母完全等同对待.
  * 要支持这个功能，库编译时需要定义#define TOP_PREFIX_TREE_HAS_MORE_KEY_MAP 1
  */
void top_prefix_tree_slot_map_init(top_prefix_tree_slot_map slot_map, const char* key_map, unsigned int key_map_size);
void top_prefix_tree_slot_map_init_more(top_prefix_tree_slot_map slot_map,char equal_start_key,const char* more_key_map,unsigned int key_map_size);

/**
  * default is BASE64 CODEC:
  * if slot_map == 0 || key_map == 0 then 
  *    key_map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
  *    slot_map = init with key_map
  * end
  */
struct top_alloc;
struct top_prefix_tree_conf {
	const unsigned char* slot_map; /** if null, instead of g_top_prefix_def_table */
	const char* key_map;
	unsigned char key_map_size;
	unsigned char bulk; /** 一次申请的node数量, 申请的内存 = bulk * tree->node_size ,if bulk == 0 then bulk = 10*/
	unsigned long max_capacity; /** if max_capacity == 0 then max_capacity = (unsigned long)-1; */
	unsigned long max_length; /** max length of key */
	struct top_alloc* alloc;
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

/**
  * 根据前缀遍历所有节点
  */
struct top_prefix_tree_visit_ctx
{
	/**
	  * @suffix_length 指示后缀的真实长度，即便suffix_buf为空指针，也会提供这个值
	  * @return 0 表示中断visit，非0值表示继续。如果需要返回错误码，用户可以使用user_data来实现.
	  */
	int (*visit)(struct top_prefix_tree_visit_ctx* ctx,void* data,int suffix_length,struct top_prefix_tree* tree);
	void* user_data;
	char* suffix_buf; //用于拷贝后缀
	int suffix_buf_len; // == 0, 表示无需拷贝
};

int top_prefix_tree_simple_visit(struct top_prefix_tree* tree, const char* prefix,struct top_prefix_tree_visit_ctx* ctx);

struct top_prefix_tree_key_vec{
	const char* key;
	const char* key_end;
};

top_error_t top_prefix_tree_insert(struct top_prefix_tree* tree,const struct top_prefix_tree_key_vec* key,int count,void* data,void** pold_data);

void* top_prefix_tree_find(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key,int count);

void* top_prefix_tree_delete(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key,int count);

int top_prefix_tree_visit(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* prefix,int count,struct top_prefix_tree_visit_ctx* ctx);

#ifdef __cplusplus
}
#endif

#endif


