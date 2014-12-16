
#include <top/core/prefix_tree.h>
#include <stdlib.h> //for malloc/free
#include <string.h> //for memset
#include <stdio.h>

#define KEY_NODE 1

#define PREFIX_TREE_NODE_KEY( n ) ((struct top_prefix_tree_key*)((unsigned long)(n) & ~3))
#define PREFIX_TREE_NODE_SLOTS( n ) ((struct top_prefix_tree_slots*)((unsigned long)(n) & ~3))
#define PREFIX_TREE_NODE_IS_KEY( n ) ( (unsigned long)(n) & KEY_NODE )
#define PREFIX_TREE_GEN_NODE_KEY ( n ) ((struct top_prefix_tree_node*)((unsigned long)(n) | KEY_NODE))

#define PREFIX_TREE_NODE_SIZE(tree) (((tree)->conf.key_map_size + 2) * sizeof(void*))
#define PREFIX_TREE_NODE_SLOTS_KEY_SIZE (sizeof(void*) - 1)
#define PREFIX_TREE_NODE_SLOTS_HAS_KEY(slots) (slots->flags & KEY_NODE)

/**
  * 字符集大小设置为64，任何字符可以通过BASE64编码进行保存
  * 当然也可以直接限制用户输入字符集合，忽略大小写，可能更少
  * 也就是对于输入字符可能存在一个预处理过程，
  * 如果是prefix_tree提供回调机制来实现预处理，一个函数指针可能太重量级
 * 放到外部处理，外部需要遍历实现预处理，内部查询插入等需要再次遍历
 * 考虑到prefix tree更多为查询使用，查询就是一个key值应该是被重复使用的
 * 因此放到外部实现一次预处理应该更合适一些。另外，对于用户输入的，可以
 * 在输入界面上就保证用户输入的内容满足规范要求；对于HTTP URL等的查询处理
 * 需要每次实现URL的BASE64编码，两难?????
 *
 * 当前选择内部不提供预处理机制.
 *
  */
#define PREFIX_TREE_NODE_KEY_MIN_SIZE() (2 * sizeof(void*))
#define PREFIX_TREE_NODE_KEY_MAX_SIZE(tree) (tree->node_size - sizeof(void*))

#define PREFIX_TREE_NODE_KEY_EOF_DATA 255
#define PREFIX_TREE_NODE_KEY_EOF 254
#define PREFIX_TREE_NODE_KEY_EOF_KEY 253
#define PREFIX_TREE_NODE_KEY_IS_EOF(c) ( (c) >= PREFIX_TREE_NODE_KEY_EOF )
#define PREFIX_TREE_NODE_KEY_IS_EOF_DATA(c) ((c) == PREFIX_TREE_NODE_KEY_EOF_DATA )
#define PREFIX_TREE_NODE_KEY_IS_EOF_KEY(c) ((c) == PREFIX_TREE_NODE_KEY_EOF_KEY )


#define PREFIX_TREE_PAGE_SIZE(tree) (8 * 1024)

void top_prefix_slot_map_init(top_prefix_tree_slot_map slot_map, top_prefix_tree_key_map_const key_map, unsigned int map_size)
{
    int i;
    for(i = 0; i < 256; ++i)
        slot_map[i] = 255;
    for(i = 0; i < map_size; ++i)
        slot_map[key_map[i]] = i;
}

/**
  * KEY的长度是不确定的，为了简化内存管理，key设计为一个链表结构
  * 分配策略是先分配slots大小，然后按需占用最小空间，保证按照sizeof(void*)边界对齐,
  * 这样的问题是剩下空间的利用策略，可以将剩余大小的值写在key[0]，简单按序分配使用，
  * 这样可能某个key会使用到几个极小空间的内容串接起来，对于查询性能有些影响
  * 链表结构在删除时需要合并节点有很大的便利之处，只需要将KEY节点链接起来即可完成.
  */

/**
  * 最后的空间是在父节点的指针低位指示是本节点是key还是slots结构，
  * 如果是key结构，叶子节点中，key的结束字符是255,NEXT指针指向data数据
  * 如果是key结构，内部节点中，key的结束字符是0，NEXT指针指向一个slots数据
  * 这种方式下可以最大节省叶子节点和无key的内部节点的空间
  */
struct top_prefix_tree_key {
    void* next;
    union {
        unsigned char size;
        unsigned char key[1];
    };
};


