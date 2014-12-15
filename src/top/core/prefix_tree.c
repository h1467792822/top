
#include <top/core/prefix_tree.h>
#include <stdlib.h> //for malloc/free
#include <string.h> //for memset
#include <stdio.h>

#define SLOT_DATA 2
#define KEY_DATA 1

#define PREFIX_TREE_KEY( n ) ((struct top_prefix_tree_node*)(n & ~3))
#define PREFIX_TREE_SLOTS( n ) ((struct top_prefix_tree_node*)(n & ~3))
#define PREFIX_TREE_IS_KEY( n ) ( n & KEY_DATA )
#define PREFIX_TREE_SLOT_HAS_DATA( slot ) (slot->slots[0] & SLOT_DATA)

#define PREFIX_TREE_SLOT_SIZE 15
#define PREFIX_TREE_KEY_SIZE (3 * sizeof(void*))


typedef unsigned int top_prefix_tree_slot_map[256];
typedef const unsigned int top_prefix_tree_slot_map_const[256];
typedef char top_prefix_tree_key_map[65];
typedef const char top_prefix_tree_key_map_const[65];

void top_prefix_slot_map_init(to_prefix_tree_slot_map slot_map, top_prefix_tree_key_map_const key_map)
{
    int i;
    for(i = 0; i < 256; ++i)
        slot_map[i] = 255;
    for(i = 0; i < 64; ++i)
        slot_map[key_map[i]] = i;
    slot_map[255] = 254;
}

struct top_prefix_tree_key {
    unsigned long next;
    char key[PREFIX_TREE_KEY_SIZE];
};


struct top_prefix_tree_slots {
	union {
		struct {
    unsigned long slots[PREFIX_TREE_SLOT_SIZE];
    unsigned long next;
		};
		struct top_prefix_tree_key keys[4];
		struct {
			int avail_num;
			struct top_prefix_tree_slots* next_page;
		};
	};
};

enum top_prefix_match_result {
    MATCH_ALL = 0,
    MATCH_NEXT , /** 本节点匹配成功，还需要继续匹配下个节点 */
    MATCH_PREFIX, /** 本节点部分匹配成功，如果是插入操作，需要分裂当前节点 */
    MATCH_PARTIAL, /** 存在不匹配的字符，如果是插入操作，需要生成新的节点 */
};

struct top_prefix_tree_ctx {
    unsigned long* pparent; /** 父节点，父节点的低位会记录当前节点的状态，是否有data，是否有压缩的key */
    struct top_prefix_tree_node_key* current; /** 当前节点，因为内部是链表结构，这可能不是首节点 */
    unsigned int matched_size; /** 当前key中成功匹配的数量 */
    unsigned int end_pos;
    const char* key; /** 待匹配key */
};

static inline void top_prefix_tree_free_bulk(struct top_prefix_tree* tree)
{
	struct top_prefix_tree_slots* slots ;
	while(tree->bulk_alloc) {
		slots = tree->bulk_alloc;
		tree->bulk_alloc = slots->next_page;
		tree->conf.pffree(tree->conf.user_data,slots,PREFIX_TREE_PAGE_SIZE);
	}
}

static inline void top_prefix_tree_free_slots(struct top_prefix_tree* tree, struct top_prefix_tree_slots* slots)
{
	slots->next = (unsigned long)tree->cached_slots;
	tree->cached_slots = slots;
}

