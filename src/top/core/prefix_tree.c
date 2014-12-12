
#include <top/core/prefix_tree.h>
#include <stdlib.h> //for malloc/free
#include <string.h> //for memset
#include <stdio.h>

#define PREFIX_TREE_NODE( n ) ((struct top_prefix_tree_node*)(n & ~3))

#define PREFIX_TREE_MAP_SLOT_SIZE 7
#define PREFIX_TREE_MAP_KEY_SIZE (PREFIX_TREE_MAP_SLOT_SIZE * sizeof(void*))

struct top_prefix_tree_node {
    union {
        char key[PREFIX_TREE_MAP_KEY_SIZE];
        struct top_prefix_tree_node* slots[PREFIX_TREE_MAP_SLOT_SIZE];
    };

    union {
        struct top_prefix_tree_node* next;
        void* data;
    };
};

struct top_prefix_tree_node_key {
	struct {
		void* data;
	};
	struct top_prefix_tree_node_key* next;
};

struct top_prefix_tree_node_slots {
	union {
	struct top_prefix_tree_node_slots* slots[16];
	struct {
	struct top_prefix_tree_node_slots* small_slots[15];
	void* data;
	};
	};
	struct top_prefix_tree_node_slots* next;
};

union top_prefix_tree_node {
	struct top_prefix_tree_node_key keys; /** sizeof(keys) == sizeof(slots) / 2 */
	struct top_prefix_tree_node_slots slots;
};

enum top_prefix_match_result {
	MATCHED = 0,
	MATCH_NEXT , /** 本节点匹配成功，还需要继续匹配下个节点 */
	MATCH_PARTIAL, /** 本节点部分匹配成功，如果是插入操作，需要分裂当前节点 */
	UNMATCHED, /** 第一个字节就不匹配，如果是插入操作，需要生成新的节点 */
};

struct top_prefix_match_ctx {
	unsigned long* pparent; /** 父节点，父节点的低位会记录当前节点的状态，是否有data，是否有压缩的key */
	struct top_prefix_tree_node_key* current; /** 当前节点，因为内部是链表结构，这可能不是首节点 */
	unsigned int matched_size; /** 当前key中成功匹配的数量 */
	unsigned int max_size; /** 当前key的最大长度，首节点可能保存data数据，因此各个节点的长度是有可能有差异的 */
	unsigned long* pnext; /** 本节点匹配完全成功后需要继续匹配的下一个节点的地址，它成为下一次匹配的pparent的值 */
};

static inline enum top_prefix_match_result top_prefix_tree_node_match(struct top_prefix_tree_node* node,const char* key,struct top_prefix_match_ctx* ctx)
{
	int has_compress_key = ctx->pparent & PREFIX_TREE_COMP_KEY;
	int has_data = ctx->pparent & PREFIX_TREE_DATA;

	if(has_compress_key) {
		ctx->current = &node->key;
		ctx->has_data = has_data;
		return top_prefix_tree_node_match_key(key,ctx);
	}else {
		/** 当前节点匹配成功，直接转下个节点处理 */
		ctx->pnext = top_prefix_tree_node_get_next_addr(node,*key,has_data);
		if(*ctx->pnext) {
			ctx->matched_size = 1;
			return MATCH_NEXT;
		}else {
			ctx->matched_size = 0;
			return UNMATCHED;
		}
	}
}

static inline enum top_prefix_match_result top_prefix_node_match_key(const char* key,struct top_prefix_match_ctx* ctx)
{
    int i;
	unsigned int size = has_data ? PREFIX_TREE_MAP_KEY_SIZE - sizeof(void*) : PREFIX_TREE_MAP_KEY_SIZE;
    do {
        for(i = 0; i < size ++i) {
            if(node->key[i] == 0) {
                if(*key == 0) {
                    goto matched;
                } else {
                    goto suffix;
                }
            } else if(*key == 0) {
                goto prefix;
            } else if(node->key[i] != *key) {
                goto split;
            }
        }
        node = node->next;
    } while(1);
}



void * top_prefix_tree_simple_find(struct top_prefix_tree* tree, const char* key)
{
	if(tree->root == 0) return 0;
	struct top_prefix_tree_match_ctx ctx;
	memset(&ctx,0,sizeof(ctx));
	ctx->pparent = &tree->root;
	enum top_prefix_match_result rlt;
	rlt = top_prefix_tree_node_match(&ctx,PREFIX_TREE_NODE(tree->root),key);
}