/**
  * flags: 0-7
  * 0-5: slots占用情况的计数器，在删除节点的时候决定是否需要进行节点合并
  * 7: 指示key中是否保存key值，在key比较小的时候减少空间占用.
  * key: 对于中间节点，如果key的长度在[1,(sizeof(void*) - 1)] 的范围内可以使用它，而无需申请独立的top_prefix_tree_key结构.
  */
struct top_prefix_tree_slots {
    union {
		struct {
			struct top_prefix_tree_slots* next;
		};
        struct {
            unsigned char flags;
            unsigned char key[PREFIX_TREE_NODE_SLOTS_KEY_SIZE];
            void* data;
            unsigned long slots[1];
        };
        struct {
            unsigned long avail_num;
            struct top_prefix_tree_slots* next_page;
        };
    };
};

enum top_prefix_match_result {
    MATCH_ALL = 0,
    MATCH_NEXT , /** 本节点匹配成功，还需要继续匹配下个节点 */
    MATCH_PREFIX, /** 本节点部分匹配成功，如果是插入操作，需要分裂当前节点 */
    MATCH_PARTIAL, /** 存在不匹配的字符，如果是插入操作，需要生成新的节点 */
	MATCH_INTRP, /** 匹配结束，到达叶子节点了 */
};

struct top_prefix_tree_ctx {
	struct top_prefix_tree* tree;
    unsigned long* pself; /** 低位会记录当前节点的状态，是key struct还是slots struct */
    struct top_prefix_tree_key* current; /** 当前节点，因为内部是链表结构，这可能不是首节点 */
    struct top_prefix_tree_slots* slots; /** slots表示当前已经搜索到 slots结构了，这样的话，current是最后一个key节点 */
    const unsigned char* key; /** 待匹配key */
    unsigned char matched_size; /** 当前key中成功匹配的数量 */
    unsigned char end_pos;
};

static inline void top_prefix_tree_free_bulk(struct top_prefix_tree* tree)
{
    struct top_prefix_tree_slots* slots ;
    while(tree->bulk_alloc) {
        slots = tree->bulk_alloc;
        tree->bulk_alloc = slots->next_page;
        tree->conf.pffree(tree->conf.user_data,slots,PREFIX_TREE_PAGE_SIZE(tree));
    }
}

static inline void top_prefix_tree_free_slots(struct top_prefix_tree* tree, struct top_prefix_tree_slots* slots)
{
    slots->next = tree->cached_slots;
    tree->cached_slots = slots;
}

static inline top_error_t top_prefix_tree_alloc_slots(struct top_prefix_tree* tree, struct top_prefix_tree_slots** pslots)
{
    if(tree->cached_slots) {
        *pslots = tree->cached_slots;
        tree->cached_slots = (struct top_prefix_tree_slots*)(*pslots)->next;
        memset(*pslots,0,sizeof(**pslots));
        return TOP_OK;
    }

    if(tree->bulk_alloc && tree->bulk_alloc->avail_num) {
        *pslots = (struct top_prefix_tree_slots*)((char*)tree->bulk_alloc + tree->bulk_alloc->avail_num * tree->node_size);
        --tree->bulk_alloc->avail_num;
        memset(*pslots,0,sizeof(**pslots));
        return TOP_OK;
    }

	if(tree->conf.max_capacity - PREFIX_TREE_PAGE_SIZE(tree) < tree->capacity) {
		return TOP_ERROR(-1);
	}
    top_error_t err;
    struct top_prefix_tree_slots* slots;
    err = tree->conf.pfmalloc(tree->conf.user_data,PREFIX_TREE_PAGE_SIZE(tree),(void**)&slots);
    if(top_errno(err)) return err;

	tree->capacity += PREFIX_TREE_PAGE_SIZE(tree);
    slots->avail_num = (PREFIX_TREE_PAGE_SIZE(tree) / tree->node_size - 2);
    slots->next = tree->bulk_alloc;
    tree->bulk_alloc = slots;

    *pslots = (struct top_prefix_tree_slots*)((char*)slots + (slots->avail_num + 1) * tree->node_size);
    memset(*pslots,0,tree->node_size);
    return TOP_OK;
}

static inline void top_prefix_tree_free_key(struct top_prefix_tree* tree, struct top_prefix_tree_key* key)
{
    key->next = tree->cached_key;
    tree->cached_key = key;
}