static inline top_error_t top_prefix_tree_alloc_slots(struct top_prefix_tree* tree, struct top_prefix_tree_slots** pslots)
{
	if(tree->cached_slots) {
		*pslots = tree->cached_slots;
		tree->cached_slots = (struct top_prefix_tree_slots*)(*pslots)->next;
		return TOP_OK;
	}

	if(tree->bulk_alloc && tree->bulk_alloc->avail_num) {
		*pslots = (tree->bulk_alloc + tree->bulk_alloc->avail_num);
		--tree->bulk_alloc_avail_num;
		return TOP_OK;
	}

	top_error_t err;
	struct top_prefix_tree_slots* slots;
	err = tree->conf.pfmalloc(tree->conf.user_data,PREFIX_TREE_PAGE_SIZE,&slots);
	if(top_errno(err)) return err;

	slots->avail_num = (PREFIX_TREE_PAGE_SIZE / sizeof(*slots) - 2);
	slots->next = tree->bulk_alloc;
	tree->bulk_alloc = slots;

	*pslots = slots + (PREFIX_TREE_PAGE_SIZE / sizeof(*slots) - 1);
	return TOP_OK;
}

static inline void top_prefix_tree_free_key(struct top_prefix_tree* tree, struct top_prefix_tree_key* key)
{
	key->next = (unsigned long) tree->cached_key;
	tree->cached_key = key;
}

static inline top_error_t top_prefix_tree_alloc_key(struct top_prefix_tree* tree, struct top_prefix_tree_key** pkey)
{
	if(tree->cached_key) {
		*pkey = tree->cached_key;
		tree->cached_key = (struct top_prefix_tree_key*)(*pkey)->next;
		return TOP_OK;
	}

	struct top_prefix_tree_slots* slots;
	top_error_t err;
	err = top_prefix_tree_alloc_slots(struct top_prefix_tree* tree,&slots);
	if(top_errno(err)) return err;

	slots->keys[1].next = (unsigned long)&slots->keys[2];	
	slots->keys[2].next = (unsigned long)&slots->keys[3];	
	*pkey = &slots->keys[0];
	return TOP_OK;
}


static inline void* top_prefix_tree_slots_get_data(struct top_prefix_tree_slots* slots);
static inline void* top_prefix_tree_slots_set_data(struct top_prefix_tree_slots* slots,void* data);
static inline void* top_prefix_tree_ctx_get_data(struct top_prefix_tree_ctx* ctx);
static inline void* top_prefix_tree_ctx_set_data(struct top_prefix_tree_ctx* ctx,void* data);
static inline struct top_prefix_tree_slots* top_prefix_tree_slots_get_slot(struct top_prefix_tree_slots* slots,char c);
struct enum top_prefix_match_result top_prefix_tree_key_match(struct top_prefix_tree_ctx* ctx);


static inline void* top_prefix_tree_ctx_find(struct top_prefix_tree_ctx* ctx)
{
    int has_compress_key;
    struct top_prefix_tree_key* tree_key;
    struct top_prefix_tree_slots* tree_slots;
    do {
        has_compress_key = PREFIX_TREE_IS_KEY(*ctx->pparent);
        if(has_compress_key) {
            tree_key = PREFIX_TREE_KEY(*ctx->pparent);
            if(tree_key == 0 || *ctx->key == 0) return 0;
            rlt = top_prefix_tree_key_match(ctx);
            switch(rlt) {
            case MATCHED:
                return top_prefix_tree_ctx_get_data(ctx);
            case MATCH_NEXT:
                ctx->pparent = top_prefix_tree_ctx_get_slot(ctx);
                break;
            case MATCH_PREFIX:
            case MATCH_PARTIAL:
            default:
                return 0;
            }
        } else {
            tree_slots = PREFIX_TREE_SLOTS(*ctx->pparent);
            if(tree_slots == 0) return 0;
            if(*ctx->key == 0) return top_prefix_tree_slots_get_data(tree_slots);
            ctx->pparent = top_prefix_tree_slots_get_slot(tree,slots,*ctx->key);
            ++ctx->key;
        }
    } while(1);
}

static inline top_error_t top_prefix_tree_ctx_new_key(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree,void* data,void** pold_data);
static inline top_error_t top_prefix_tree_ctx_split_key(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree,void* data,void** pold_data);
static inline top_error_t top_prefix_tree_ctx_split_new_key(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree,void* data,void** pold_data);

