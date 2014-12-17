
#include <top/core/prefix_tree.h>
#include <stdlib.h> //for malloc/free
#include <string.h> //for memset
#include <stdio.h>
#include <assert.h>

#define KEY_NODE 1ul

#define PREFIX_TREE_NODE_KEY( n ) ((struct top_prefix_tree_key*)((unsigned long)(n) & ~3))
#define PREFIX_TREE_NODE_SLOTS( n ) ((struct top_prefix_tree_slots*)((unsigned long)(n) & ~3))
#define PREFIX_TREE_NODE_IS_KEY( n ) ( (unsigned long)(n) & KEY_NODE )
#define PREFIX_TREE_GEN_NODE_KEY( n ) (((unsigned long)(n) | KEY_NODE))

#define PREFIX_TREE_NODE_SIZE(tree) (((tree)->conf.key_map_size + 2) * sizeof(void*))
#define PREFIX_TREE_NODE_SLOTS_KEY_SIZE (sizeof(void*) - 1)

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
#define PREFIX_TREE_NODE_KEY_SIZE( key_len ) ( ((key_len) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))
#define PREFIX_TREE_NODE_KEY_NEXT( key,key_len) ((struct top_prefix_tree_key*)((char*)(key) + sizeof(void*) + PREFIX_TREE_NODE_KEY_SIZE(key_len)))

#define PREFIX_TREE_NODE_KEY_EOF_DATA 255
#define PREFIX_TREE_NODE_KEY_EOF 254
#define PREFIX_TREE_NODE_KEY_EOF_KEY 253
#define PREFIX_TREE_NODE_KEY_IS_EOF(c) ( (c) >= PREFIX_TREE_NODE_KEY_EOF )
#define PREFIX_TREE_NODE_KEY_IS_EOF_DATA(c) ((c) == PREFIX_TREE_NODE_KEY_EOF_DATA )
#define PREFIX_TREE_NODE_KEY_IS_EOF_KEY(c) ((c) == PREFIX_TREE_NODE_KEY_EOF_KEY )


#define PREFIX_TREE_PAGE_SIZE(tree) (8 * 1024)

static const unsigned char g_prefix_tree_key_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char g_prefix_tree_slot_map[256] = {
    255,255,255,255,255,255,255,255,255,255, // 0-9
    255,255,255,255,255,255,255,255,255,255, // 10-19
    255,255,255,255,255,255,255,255,255,255, // 20-29
    255,255,255,255,255,255,255,255,255,255, // 30-39
    255,255,255,62 ,255,255,255,63 ,52 ,53 , // 40-49
    54 ,55 ,56 ,57 ,58 ,59 ,60 ,61 ,255,255, // 50-59
    255,255,255,255,255,0  ,1  ,  2,  3,  4, // 60-69
    5  ,6  ,7  ,8  ,9  ,10 , 11, 12, 13, 14, // 70-79
    15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,24 , // 80-89
    25 ,255,255,255,255,255,255, 26, 27, 28, // 90-99
    29 ,30 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,38 , //100- 109
    39 ,40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,48 , //110 -119
    49 ,50 ,51 ,255,255,255,255,255,255,255, //120 - 129
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255
};

void top_prefix_slot_map_init(top_prefix_tree_slot_map slot_map, top_prefix_tree_key_map_const key_map, unsigned int map_size)
{
    int i;
    for(i = 0; i < 255; ++i)
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
        unsigned short size;
        unsigned char key[2];
    };
};


/**
  * counter: slots占用情况的计数器，在删除节点的时候决定是否需要进行节点合并
  * key: 对于中间节点，如果key的长度在[1,(sizeof(void*) - 1)] 的范围内可以使用它，而无需申请独立的top_prefix_tree_key结构,查询时候达到最大长度或者以0表示结束.
  */