static inline top_error_t top_prefix_tree_alloc_key(struct top_prefix_tree* tree, struct top_prefix_tree_key** pkey)
{
    if(tree->cached_key) {
        *pkey = tree->cached_key;
        tree->cached_key = (struct top_prefix_tree_key*)((*pkey)->next);
		(*pkey)->next = 0;
        return TOP_OK;
    }

    struct top_prefix_tree_key* key;
    top_error_t err;
    err = top_prefix_tree_alloc_slots(tree,(struct top_prefix_tree_slots**)pkey);
    if(top_errno(err)) return err;

    (*pkey)->size = PREFIX_TREE_NODE_KEY_MAX_SIZE(tree);
	(*pkey)->next = 0;
    return TOP_OK;
}


static inline void* top_prefix_tree_slots_get_data(struct top_prefix_tree_slots* slots);
static inline void* top_prefix_tree_slots_set_data(struct top_prefix_tree_slots* slots,void* data);
static inline void* top_prefix_tree_ctx_get_data(struct top_prefix_tree_ctx* ctx);
static inline void* top_prefix_tree_ctx_set_data(struct top_prefix_tree_ctx* ctx,void* data);
static inline void top_prefix_tree_ctx_move_to_next(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree_ctx* rollback);
static inline unsigned long top_prefix_tree_slots_get_slot(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c);
static inline unsigned long* top_prefix_tree_slots_get_slot_addr(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c);
static inline enum top_prefix_match_result top_prefix_tree_key_match(struct top_prefix_tree_ctx* ctx);
static inline enum top_prefix_match_result top_prefix_tree_slots_match(struct top_prefix_tree_ctx* ctx);


static inline void* top_prefix_tree_ctx_find(struct top_prefix_tree_ctx* ctx)
{
    int has_compress_key;
    struct top_prefix_tree_key* tree_key;
    struct top_prefix_tree_slots* tree_slots;
	enum top_prefix_match_result rlt;
    do {
        has_compress_key = PREFIX_TREE_NODE_IS_KEY(*ctx->pself);
        if(has_compress_key) {
            ctx->current = PREFIX_TREE_NODE_KEY(*ctx->pself);
            if(ctx->current == 0) return 0;
            rlt = top_prefix_tree_key_match(ctx);
        } else {
            ctx->slots = PREFIX_TREE_NODE_SLOTS(*ctx->pself);
            if(ctx->slots == 0) return 0;
            rlt = top_prefix_tree_slots_match(ctx);
        }
        switch(rlt) {
        case MATCH_ALL:
            return top_prefix_tree_ctx_get_data(ctx);
        case MATCH_NEXT:
            top_prefix_tree_ctx_move_to_next(ctx,0);
            break;
		case MATCH_INTRP:
        case MATCH_PREFIX:
        case MATCH_PARTIAL:
        default:
            return 0;
        }
    } while(1);
}

static inline void top_prefix_tree_ctx_move_to_next(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree_ctx* rollback)
{
	assert(ctx->slots || ctx->current);
    if(ctx->slots == 0) {
        ctx->slots = (struct top_prefix_tree_slots*)ctx->current->next;
        assert(ctx->slots);
    }
	if(rollback) {
		rollback->pself = ctx->pself;
		rollback->slots = ctx->slots;
		rollback->current = ctx->current;
	}
    ctx->pself = top_prefix_tree_slots_get_slot_addr(ctx->tree,ctx->slots,*(ctx->key++));
    ctx->current = 0;
    ctx->slots = 0;
}

static inline void top_prefix_tree_ctx_rollback(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree_ctx* rollback)
{
	ctx->pself = rollback->pself;
	ctx->current = rollback->current;
	ctx->slots = rollback->slots;
}

static inline void* top_prefix_tree_slots_set_data(struct top_prefix_tree_slots* slots,void* data)
{
    void* old = slots->data;
    slots->data = data;
    return old;
}

static inline void* top_prefix_tree_ctx_get_data(struct top_prefix_tree_ctx* ctx)
{
    assert(ctx->current || ctx->slots);
    if(ctx->slots) {
        return top_prefix_tree_slots_get_data(ctx->slots);
    } else if(PREFIX_TREE_NODE_KEY_IS_EOF_DATA(ctx->current->key[ctx->end_pos])) {
        return ctx->current->next;
    } else {
		assert(0);
    }
}