static inline top_error_t top_prefix_tree_ctx_insert(struct top_prefix_tree* tree,struct top_prefix_tree_ctx* ctx,void* data,void** pold_data)
{
    int has_compress_key;
    struct top_prefix_tree_key* tree_key;
    struct top_prefix_tree_slots* tree_slots;
    while(1) {
        if(*ctx->pparent == 0) {
            return top_prefix_tree_ctx_new_key(ctx,tree,data,pold_data);
        }
        has_compress_key = PREFIX_TREE_IS_KEY(*ctx->pparent);
        if(has_compress_key) {
            tree_key = PREFIX_TREE_KEY(*ctx->pparent);
            rlt = top_prefix_tree_key_match(ctx) ;
            switch(rlt) {
            case MATCHED:
				if(pold_data)
                *pold_data = top_prefix_tree_ctx_set_data(ctx,data);
				else
                (void)top_prefix_tree_ctx_set_data(ctx,data);
                return TOP_OK;
            case MATCH_NEXT:
                ctx->pparent = top_prefix_tree_ctx_get_slot(ctx);
                break;
            case MATCH_PREFIX:
                return top_prefix_tree_ctx_split_key(ctx,tree,data,pold_data);
            case MATCH_PARTIAL:
                return top_prefix_tree_ctx_split_new_key(ctx,tree,data,pold_data);
            default:
                assert(0);
                return TOP_ERROR(-1);
            }
        } else {
            tree_slots = PREFIX_TREE_SLOTS(*ctx->pparent);
            if(*ctx->key == 0) {
				if(pold_data)
                *pold_data = top_prefix_tree_slots_set_data(tree_slots,data);
				else
                (void)top_prefix_tree_slots_set_data(tree_slots,data);
                return TOP_OK;
            }
            ctx->pparent = top_prefix_tree_slots_get_slot(tree,slots,*ctx->key);
            ++ctx->key;
        }
    };
}


static inline void* top_prefix_tree_slots_get_data(struct top_prefix_tree_slots* slots)
{
    while(slots) {
        if(slots->slots[0] & SLOT_DATA) return (void*)slots->next;
        slots = (struct top_prefix_tree_slots*)slots->next;
    }
    return 0;
}

static inline void* top_prefix_tree_slots_set_data(struct top_prefix_tree_slots* slots,void* data)
{
    while(slots) {
        if(slots->next == 0) {
            slots->next = (unsigned long)data;
            slots->slots[0] |= SLOT_DATA;
            return 0;
        }
        if(slots->slots[0] & SLOT_DATA) {
            void* old = (void*)slots->next;
            slots->next = (unsigned long)data;
            return old;
        }
        slots = (struct top_prefix_tree_slots*)slots->next;
    }
    return 0;
}

static inline void* top_prefix_tree_ctx_get_data(struct top_prefix_tree_ctx* ctx)
{
    assert(ctx->current);
    if(ctx->current->key[ctx->end_pos] == 254) {
        return (void*)ctx->current->next;
    } else {
        assert(ctx->current->key[ctx->end_pos] == 255);
        return top_prefix_tree_slots_get_data((struct top_prefix_tree_slots*)ctx->current->next);
    }
}

static inline void top_prefix_tree_ctx_set_data(struct top_prefix_tree_ctx* ctx,void* data)
{
    struct top_prefix_tree_key* tree_key = ctx->current;
    assert(tree_key);
    if(tree_key->key[ctx->end_pos] == 255) {
        void* old = (void*)tree_key->next;
        tree_key->next = (unsigned long)data;
        return old;
    } else if (tree_key->next) {
        assert(tree_key->key[ctx->end_pos] == 0);
        return top_prefix_tree_slots_set_data((struct top_prefix_tree_slots*)tree_key->next);
    } else {
        tree_key->key[ctx->end_pos] = 255;
        tree_key->next = (unsigned long)data;
        return 0;
    }
}