struct top_prefix_tree_slots {
    union {
        struct {
            struct top_prefix_tree_slots* next;
        };
        struct {
            unsigned char counter;
            unsigned char key[PREFIX_TREE_NODE_SLOTS_KEY_SIZE];
            void* data;
            unsigned long slots[1];
        };
        struct {
            unsigned long avail_num;
            struct top_prefix_tree_slots* bulk_next;
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
    struct top_prefix_tree_key* prev; /** 当前节点的上一级，因为内部是链表结构,插入的时候需要它修改链表结构 */
    struct top_prefix_tree_slots* slots; /** slots表示当前已经搜索到 slots结构了，这样的话，current是最后一个key节点 */
    const unsigned char* key; /** 待匹配key */
    unsigned short matched_size; /** 当前key中成功匹配的数量 */
    unsigned short prev_end_pos;
};

static inline void top_prefix_tree_free_bulk(struct top_prefix_tree* tree)
{
    struct top_prefix_tree_slots* slots ;
    while(tree->bulk_alloc) {
        slots = tree->bulk_alloc;
        tree->bulk_alloc = slots->bulk_next;
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
	printf("\nalloc_slots: %p,cached:%p,bulk: %p\n",tree,tree->cached_slots,tree->bulk_alloc);
    if(tree->cached_slots) {
        *pslots = tree->cached_slots;
        tree->cached_slots = (struct top_prefix_tree_slots*)(*pslots)->next;
        memset(*pslots,0,sizeof(**pslots));
        return TOP_OK;
    }

    if(tree->bulk_alloc && tree->bulk_alloc->avail_num) {
        *pslots = (struct top_prefix_tree_slots*)((char*)tree->bulk_alloc + tree->bulk_alloc->avail_num * tree->node_size);
        --tree->bulk_alloc->avail_num;
		printf("\navail_num: %d\n",tree->bulk_alloc->avail_num);
        memset(*pslots,0,tree->node_size);
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
    slots->bulk_next = tree->bulk_alloc;
    tree->bulk_alloc = slots;
	printf("\nnew bulk: %p,%d,%d\n",slots,slots->avail_num,tree->node_size);

    *pslots = (struct top_prefix_tree_slots*)((char*)slots + (slots->avail_num + 1) * tree->node_size);
    memset(*pslots,0,tree->node_size);
    return TOP_OK;
}

static inline void top_prefix_tree_free_key(struct top_prefix_tree* tree, struct top_prefix_tree_key* key,unsigned short key_buf_size)
{
	printf("\nfree_key: %d\n",key_buf_size);
    assert(0 == (key_buf_size & (sizeof(void*) - 1)));
    key->size = key_buf_size;
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

    (*pkey)->size = tree->max_key_size;
    (*pkey)->next = 0;
    return TOP_OK;
}


static inline void* top_prefix_tree_ctx_get_data(struct top_prefix_tree_ctx* ctx);
static inline void top_prefix_tree_ctx_set_data(struct top_prefix_tree_ctx* ctx,void* data,void** pold_data);
static inline void top_prefix_tree_ctx_move_to_next(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree_ctx* rollback);
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
            assert(ctx->current);
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
    assert(ctx->slots);
    assert(0 == ctx->current);
    if(rollback) {
        rollback->pself = ctx->pself;
        rollback->slots = ctx->slots;
        rollback->prev = ctx->prev;
        rollback->prev_end_pos = ctx->prev_end_pos;
    }
    ctx->pself = top_prefix_tree_slots_get_slot_addr(ctx->tree,ctx->slots,*(ctx->key++));
    ctx->slots = 0;
    ctx->prev = 0;
    ctx->prev_end_pos = 0;
}

static inline void top_prefix_tree_ctx_rollback(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree_ctx* rollback)
{
    ctx->pself = rollback->pself;
    ctx->slots = rollback->slots;
    ctx->prev = rollback->prev;
    ctx->prev_end_pos = rollback->prev_end_pos;
    ctx->current = 0;
}

static inline void* top_prefix_tree_ctx_get_data(struct top_prefix_tree_ctx* ctx)
{
    assert(ctx->current || ctx->slots);
    if(ctx->slots) {
        return ctx->slots->data;
    } else {
        assert(PREFIX_TREE_NODE_KEY_EOF_DATA == ctx->current->key[ctx->matched_size]);
        return ctx->current->next;
    }
}

static inline void top_prefix_tree_ctx_set_data(struct top_prefix_tree_ctx* ctx,void* data,void** pold_data)
{
    assert(ctx->current || ctx->slots);
    if(ctx->slots) {
        if(pold_data) *pold_data = ctx->slots->data;
        ctx->slots->data = data;
    } else {
        assert(PREFIX_TREE_NODE_KEY_EOF_DATA == ctx->current->key[ctx->matched_size]);
        if(pold_data) *pold_data = ctx->current->next;
        ctx->current->next = data;
    }
}

static inline void top_prefix_tree_slots_link_slots(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c,struct top_prefix_tree_slots* sub_slots)
{
    unsigned int slot_idx = tree->conf.slot_map[c];
    slots->slots[slot_idx] = (unsigned long)sub_slots;
}

static inline void top_prefix_tree_slots_link_key(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c,struct top_prefix_tree_key* key)
{
    unsigned int slot_idx = tree->conf.slot_map[c];
    slots->slots[slot_idx] = PREFIX_TREE_GEN_NODE_KEY(key);
}

static inline unsigned long* top_prefix_tree_slots_get_slot_addr(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c)
{
    unsigned int slot_idx = (tree->conf.slot_map)[c];
    assert(slot_idx < tree->conf.key_map_size);
    return &slots->slots[slot_idx];
}

static inline enum top_prefix_match_result top_prefix_tree_key_match(struct top_prefix_tree_ctx* ctx) {
    struct top_prefix_tree_key* tree_key = ctx->current;
    assert(tree_key);
    unsigned int i;
    do {
		printf("\n%s, key match: max_size: %d\n",__func__,ctx->tree->max_key_size);
        for(i = 0; i < ctx->tree->max_key_size; ++i)
        {
			printf("\ncompare: %c == %c, at %d\n",(char)tree_key->key[i],(char)ctx->key[i],i);
            if(tree_key->key[i] == ctx->key[i]) {
				assert(ctx->key[i] < 128);
                continue;
            }
            switch(tree_key->key[i]) {
            case PREFIX_TREE_NODE_KEY_EOF:
                ctx->prev = tree_key;
                ctx->prev_end_pos = i;
                ctx->current = 0;
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
				printf("\n%s, key not match\n",__func__);
                return ctx->key[i] == 0 ? MATCH_PREFIX : MATCH_PARTIAL;
            }
        }
        assert(0);
cont:
        ctx->prev = tree_key;
        ctx->prev_end_pos = i;
        tree_key = tree_key->next;
        ctx->key += i;
        assert(tree_key);
    } while(1);
}

static inline enum top_prefix_match_result top_prefix_tree_slots_match(struct top_prefix_tree_ctx* ctx) {
    struct top_prefix_tree_slots* tree_slots = ctx->slots;
    assert(tree_slots);
    int i;
    for(i = 0; i < PREFIX_TREE_NODE_SLOTS_KEY_SIZE; ++i)
    {
        if(tree_slots->key[i] != ctx->key[i]) {
            ctx->key += i;
            ctx->matched_size = i;
            return *ctx->key == 0 ? MATCH_PREFIX : MATCH_PARTIAL;
        }
        if(tree_slots->key[i] == 0) {
			ctx->key += i;
            ctx->matched_size = i;
            return MATCH_ALL;
        }
    }
    ctx->matched_size = i;
    ctx->key += i;
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

static inline top_error_t top_prefix_tree_ctx_new_key(struct top_prefix_tree_ctx* ctx,void* data);
static inline top_error_t top_prefix_tree_ctx_split_node(struct top_prefix_tree_ctx* ctx,void* data);
static inline top_error_t top_prefix_tree_ctx_insert_data(struct top_prefix_tree_ctx* ctx,void* data);
static inline top_error_t top_prefix_tree_ctx_append_slots(struct top_prefix_tree_ctx* ctx,void* data);

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
                err = top_prefix_tree_ctx_new_key(ctx,data);
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
            err = top_prefix_tree_ctx_append_slots(ctx,data);
            goto fail;
        case MATCH_PREFIX:
            err = top_prefix_tree_ctx_insert_data(ctx,data);
            goto fail;
        case MATCH_PARTIAL:
            err = top_prefix_tree_ctx_split_node(ctx,data);
            goto fail;
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

static inline unsigned short top_prefix_tree_key_get_size(const unsigned char* key)
{
    unsigned short size = 0;
    while(*key < PREFIX_TREE_NODE_KEY_EOF_KEY) ++size;
    ++size;
    return PREFIX_TREE_NODE_KEY_SIZE(size);
}

static inline unsigned short top_prefix_tree_strncpy(unsigned char* dest,const unsigned char* src,unsigned short size)
{
    unsigned short i ;
    for(i = 0; i < size; ++i) {
        if(src[i] == 0) {
            return i;
        }
        dest[i] = src[i];
    }
    return i;
}

static inline void top_prefix_tree_key_reclaim(struct top_prefix_tree* tree,struct top_prefix_tree_key* tree_key,unsigned int old_key_len,unsigned int new_key_len)
{
    old_key_len = PREFIX_TREE_NODE_KEY_SIZE(old_key_len);
    new_key_len = PREFIX_TREE_NODE_KEY_SIZE(new_key_len);
    if(old_key_len - new_key_len >= PREFIX_TREE_NODE_KEY_MIN_SIZE() + sizeof(void*)) {
        tree_key = (struct top_prefix_tree_key*)((char*)tree_key + new_key_len + sizeof(void*));
        top_prefix_tree_free_key(tree,tree_key,old_key_len - new_key_len - sizeof(void*));
    }
}

static inline top_error_t top_prefix_tree_new_leaf(struct top_prefix_tree* tree,const char* key,void* data,struct top_prefix_tree_key** pkey)
{
    struct top_prefix_tree_key* new_key,*tree_key,*next_key;
    unsigned short size,copied_size;
    top_error_t err;
    err = top_prefix_tree_alloc_key(tree,&tree_key);
    if(top_errno(err)) return err;
	new_key = tree_key;
    size = tree_key->size;
    copied_size = top_prefix_tree_strncpy(tree_key->key,key,size);
	printf("\nnew key: %d,key: %s,copied: %d\n",size,key,copied_size);
    while(copied_size == size) {
        err = top_prefix_tree_alloc_key(tree,&next_key);
        if(top_errno(err)) goto fail;
        --size;
        tree_key->key[size] = PREFIX_TREE_NODE_KEY_EOF_KEY;
        key += size;
        size = next_key->size;
        copied_size = top_prefix_tree_strncpy(next_key->key,key,size);
        tree_key->next = next_key;
        tree_key = next_key;
		printf("\nnew next key: %d\n",size);
    }
    tree_key->next = data;
    tree_key->key[copied_size] = PREFIX_TREE_NODE_KEY_EOF_DATA;
    top_prefix_tree_key_reclaim(tree,tree_key,size,copied_size + 1);
	*pkey = new_key;
    err = TOP_OK;
out:
    return err;
fail:
    while(new_key) {
        next_key = new_key->next;
        top_prefix_tree_free_key(tree,new_key,top_prefix_tree_key_get_size(new_key->key));
		new_key = next_key;
    }
    goto out;
}

static inline top_error_t top_prefix_tree_ctx_new_key(struct top_prefix_tree_ctx* ctx,void* data)
{
	struct top_prefix_tree_key* key;
	top_error_t err;
	err = top_prefix_tree_new_leaf(ctx->tree,ctx->key,data,&key);
	if(top_errno(err) == 0) {
		*ctx->pself = PREFIX_TREE_GEN_NODE_KEY(key);
	}
	return err;
}

static inline top_error_t top_prefix_tree_ctx_append_slots(struct top_prefix_tree_ctx* ctx,void* data)
{
    top_error_t err;
    struct top_prefix_tree_slots* slots;
    struct top_prefix_tree_ctx rollback;
    struct top_prefix_tree_key* current;
    unsigned long* pself_old;
    err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
    if(top_errno(err)) return err;
    assert(current->key[ctx->matched_size] == PREFIX_TREE_NODE_KEY_EOF_DATA);
    current->key[ctx->matched_size] = PREFIX_TREE_NODE_KEY_EOF;
    slots->data = ctx->current->next;
    current->next = slots;
    ctx->slots = slots;
    top_prefix_tree_ctx_move_to_next(ctx,&rollback);
    err = top_prefix_tree_ctx_new_key(ctx,data);
    if(top_errno(err)) {
        current->next = slots->data;
        current->key[ctx->matched_size] = PREFIX_TREE_NODE_KEY_EOF_DATA;
        top_prefix_tree_ctx_rollback(ctx,&rollback);
        ctx->slots = 0;
        top_prefix_tree_free_slots(ctx->tree,slots);
    }
    return err;
}


static inline void top_prefix_tree_slots_remove(struct top_prefix_tree_slots* tree_slots,unsigned int start,unsigned int count)
{
    unsigned int i;
    assert( PREFIX_TREE_NODE_SLOTS_KEY_SIZE >= count);
    for(i = start + count; i < PREFIX_TREE_NODE_SLOTS_KEY_SIZE; ++i) {
        tree_slots->key[i - count] = tree_slots->key[i];
        if(0 == tree_slots->key[i]) {
            return;
        }
    }
    tree_slots->key[PREFIX_TREE_NODE_SLOTS_KEY_SIZE - count] = 0;
}

static inline void top_prefix_tree_key_remove(struct top_prefix_tree* tree,struct top_prefix_tree_key* tree_key,unsigned int start,unsigned int count)
{
    unsigned int i;
    for(i = start + count; i < tree->max_key_size; ++i ) {
        tree_key->key[i - count] = tree_key->key[i];
        if(tree_key->key[i] >= PREFIX_TREE_NODE_KEY_EOF_KEY) {
            ++i;
            break;
        }
    }
    top_prefix_tree_key_reclaim(tree,tree_key,i,i - count);
}

static inline top_error_t top_prefix_tree_ctx_insert_slots(struct top_prefix_tree_ctx* ctx,void* data,struct top_prefix_tree_slots** pslots)
{
    top_error_t err;
    struct top_prefix_tree_slots* slots;
    err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
    if(top_errno(err)) return err;
    if(ctx->prev) {
        ctx->prev->key[ctx->prev_end_pos] = PREFIX_TREE_NODE_KEY_EOF;
        ctx->prev->next = slots;
    } else {
        *ctx->pself = (unsigned long)slots;
    }

    if(ctx->current) {
        top_prefix_tree_slots_link_key(ctx->tree,slots,ctx->current->key[0],ctx->current);
        top_prefix_tree_key_remove(ctx->tree,ctx->current,0,1);
    } else {
        assert(ctx->slots);
        assert(ctx->slots->key[0]);
        top_prefix_tree_slots_link_slots(ctx->tree,slots,ctx->slots->key[0],ctx->slots);
        top_prefix_tree_slots_remove(ctx->slots,0,1);
    }
    slots->data = data;
    *pslots = slots;
    return TOP_OK;
}

static inline top_error_t top_prefix_tree_ctx_split_slots(struct top_prefix_tree_ctx* ctx,void* data,struct top_prefix_tree_slots** pslots)
{
    top_error_t err;
    struct top_prefix_tree_slots* slots;
	printf("\n%s, 1 split_key before alloc_slots:%d \n",__func__,top_errno(err));
    assert(ctx->slots->key[ctx->matched_size]);
    assert(ctx->matched_size < PREFIX_TREE_NODE_SLOTS_KEY_SIZE);
    err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
    if(top_errno(err)) return err;
	printf("\n%s, 2 split_key before alloc_slots:%d \n",__func__,top_errno(err));

    slots->data = data;
    if(ctx->prev) {
        ctx->prev->next = slots;
    } else {
        *ctx->pself = (unsigned long)slots;
    }
	printf("\n%s, 3 split_key before alloc_slots:%d \n",__func__,top_errno(err));
    top_prefix_tree_slots_link_slots(ctx->tree,slots,ctx->slots->key[ctx->matched_size],ctx->slots);
	printf("\n%s, 4 split_key before alloc_slots:%d \n",__func__,top_errno(err));
    top_prefix_tree_slots_remove(ctx->slots,0,ctx->matched_size + 1);
    *pslots = slots;
    return TOP_OK;
}

static inline top_error_t top_prefix_tree_ctx_split_key(struct top_prefix_tree_ctx* ctx,void* data,struct top_prefix_tree_slots** pslots)
{
    top_error_t err;
    struct top_prefix_tree_key* new_key = 0,*tree_key;
    struct top_prefix_tree_slots* slots;
    err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
	printf("\n%s, split_key before alloc_slots:%d \n",__func__,top_errno(err));
    if(top_errno(err)) return err;

	printf("\n%s, split_key: %p \n",__func__,slots);
    slots->data = data;
    if(ctx->matched_size <= PREFIX_TREE_NODE_SLOTS_KEY_SIZE) {
		printf("\n%s, key in slots\n",__func__);
        memcpy(slots->key,&ctx->current->key, ctx->matched_size);
        slots->key[ctx->matched_size] = 0; /** 即便越界写也没有问题 */
        if(ctx->prev) {
            ctx->prev->next = slots;
            ctx->prev->key[ctx->prev_end_pos] = PREFIX_TREE_NODE_KEY_EOF;
        } else {
            *ctx->pself = (unsigned long)slots;
        }
    } else {
        struct top_prefix_tree_key* next_key;
        unsigned short size,copied_size;
        unsigned char* key;
        err = top_prefix_tree_alloc_key(ctx->tree,&tree_key);
        if(top_errno(err)) goto fail;
        new_key = tree_key;
        copied_size = ctx->matched_size - PREFIX_TREE_NODE_SLOTS_KEY_SIZE;
        size = new_key->size;
        key = ctx->current->key;
        while(copied_size >= size) {
            err = top_prefix_tree_alloc_key(ctx->tree,&next_key);
            if(top_errno(err)) goto fail;
            --size;
            memcpy(tree_key->key,key,size);
            tree_key->key[size] = PREFIX_TREE_NODE_KEY_EOF_KEY;
            tree_key->next = next_key;
            tree_key = next_key;
            copied_size -= size;
            key += size;
            size = next_key->size;
        }
        memcpy(tree_key->key,key,copied_size);
        tree_key->key[copied_size] = PREFIX_TREE_NODE_KEY_EOF;
        tree_key->next = slots;
        memcpy(slots->key,&ctx->current->key[ctx->matched_size - PREFIX_TREE_NODE_SLOTS_KEY_SIZE], PREFIX_TREE_NODE_SLOTS_KEY_SIZE);
        if(ctx->prev) {
            ctx->prev->next = new_key;
            assert(ctx->prev->key[ctx->prev_end_pos] == PREFIX_TREE_NODE_KEY_EOF);
        } else {
			assert(ctx->pself);
            *ctx->pself = PREFIX_TREE_GEN_NODE_KEY(new_key);
        }
    }
    char c = ctx->current->key[ctx->matched_size];
    switch(ctx->current->key[ctx->matched_size + 1]) {
    case PREFIX_TREE_NODE_KEY_EOF:
		printf("\n%s, link original node: PREFIX_TREE_NODE_KEY_EOF\n",__func__);
        top_prefix_tree_slots_link_slots(ctx->tree,slots,c,ctx->current->next);
        top_prefix_tree_free_key(ctx->tree,ctx->current,PREFIX_TREE_NODE_KEY_SIZE(ctx->matched_size+1));
        break;
    case PREFIX_TREE_NODE_KEY_EOF_KEY:
		printf("\n%s, link original node: PREFIX_TREE_NODE_KEY_EOF_KEY\n",__func__);
        top_prefix_tree_slots_link_key(ctx->tree,slots,c,ctx->current->next);
        top_prefix_tree_free_key(ctx->tree,ctx->current,PREFIX_TREE_NODE_KEY_SIZE(ctx->matched_size+1));
    default:
		printf("\n%s, link original node: PREFIX_TREE_NODE_KEY_EOF\n",__func__);
        if(((ctx->matched_size + 1) & (sizeof(void*) - 1)) == 0) {
            tree_key = PREFIX_TREE_NODE_KEY_NEXT(ctx->current,ctx->matched_size + 1 - sizeof(void*));
            top_prefix_tree_slots_link_key(ctx->tree,slots,c,tree_key);
            top_prefix_tree_free_key(ctx->tree,ctx->current,ctx->matched_size + 1 - sizeof(void*));
        } else {
            top_prefix_tree_slots_link_key(ctx->tree,slots,c,ctx->current);
            top_prefix_tree_key_remove(ctx->tree,ctx->current,0,ctx->matched_size + 1);
        }
        break;
    }
    *pslots = slots;
    return TOP_OK;
fail:
    while(new_key) {
        tree_key = new_key->next;
        top_prefix_tree_free_key(ctx->tree,new_key,top_prefix_tree_key_get_size(new_key->key));
        new_key = tree_key;
    }
    top_prefix_tree_free_slots(ctx->tree,slots);
    return err;
}

static inline top_error_t top_prefix_tree_ctx_insert_data(struct top_prefix_tree_ctx* ctx,void* data)
{
    struct top_prefix_tree_slots* slots;
    if(ctx->matched_size == 0) {
        assert(ctx->slots == 0);
        return top_prefix_tree_ctx_insert_slots(ctx,data,&slots);
    } else if(ctx->slots) {
        return top_prefix_tree_ctx_split_slots(ctx,data,&slots);
    } else {
        return top_prefix_tree_ctx_split_key(ctx,data,&slots);
    }
}

static inline top_error_t top_prefix_tree_ctx_split_node(struct top_prefix_tree_ctx* ctx,void* data)
{
    struct top_prefix_tree_slots* slots;
    struct top_prefix_tree_key* new_leaf;
    unsigned long* pself_old;
    unsigned char c = *ctx->key++;
    top_error_t err;

	printf("\n%s\n",__func__);
    pself_old = ctx->pself;
    ctx->pself = &new_leaf;
    err = top_prefix_tree_new_leaf(ctx->tree,ctx->key,data,&new_leaf);
    if(top_errno(err)) return err;
	printf("\nsplit: match_size=%d,slots: %p,current:%p,%p\n",ctx->matched_size,ctx->slots,ctx->current,ctx->current->next);
    if(ctx->matched_size == 0) {
        err = top_prefix_tree_ctx_insert_slots(ctx,0,&slots);
    } else if(ctx->slots) {
        err = top_prefix_tree_ctx_split_slots(ctx,0,&slots);
    } else {
        err = top_prefix_tree_ctx_split_key(ctx,0,&slots);
    }
    if(top_errno(err)) goto fail;
    top_prefix_tree_slots_link_key(ctx->tree,slots,c,PREFIX_TREE_NODE_KEY(new_leaf));
    return TOP_OK;
fail: {
        struct top_prefix_tree_key* tree_key;
        struct top_prefix_tree_key* next_key;
        unsigned short size;
        tree_key = PREFIX_TREE_NODE_KEY(new_leaf);
        while(tree_key) {
            size = 0;
            while(tree_key->key[size] < PREFIX_TREE_NODE_KEY_EOF_KEY) ++size;
            if(tree_key->key[size] == PREFIX_TREE_NODE_KEY_EOF_DATA) {
                top_prefix_tree_free_key(ctx->tree,tree_key,PREFIX_TREE_NODE_KEY_SIZE(++size));
                break;
            }
            next_key = tree_key->next;
            top_prefix_tree_free_key(ctx->tree,tree_key,PREFIX_TREE_NODE_KEY_SIZE(++size));
            tree_key = next_key;
        }
        return err;
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

static top_error_t top_prefix_tree_malloc(void* data, unsigned long size,void** pallocated)
{
    void* p = malloc(size);
    if(p) {
        *pallocated = p;
        return TOP_OK;
    }
    return TOP_ERROR(-1);
}

static void top_prefix_tree_free(void* data,void* allocated,unsigned long size)
{
    free(allocated);
}

void top_prefix_tree_init(struct top_prefix_tree* tree,const struct top_prefix_tree_conf* conf)
{
    struct top_prefix_tree_conf* tree_conf;
    assert(tree);
    memset(tree,0,sizeof(*tree));
    tree_conf = (struct top_prefix_tree_conf*)&tree->conf;
    if(conf) {
        *tree_conf = *conf;
    }
    if(tree_conf->pfmalloc == 0 ||tree_conf->pffree == 0) {
        tree_conf->pfmalloc = top_prefix_tree_malloc;
        tree_conf->pffree = top_prefix_tree_free;
    }
    if(tree_conf->key_map == 0 || tree_conf->slot_map == 0 || tree_conf->key_map_size == 0) {
        tree_conf->key_map = g_prefix_tree_key_map;
        tree_conf->slot_map = g_prefix_tree_slot_map;
        tree_conf->key_map_size = 64;
    }
    *(unsigned short*)&tree->node_size = PREFIX_TREE_NODE_SIZE(tree);
    *(unsigned short*)&tree->max_key_size = PREFIX_TREE_NODE_KEY_MAX_SIZE(tree);
}

void top_prefix_tree_fini(struct top_prefix_tree* tree)
{
    top_prefix_tree_free_bulk(tree);
}


void* top_prefix_tree_simple_delete(struct top_prefix_tree* tree, const char* key)
{
	return 0;
}