static inline void top_prefix_tree_ctx_set_data(struct top_prefix_tree_ctx* ctx,void* data,void** pold_data)
{
    assert(ctx->current || ctx->slots);
    if(ctx->slots) {
		if(pold_data) *pold_data = ctx->slots->data;
		ctx->slots->data = data;
    } else if(PREFIX_TREE_NODE_KEY_IS_EOF_DATA(ctx->current->key[ctx->end_pos])) {
		if(pold_data) *pold_data = ctx->current->next;
		ctx->current->next = data;
    } else {
		assert(0);
    }
}

static inline unsigned long top_prefix_tree_slots_get_slot(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c)
{
    unsigned int slot_idx = (*tree->conf.slot_map)[c];
    assert(slot_idx < tree->conf.key_map_size);
    return slots->slots[slot_idx];
}

static inline unsigned long* top_prefix_tree_slots_get_slot_addr(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c)
{
    unsigned int slot_idx = (*tree->conf.slot_map)[c];
    assert(slot_idx < tree->conf.key_map_size);
    return &slots->slots[slot_idx];
}

static inline enum top_prefix_match_result top_prefix_tree_key_match(struct top_prefix_tree_ctx* ctx) {
    struct top_prefix_tree_key* tree_key = ctx->current;
    assert(tree_key);
    unsigned int i;
    do {
        for(i = 0; i < PREFIX_TREE_NODE_KEY_MAX_SIZE(ctx->tree) ; ++i)
        {
            assert(ctx->key[i] < 128);
            if(tree_key->key[i] == ctx->key[i]) {
                continue;
            }
            switch(tree_key->key[i]) {
            case PREFIX_TREE_NODE_KEY_EOF:
                ctx->current = tree_key;
                ctx->matched_size = i;
                ctx->key += i;
                ctx->slots = (struct top_prefix_tree_slots*)tree_key->next;
                return top_prefix_tree_slots_match(ctx);
            case PREFIX_TREE_NODE_KEY_EOF_DATA:
                ctx->current = tree_key;
                ctx->matched_size = i;
                ctx->key += i;
                return *ctx->key == 0 ? MATCH_ALL : MATCH_INTRP;
            case PREFIX_TREE_NODE_KEY_EOF_KEY:
                goto cont;
            default:
                ctx->current = tree_key;
                ctx->matched_size = i;
                ctx->key += i;
                return ctx->key[i] == 0 ? MATCH_PREFIX : MATCH_PARTIAL;
            }
        }
cont:
        tree_key = tree_key->next;
        ctx->key += i;
        assert(tree_key);
    } while(1);
}

static inline enum top_prefix_match_result top_prefix_tree_slots_match(struct top_prefix_tree_ctx* ctx) {
    struct top_prefix_tree_slots* tree_slots = ctx->slots;
    assert(tree_slots);
    if(PREFIX_TREE_NODE_SLOTS_HAS_KEY(tree_slots))
    {
        int i;
        for(i = 0; i < PREFIX_TREE_NODE_SLOTS_KEY_SIZE; ++i) {
            if(tree_slots->key[i] != ctx->key[i]) {
                ctx->key += i;
                ctx->matched_size = i;
                return *ctx->key == 0 ? MATCH_PREFIX : MATCH_PARTIAL;
            }
            if(tree_slots->key[i] == 0) {
                ctx->matched_size = i;
                return MATCH_ALL;
            }
        }
        ctx->matched_size = i;
        ctx->key += i;
    }
    return *ctx->key == 0 ? MATCH_ALL : MATCH_NEXT;
}

void * top_prefix_tree_simple_find(struct top_prefix_tree* tree, const char* key)
{
    if(tree->root == 0 || 0 == key || *key == 0) return 0;
    struct top_prefix_tree_ctx ctx;
    memset(&ctx,0,sizeof(ctx));
	ctx.tree = tree;
    ctx.pself = &tree->root;
    ctx.key = key;
    return top_prefix_tree_ctx_find(&ctx);
}

static inline top_error_t top_prefix_tree_ctx_new_key(struct top_prefix_tree_ctx* ctx,void* data,void** pold_data);
static inline top_error_t top_prefix_tree_ctx_split_key(struct top_prefix_tree_ctx* ctx,void* data,void** pold_data);