static inline unsigned long* top_prefix_tree_slots_get_slot(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c)
{
    unsigned int slot_idx = tree->slot_map[c];
    unsigned int seg_idx ;
    assert(slot_idx < 64);
    while(slots) {
        seg_idx = ((slots->slots[2] )& 1) | ((slots->slots[1]) & 1 << 1) | (slots->slots[0] & 1);
        if(seg_idx == slot_idx / 15) return &slots->slots[slot_idx % 15];
        if(slots->slots[0] & SLOT_DATA) return 0;
        slots = (struct top_prefix_tree_slots*)slots->next;
    }
    return 0;
}

static inline enum top_prefix_match_result top_prefix_tree_key_match(struct top_prefix_tree_key* tree_key,struct top_prefix_tree_ctx* ctx) {
    unsigned int i;
    do {
        for(i = 0; i < PREFIX_TREE_KEY_SIZE; ++i)
        {
            if(tree_key->key[i] == PREFIX_TREE_SKIP_KEY) continue;
            ctx->end_char = tree->slot_map[tree_key->key[i]];
            if(ctx->end_char > 64) {
                ctx->current = tree_key;
                ctx->matched_siz = i;
                ctx->key += i;
                return ctx->key[i] == 0 ? MATCHED : MATCH_NEXT;
            } else if(tree_key->key[i] != ctx->key[i]) {
                ctx->current = tree_key;
                ctx->matched_size = i;
                ctx->key += i;
                return ctx->key[i] == 0 ? MATCH_PREFIX : MATCH_PARTIAL;
            }
        }
        tree_key = tree_key->next;
        ctx->key += PREFIX_TREE_KEY_SIZE;
    } while(tree_key);
}

static inline top_error_t top_prefix_tree_ctx_new_key(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree,void* data,void** pold_data)
{
	struct top_prefix_tree_key* tree_key,*next_key;
	const char* key = ctx->key;
	top_error_t err;
	err = top_prefix_tree_alloc_key(tree,&tree_key);
	if(top_errno(err)) return err;
	*ctx->pparent = (unsigned long)tree_key;
	strncpy(tree_key->key,key,PREFIX_TREE_KEY_SIZE);
	while(tree_key->key[PREFIX_TREE_KEY_SIZE - 1] != 0){
		err = top_prefix_tree_alloc_key(tree,&next_key);
		if(top_errno(err)) goto fail;
		key += PREFIX_TREE_KEY_SIZE;
		strncpy(next_key->key,key,PREFIX_TREE_KEY_SIZE);
		tree_key->next = (unsigned long)next_key;
		tree_key = next_key;
	}
	top_prefix_tree_key_set_data(tree_key,data);
	if(pold_data) *pold_data = 0;
	return TOP_OK;
fail:
	tree_key = *ctx->pparent;
	*ctx->pparent = 0;
	while(tree_key) {
		next_key = tree_key->next;
		top_prefix_tree_free_key(tree,tree_key);
	}
	return err;
}

static inline top_error_t top_prefix_tree_ctx_split_key(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree,void* data,void** pold_data);
static inline top_error_t top_prefix_tree_ctx_split_new_key(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree,void* data,void** pold_data);


void * top_prefix_tree_simple_find(struct top_prefix_tree* tree, const char* key)
{
    if(tree->root == 0 || 0 == key || *key == 0) return 0;
    struct top_prefix_tree_match_ctx ctx;
    memset(&ctx,0,sizeof(ctx));
    ctx->pparent = &tree->root;
    ctx->key = key;
    return top_prefix_tree_ctx_find(&ctx);
}

top_error_t top_prefix_tree_simple_insert(struct top_prefix_tree* tree,const char* key,void* data,void** pold_data)
{
    if(0 == key || *key == 0) {
        return TOP_ERROR(-1);
    }

    struct top_prefix_tree_match_ctx ctx;
    memset(&ctx,0,sizeof(ctx));
    ctx->pparent = &tree->root;
    ctx->key = key;
    return top_prefix_tree_ctx_insert(&ctx,data,pold_data);
}