static inline top_error_t top_prefix_tree_ctx_insert(struct top_prefix_tree_ctx* ctx,void* data,void** pold_data)
{
    int has_compress_key;
    struct top_prefix_tree_key* tree_key;
    struct top_prefix_tree_slots* tree_slots;
	enum top_prefix_match_result rlt;
	top_error_t err;
    do {
        has_compress_key = PREFIX_TREE_NODE_IS_KEY(*ctx->pself);
        if(has_compress_key) {
            ctx->current = PREFIX_TREE_NODE_KEY(*ctx->pself);
			assert(ctx->current);
            rlt = top_prefix_tree_key_match(ctx);
        } else {
            ctx->slots = PREFIX_TREE_NODE_SLOTS(*ctx->pself);
            if(ctx->slots == 0) {
				err = top_prefix_ctx_new_key(ctx,data);
				goto fail;
			}
            rlt = top_prefix_tree_slots_match(ctx);
        }
        switch(rlt) {
        case MATCH_ALL:
            top_prefix_tree_ctx_set_data(ctx,data,pold_data);
			return TOP_OK;
        case MATCH_NEXT:
            top_prefix_tree_ctx_move_to_next(ctx,0);
            break;
		case MATCH_INTRP:
			err = top_prefix_tree_new_slots(ctx,data);
			goto fail;
        case MATCH_PREFIX:
        case MATCH_PARTIAL:
			{
			err = top_prefix_tree_ctx_split_key(ctx,data);
			goto fail;
			}
        default:
			assert(0);
        }
    } while(1);
fail:
	if(!top_errno(err) && pold_data) {
		*pold_data = 0;
	}
	return err;
}

static inline top_error_t top_prefix_tree_ctx_new_key(struct top_prefix_tree_ctx* ctx,void* data);
{
    struct top_prefix_tree_key* tree_key,*next_key;
    const char* key = ctx->key;
	unsigned char size;
    top_error_t err;
    err = top_prefix_tree_alloc_key(ctx->tree,&tree_key);
    if(top_errno(err)) return err;
    *ctx->pself = PREFIX_TREE_GEN_NODE_KEY(tree_key);
	size = tree_key->size;
    strncpy(tree_key->key,key,size);
    while(tree_key->key[size - 1] != 0) {
        err = top_prefix_tree_alloc_key(tree,&next_key);
        if(top_errno(err)) goto fail;
		tree_key->key[size - 1] = PREFIX_TREE_NODE_KEY_EOF_KEY;
        key += size;
		size = next_key->size;
        strncpy(next_key->key,key,size);
        tree_key->next = next_key;
        tree_key = next_key;
    }
	tree_key->key[size - 1] = PREFIX_TREE_NODE_KEY_EOF_DATA;
	tree_key->next = data;
	err = TOP_OK;
out:
    return err;
fail:
    tree_key = *ctx->pself;
    *ctx->pself = 0;
    while(tree_key) {
        next_key = tree_key->next;
        top_prefix_tree_free_key(tree,tree_key);
    }
	goto out;
}

static inline top_error_t top_prefix_tree_ctx_new_slots(struct top_prefix_tree_ctx* ctx,void* data)
{
	top_error_t err;
	struct top_prefix_tree_slots* slots;
	struct top_prefix_tree_ctx rollback;
	struct top_prefix_tree_key* current;
	unsigned long* pself_old;
	err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
	if(top_errno(err)) return err;
	current->key[ctx->end_pos] = PREFIX_TREE_NODE_KEY_EOF;
	slots->data = ctx->current->next;
	current->next = slots;
	ctx->slots = slots;
	top_prefix_tree_ctx_move_to_next(ctx,&rollback);
	err = top_prefix_tree_ctx_new_key(ctx,data);
	if(top_errno(err)) {
		current->next = slots->data;
		current->key[ctx->end_pos] = PREFIX_TREE_NODE_KEY_EOF_DATA;
		top_prefix_tree_ctx_rollback(ctx,&rollback);
		ctx->slots = 0;
		top_prefix_tree_free_slots(ctx->tree,slots);
	}
	return err;
}

static inline top_error_t top_prefix_tree_ctx_split_key(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree,void* data,void** pold_data)
{
	top_error_t err;
	struct top_prefix_tree_key* new_key;
	struct top_prefix_tree_slots* slots;
	err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
	if(top_errno(err)) return err;

	if(ctx->matched_size) {
	}else {
	}
}

top_error_t top_prefix_tree_simple_insert(struct top_prefix_tree* tree,const char* key,void* data,void** pold_data)
{
    if(0 == key || *key == 0) {
        return TOP_ERROR(-1);
    }
    struct top_prefix_tree_ctx ctx;
    memset(&ctx,0,sizeof(ctx));
	ctx.tree = tree;
    ctx.pself = &tree->root;
    ctx.key = key;
    return top_prefix_tree_ctx_insert(&ctx,data,pold_data);
}

