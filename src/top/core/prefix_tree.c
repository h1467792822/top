
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

#define PREFIX_TREE_NODE_SLOTS_ALIGN(n) (( (n) + 1) & ~1)
#define PREFIX_TREE_NODE_SIZE(tree) ((PREFIX_TREE_NODE_SLOTS_ALIGN((tree)->conf.key_map_size) + 2) * sizeof(void*))
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
#define PREFIX_TREE_KEY_ALIGNMENT (2 * sizeof(void*))
#define PREFIX_TREE_NODE_KEY_MIN_SIZE() (sizeof(void*))
#define PREFIX_TREE_NODE_KEY_MAX_SIZE(tree) (tree->node_size - sizeof(void*))
#define PREFIX_TREE_NODE_KEY_SIZE( key_len ) ( ((key_len + sizeof(void*) + PREFIX_TREE_KEY_ALIGNMENT - 1) & ~(PREFIX_TREE_KEY_ALIGNMENT - 1)) - sizeof(void*))
#define PREFIX_TREE_NODE_KEY_NEXT( key,key_len) ((struct top_prefix_tree_key*)((char*)(key) + sizeof(void*) + PREFIX_TREE_NODE_KEY_SIZE(key_len)))

#define PREFIX_TREE_NODE_KEY_EOF_DATA 255
#define PREFIX_TREE_NODE_KEY_EOF 254
#define PREFIX_TREE_NODE_KEY_EOF_KEY 253
#define PREFIX_TREE_NODE_KEY_IS_EOF(c) ( (c) >= PREFIX_TREE_NODE_KEY_EOF )
#define PREFIX_TREE_NODE_KEY_IS_EOF_DATA(c) ((c) == PREFIX_TREE_NODE_KEY_EOF_DATA )
#define PREFIX_TREE_NODE_KEY_IS_EOF_KEY(c) ((c) == PREFIX_TREE_NODE_KEY_EOF_KEY )


#define PREFIX_TREE_VISIT_MAX_SUFFIX_LEN (2 * 1024)

#define PREFIX_TREE_PAGE_SIZE(tree) (tree->bulk_size)

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

/**
  * 比较索引而不是直接比较，这样可以较低成本支持忽略大小写等功能，而无需用户对数据进行预处理，比较成本增加较小.
  */
/**
    * 如果只想支持小写字母，但输入可能有大写，不想事先遍历进行预处理，则可以如下:
	* top_prefix_tree_slot_map_init(slot_map,"abcdefghijklmnopqrstuvwxyz",-1);
	* top_prefix_tree_slot_map_init_more(slot_map,'a',"ABCDEFGHIJKLMNOPQRSTUVWXYZ",-1);
	* 内部比较的时候则会将小写和大写字母完全等同对待.
	* 性能代价到底如何需要通过性能测试来看看
	*/
#define TOP_PREFIX_TREE_HAS_MORE_KEY_MAP 1

#if defined(TOP_PREFIX_TREE_HAS_MORE_KEY_MAP) && (TOP_PREFIX_TREE_HAS_MORE_KEY_MAP)
#define PREFIX_TREE_NOT_EQUAL(tree,c1,c2) ((tree)->conf.slot_map[c1] != (tree)->conf.slot_map[c2])
#else
#define PREFIX_TREE_NOT_EQUAL(tree,c1,c2) ((c1) != (c2))
#endif

void top_prefix_tree_slot_map_init(top_prefix_tree_slot_map slot_map, top_prefix_tree_key_map_const key_map, unsigned int map_size)
{
    int i;
    for(i = 0; i < 255; ++i)
        slot_map[i] = 255;
    for(i = 0; i < map_size && key_map[i]; ++i)
        slot_map[key_map[i]] = i;
}

/**
  * 如果支持通过索引比较key值，则应该对外暴露此函数
  */
void top_prefix_tree_slot_map_init_more(top_prefix_tree_slot_map slot_map,char equal_start_key,const char* more_key_map,unsigned int key_map_size)
{
    int i ;
    unsigned char start = slot_map[equal_start_key];
    assert(start < 255);
    for(i = 0; i < key_map_size && more_key_map[i]; ++i)
        slot_map[more_key_map[i]] = i + start;
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
  * height: 树的高度指示，有了它在遍历树的时候比较方便构建栈数据结构，栈的大小就是tree->height，最新C标准支持定义变长数组，非常方便
  *         不利之处就是height的维护有代价的，在插入删除节点的时候可能需要回溯到根节点修改路径上每个节点的height值
  *         height的主要应用场景是在删除场景，删除的时候需要合并节点，需要回溯访问父节点和祖父节点;此时用ctx中增加两个数据成员来简单解决.
  */
struct top_prefix_tree_slots {
    union {
        struct {
            struct top_prefix_tree_slots* next;
        };
        struct {
            void* data;
            unsigned char key[PREFIX_TREE_NODE_SLOTS_KEY_SIZE];
            unsigned char count;
            //unsigned short height;
            unsigned long slots[1];
        };
        struct {
            unsigned short avail_num;
            unsigned short bulk_size;
            struct top_prefix_tree_slots* bulk_next;
        };
    };
};

enum top_prefix_match_result {
    MATCH_INTRP = 0, /** 匹配结束，到达叶子节点了 ,避免MATCH_ALL成为缺省值*/
    MATCH_ALL ,
    MATCH_NEXT , /** 本节点匹配成功，还需要继续匹配下个节点 */
    MATCH_PREFIX, /** 本节点部分匹配成功，如果是插入操作，需要分裂当前节点 */
    MATCH_PARTIAL, /** 存在不匹配的字符，如果是插入操作，需要生成新的节点 */
};

struct top_prefix_tree_ctx {
    enum top_prefix_match_result rlt;
    struct top_prefix_tree* tree;
    unsigned long* pself; /** 低位会记录当前节点的状态，是key struct还是slots struct, 非根节点可以用parent->slots[self_idx]来替代使用它，保留它的原因就是要统一根节点的处理，减少空指针判断 */
    struct top_prefix_tree_key* current; /** 当前节点，因为内部是链表结构，这可能不是首节点 */
    struct top_prefix_tree_key* prev; /** 当前节点的上一级，因为内部是链表结构,插入的时候需要它修改链表结构 */
    struct top_prefix_tree_slots* slots; /** slots表示当前已经搜索到 slots结构了，这样的话，current是最后一个key节点 */
    const char* key; /** 待匹配key */
    const struct top_prefix_tree_key_vec* current_key;
    const struct top_prefix_tree_key_vec* last_key;
    unsigned short matched_size; /** 当前key中成功匹配的数量 */
    unsigned short prev_end_pos;
    unsigned short parent_prev_end_pos;
    struct top_prefix_tree_key* parent_prev; /** 和pparent配套使用，同prev和pself */
    unsigned long* pparent; /** 祖父节点中记录的父节点的信息，在删除合并节点的时候可能要用到；在插入节点的时候可能需要使用它来修改父节点中slot的计数值 */
};

static struct top_prefix_tree_key_vec null_key = {
    .key = "",
    .key_end = 0,
};

static inline void top_prefix_tree_ctx_init(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key_vec,int count)
{
    assert(count > 0);
    memset(ctx,0,sizeof(*ctx));
    ctx->tree = tree;
    ctx->pself = &tree->root;
    ctx->current_key = key_vec;
    ctx->key = ctx->current_key->key;
    ctx->last_key = key_vec + count - 1;
}

static inline void top_prefix_tree_ctx_next_key(struct top_prefix_tree_ctx* ctx)
{
    if(ctx->current_key == ctx->last_key) {
        ctx->current_key = &null_key;
    } else {
        ++ctx->current_key;
    }
    ctx->key = ctx->current_key->key;
}

static inline unsigned char top_prefix_tree_ctx_key(struct top_prefix_tree_ctx* ctx)
{
    while(ctx->key == ctx->current_key->key_end) {
        top_prefix_tree_ctx_next_key(ctx);
    }
    return (unsigned char)*ctx->key;
}

static inline char top_prefix_tree_ctx_key_inc(struct top_prefix_tree_ctx* ctx)
{
    while(ctx->key == ctx->current_key->key_end) {
        top_prefix_tree_ctx_next_key(ctx);
    }
    return (unsigned char)*ctx->key++;
}

#define PREFIX_TREE_CTX_KEY_END(ctx,key) ( *key == 0 || key == ctx->end_key)

static inline void top_prefix_tree_free_bulk(struct top_prefix_tree* tree)
{
    struct top_prefix_tree_slots* slots ;
    while(tree->bulk_alloc) {
        slots = tree->bulk_alloc;
        tree->bulk_alloc = slots->bulk_next;
        tree->conf.pffree(tree->conf.user_data,slots,slots->bulk_size);
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
        memset(*pslots,0,tree->node_size);
        return TOP_OK;
    }

    if(tree->conf.max_capacity <= tree->capacity) {
        return TOP_ERROR(-1);
    }

    unsigned long bulk_size = tree->conf.max_capacity - tree->capacity;
    if(bulk_size > tree->bulk_size) bulk_size = tree->bulk_size;
    unsigned short avail_num = bulk_size / tree->node_size;
    if(avail_num < 2) return TOP_ERROR(-1);
    top_error_t err;
    struct top_prefix_tree_slots* slots;
    err = tree->conf.pfmalloc(tree->conf.user_data,bulk_size,(void**)&slots);
    if(top_errno(err)) return err;

    tree->capacity += bulk_size;
    slots->avail_num = avail_num - 2;
    slots->bulk_size = bulk_size;
    slots->bulk_next = tree->bulk_alloc;
    tree->bulk_alloc = slots;

    *pslots = (struct top_prefix_tree_slots*)((char*)slots + (slots->avail_num + 1) * tree->node_size);
    memset(*pslots,0,tree->node_size);
    return TOP_OK;
}

static inline void top_prefix_tree_free_key(struct top_prefix_tree* tree, struct top_prefix_tree_key* key,unsigned short key_buf_size)
{
    assert(0 == (key_buf_size & (sizeof(void*) - 1)));
    assert(0 == ((key_buf_size + sizeof(void*)) & ( sizeof(void*) * 2 - 1)));
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

    top_error_t err;
    err = top_prefix_tree_alloc_slots(tree,(struct top_prefix_tree_slots**)pkey);
    if(top_errno(err)) return err;

    (*pkey)->size = tree->max_key_size;
    (*pkey)->next = 0;
    return TOP_OK;
}

static inline struct top_prefix_tree_slots* top_prefix_tree_ctx_get_parent(struct top_prefix_tree_ctx* ctx)
{
    if(ctx->parent_prev) {
        return (struct top_prefix_tree_slots*)ctx->parent_prev->next;
    } else if(ctx->pparent) {
        assert(!PREFIX_TREE_NODE_IS_KEY(*ctx->pparent));
        return (struct top_prefix_tree_slots*)*ctx->pparent;
    }
    return 0;
}

static inline void top_prefix_tree_key_destroy(struct top_prefix_tree* tree,struct top_prefix_tree_key* key);

static inline void* top_prefix_tree_ctx_get_data(struct top_prefix_tree_ctx* ctx);
static inline void top_prefix_tree_ctx_set_data(struct top_prefix_tree_ctx* ctx,void* data,void** pold_data);
static inline void top_prefix_tree_ctx_move_to_next(struct top_prefix_tree_ctx* ctx);
static inline unsigned long* top_prefix_tree_slots_get_slot_addr(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,char c);
static inline enum top_prefix_match_result top_prefix_tree_key_match(struct top_prefix_tree_ctx* ctx);
static inline enum top_prefix_match_result top_prefix_tree_slots_match(struct top_prefix_tree_ctx* ctx);


static inline void* top_prefix_tree_ctx_find(struct top_prefix_tree_ctx* ctx)
{
    int has_compress_key;
    struct top_prefix_tree_key* tree_key;
    struct top_prefix_tree_slots* tree_slots;
    do {
        has_compress_key = PREFIX_TREE_NODE_IS_KEY(*ctx->pself);
        if(has_compress_key) {
            ctx->current = PREFIX_TREE_NODE_KEY(*ctx->pself);
            assert(ctx->current);
            ctx->rlt = top_prefix_tree_key_match(ctx);
        } else {
            ctx->slots = PREFIX_TREE_NODE_SLOTS(*ctx->pself);
            if(ctx->slots == 0) return 0;
            ctx->rlt = top_prefix_tree_slots_match(ctx);
        }
        switch(ctx->rlt) {
        case MATCH_ALL:
            return top_prefix_tree_ctx_get_data(ctx);
        case MATCH_NEXT:
            top_prefix_tree_ctx_move_to_next(ctx);
            break;
        case MATCH_INTRP:
        case MATCH_PREFIX:
        case MATCH_PARTIAL:
        default:
            return 0;
        }
    } while(1);
}

static inline void top_prefix_tree_ctx_move_to_next(struct top_prefix_tree_ctx* ctx)
{
    assert(ctx->slots);
    assert(0 == ctx->current);

    ctx->pparent = ctx->pself;
    ctx->parent_prev = ctx->prev;
    ctx->parent_prev_end_pos = ctx->prev_end_pos;
    assert(*ctx->key);
    assert(ctx->key != ctx->current_key->key_end);
    ctx->pself = top_prefix_tree_slots_get_slot_addr(ctx->tree,ctx->slots,*ctx->key++);
    ctx->slots = 0;
    ctx->prev = 0;
    ctx->prev_end_pos = 0;
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
    assert(slots->slots[slot_idx] == 0);
    slots->slots[slot_idx] = (unsigned long)sub_slots;
    ++slots->count;
}

static inline void top_prefix_tree_slots_link_key(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,unsigned char c,struct top_prefix_tree_key* key)
{
    unsigned int slot_idx = tree->conf.slot_map[c];
    assert(slots->slots[slot_idx] == 0);
    slots->slots[slot_idx] = PREFIX_TREE_GEN_NODE_KEY(key);
    ++slots->count;
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
    unsigned char c;
    do {
        for(i = 0; i < ctx->tree->max_key_size; ++i,++ctx->key)
        {
            c = top_prefix_tree_ctx_key(ctx);
            switch(tree_key->key[i]) {
            case PREFIX_TREE_NODE_KEY_EOF:
                ctx->prev = tree_key;
                ctx->prev_end_pos = i;
                ctx->current = 0;
                ctx->slots = (struct top_prefix_tree_slots*)tree_key->next;
                return top_prefix_tree_slots_match(ctx);
            case PREFIX_TREE_NODE_KEY_EOF_DATA:
                ctx->current = tree_key;
                ctx->matched_size = i;
                return c == 0 ? MATCH_ALL : MATCH_INTRP;
            case PREFIX_TREE_NODE_KEY_EOF_KEY:
                goto cont;
            default:
                break;
            }

            if(PREFIX_TREE_NOT_EQUAL(ctx->tree,tree_key->key[i], c)) {
                ctx->current = tree_key;
                ctx->matched_size = i;
                return c == 0 ? MATCH_PREFIX : MATCH_PARTIAL;
            } else {
                assert(c < 128);
            }
        }
        assert(0);
cont:
        ctx->prev = tree_key;
        ctx->prev_end_pos = i;
        tree_key = tree_key->next;
        assert(tree_key);
    } while(1);
}

static inline enum top_prefix_match_result top_prefix_tree_slots_match(struct top_prefix_tree_ctx* ctx) {
    struct top_prefix_tree_slots* tree_slots = ctx->slots;
    assert(tree_slots);
    int i;
    unsigned char c;
    for(i = 0; i < PREFIX_TREE_NODE_SLOTS_KEY_SIZE; ++i,++ctx->key)
    {
        c = top_prefix_tree_ctx_key(ctx);
        if(tree_slots->key[i] == 0) {
            break;
        }
        if(PREFIX_TREE_NOT_EQUAL(ctx->tree,tree_slots->key[i] , c)) {
            ctx->matched_size = i;
            return c == 0 ? MATCH_PREFIX : MATCH_PARTIAL;
        }
    }
    ctx->matched_size = i;
    return c == 0 ? MATCH_ALL : MATCH_NEXT;
}

void * top_prefix_tree_simple_find(struct top_prefix_tree* tree, const char* key)
{
    if(tree->root == 0 || 0 == key || *key == 0) return 0;
    struct top_prefix_tree_key_vec key_vec = { key, 0 };
    struct top_prefix_tree_ctx ctx;
    top_prefix_tree_ctx_init(&ctx,tree,&key_vec,1);
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
    top_error_t err;
    do {
        has_compress_key = PREFIX_TREE_NODE_IS_KEY(*ctx->pself);
        if(has_compress_key) {
            ctx->current = PREFIX_TREE_NODE_KEY(*ctx->pself);
            assert(ctx->current);
            ctx->rlt = top_prefix_tree_key_match(ctx);
        } else {
            ctx->slots = PREFIX_TREE_NODE_SLOTS(*ctx->pself);
            if(ctx->slots == 0) {
                err = top_prefix_tree_ctx_new_key(ctx,data);
                goto fail;
            }
            ctx->rlt = top_prefix_tree_slots_match(ctx);
        }
        switch(ctx->rlt) {
        case MATCH_ALL:
            top_prefix_tree_ctx_set_data(ctx,data,pold_data);
            return TOP_OK;
        case MATCH_NEXT:
            top_prefix_tree_ctx_move_to_next(ctx);
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

static inline unsigned short top_prefix_tree_strncpy(unsigned char* dest,struct top_prefix_tree_ctx* ctx,unsigned short size)
{
    unsigned short i ;
    unsigned char c;
    for(i = 0; i < size; ++i,++ctx->key) {
        c = top_prefix_tree_ctx_key(ctx);
        if(0 == c) {
            return i;
        }
        dest[i] = c;
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

static inline void top_prefix_tree_key_destroy(struct top_prefix_tree* tree, struct top_prefix_tree_key* tree_key)
{
    struct top_prefix_tree_key* next_key;
    unsigned short i;

    while(tree_key) {
        next_key = (struct top_prefix_tree_key*)tree_key->next;
        for(i = 0; tree_key->key[i] < PREFIX_TREE_NODE_KEY_EOF_KEY; ++i) {}
        switch(tree_key->key[i]) {
        case PREFIX_TREE_NODE_KEY_EOF_KEY:
            top_prefix_tree_free_key(tree,tree_key,PREFIX_TREE_NODE_KEY_SIZE(i + 1));
            break;
        case PREFIX_TREE_NODE_KEY_EOF:
            top_prefix_tree_free_key(tree,tree_key,PREFIX_TREE_NODE_KEY_SIZE(i + 1));
            assert(0);
            return;
        case PREFIX_TREE_NODE_KEY_EOF_DATA:
            top_prefix_tree_free_key(tree,tree_key,PREFIX_TREE_NODE_KEY_SIZE(i + 1));
            return;
        }
        tree_key = next_key;
    }
}

static inline top_error_t top_prefix_tree_new_leaf(struct top_prefix_tree* tree,struct top_prefix_tree_ctx* ctx,void* data,struct top_prefix_tree_key** pkey)
{
    struct top_prefix_tree_key* new_key,*tree_key,*next_key;
    unsigned short size,copied_size;
    top_error_t err;
    err = top_prefix_tree_alloc_key(tree,&tree_key);
    if(top_errno(err)) return err;
    new_key = tree_key;
    size = tree_key->size;
    copied_size = top_prefix_tree_strncpy(tree_key->key,ctx,size);
    while(copied_size == size) {
        assert(tree_key->next == 0);
        --size;
        --ctx->key;
        tree_key->key[size] = PREFIX_TREE_NODE_KEY_EOF_KEY;
        err = top_prefix_tree_alloc_key(tree,&next_key);
        if(top_errno(err)) goto fail;
        size = next_key->size;
        copied_size = top_prefix_tree_strncpy(next_key->key,ctx,size);
        tree_key->next = next_key;
        tree_key = next_key;
    }
    tree_key->next = data;
    tree_key->key[copied_size] = PREFIX_TREE_NODE_KEY_EOF_DATA;
    top_prefix_tree_key_reclaim(tree,tree_key,size,copied_size + 1);
    *pkey = new_key;
    return TOP_OK;
fail:
    top_prefix_tree_key_destroy(tree,new_key);
    return err;
}

static inline top_error_t top_prefix_tree_ctx_new_key(struct top_prefix_tree_ctx* ctx,void* data)
{
    struct top_prefix_tree_key* key;
    top_error_t err;
    err = top_prefix_tree_new_leaf(ctx->tree,ctx,data,&key);
    if(top_errno(err) == 0) {
        struct top_prefix_tree_slots* parent = top_prefix_tree_ctx_get_parent(ctx);
        if(parent) ++parent->count;
        *ctx->pself = PREFIX_TREE_GEN_NODE_KEY(key);
    }
    return err;
}

static inline top_error_t top_prefix_tree_ctx_append_slots(struct top_prefix_tree_ctx* ctx,void* data)
{
    top_error_t err;
    struct top_prefix_tree_slots* slots;
    struct top_prefix_tree_ctx rollback;
    struct top_prefix_tree_key* current,*new_leaf;
    unsigned char c;
    err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
    if(top_errno(err)) return err;
    assert(current->key[ctx->matched_size] == PREFIX_TREE_NODE_KEY_EOF_DATA);
    c = top_prefix_tree_ctx_key_inc(ctx);
    err = top_prefix_tree_new_leaf(ctx->tree,ctx,data,&new_leaf);
    if(top_errno(err)) {
        top_prefix_tree_free_slots(ctx->tree,slots);
        return err;
    }
    top_prefix_tree_slots_link_key(ctx->tree,slots,c,new_leaf);
    slots->data = ctx->current->next;
    current->key[ctx->matched_size] = PREFIX_TREE_NODE_KEY_EOF;
    current->next = slots;
    return TOP_OK;
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
    assert(ctx->slots->key[ctx->matched_size]);
    assert(ctx->matched_size < PREFIX_TREE_NODE_SLOTS_KEY_SIZE);
    err = top_prefix_tree_alloc_slots(ctx->tree,&slots);
    if(top_errno(err)) return err;

    slots->data = data;
    if(ctx->prev) {
        ctx->prev->next = slots;
    } else {
        *ctx->pself = (unsigned long)slots;
    }
    memcpy(slots->key,ctx->slots->key,ctx->matched_size);
    slots->key[ctx->matched_size] = 0; /** don't warry about overring slots->slots */
    top_prefix_tree_slots_link_slots(ctx->tree,slots,ctx->slots->key[ctx->matched_size],ctx->slots);
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
    if(top_errno(err)) return err;

    slots->data = data;
    if(ctx->matched_size <= PREFIX_TREE_NODE_SLOTS_KEY_SIZE) {
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
            --size;
            tree_key->key[size] = PREFIX_TREE_NODE_KEY_EOF_KEY;
            memcpy(tree_key->key,key,size);
            err = top_prefix_tree_alloc_key(ctx->tree,&next_key);
            if(top_errno(err)) goto fail;
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
    unsigned char c = ctx->current->key[ctx->matched_size];
    switch(ctx->current->key[ctx->matched_size + 1]) {
    case PREFIX_TREE_NODE_KEY_EOF:
        top_prefix_tree_slots_link_slots(ctx->tree,slots,c,ctx->current->next);
        top_prefix_tree_free_key(ctx->tree,ctx->current,PREFIX_TREE_NODE_KEY_SIZE(ctx->matched_size+1));
        break;
    case PREFIX_TREE_NODE_KEY_EOF_KEY:
        top_prefix_tree_slots_link_key(ctx->tree,slots,c,ctx->current->next);
        top_prefix_tree_free_key(ctx->tree,ctx->current,PREFIX_TREE_NODE_KEY_SIZE(ctx->matched_size+1));
        break;
    default:
        if(ctx->matched_size > PREFIX_TREE_NODE_KEY_MIN_SIZE() && ((ctx->matched_size + 1) & (sizeof(void*) - 1)) == 0) {
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
    top_prefix_tree_key_destroy(ctx->tree,new_key);
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
    unsigned char c = top_prefix_tree_ctx_key_inc(ctx);
    top_error_t err;

    err = top_prefix_tree_new_leaf(ctx->tree,ctx,data,&new_leaf);
    if(top_errno(err)) return err;
    if(ctx->matched_size == 0) {
        err = top_prefix_tree_ctx_insert_slots(ctx,0,&slots);
    } else if(ctx->slots) {
        err = top_prefix_tree_ctx_split_slots(ctx,0,&slots);
    } else {
        err = top_prefix_tree_ctx_split_key(ctx,0,&slots);
    }
    if(top_errno(err)) goto fail;
    top_prefix_tree_slots_link_key(ctx->tree,slots,c,new_leaf);
    return TOP_OK;
fail:
    top_prefix_tree_key_destroy(ctx->tree,new_leaf);
    return err;
}

top_error_t top_prefix_tree_simple_insert(struct top_prefix_tree* tree,const char* key,void* data,void** pold_data)
{
    if(0 == key || *key == 0) {
        return TOP_ERROR(-1);
    }
    struct top_prefix_tree_key_vec key_vec = { key,0 };
    struct top_prefix_tree_ctx ctx;
    top_prefix_tree_ctx_init(&ctx,tree,&key_vec,1);
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
    if(tree_conf->bulk == 0) tree_conf->bulk = 10;
    tree->node_size = PREFIX_TREE_NODE_SIZE(tree);
    tree->bulk_size = tree_conf->bulk * tree->node_size;
    tree->max_key_size = PREFIX_TREE_NODE_KEY_MAX_SIZE(tree);
    if(tree_conf->max_capacity == 0) {
        tree_conf->max_capacity = (unsigned long)-1;
    }
}

void top_prefix_tree_fini(struct top_prefix_tree* tree)
{
    top_prefix_tree_free_bulk(tree);
}


static inline struct top_prefix_tree_key* top_prefix_tree_slots_to_leaf(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots, unsigned char append_char, unsigned short* pend_pos)
{
    struct top_prefix_tree_key* tree_key;
    unsigned short i;
    unsigned short size;
    assert(slots->count == 0);
    if(tree->cached_key) {
        /** 单个KEY的缓冲肯定大于等于slots的缓冲区长度 */
        assert(PREFIX_TREE_NODE_KEY_MIN_SIZE() > PREFIX_TREE_NODE_SLOTS_KEY_SIZE);
        (void)top_prefix_tree_alloc_key(tree,&tree_key);
        size = tree_key->size;
        assert(size >= PREFIX_TREE_NODE_SLOTS_KEY_SIZE);
        for(i = 0; i < PREFIX_TREE_NODE_SLOTS_KEY_SIZE; ++i) {
            tree_key->key[i] = slots->key[i];
            if(0 == tree_key->key[i]) break;
        }
        if(append_char) {
            if(i == PREFIX_TREE_NODE_SLOTS_KEY_SIZE && size == PREFIX_TREE_NODE_SLOTS_KEY_SIZE + 1) {
                top_prefix_tree_free_key(tree,tree_key,size);
                goto fail;
            }
            tree_key->key[i++] = append_char;
        }
        tree_key->next = slots->data;
        top_prefix_tree_key_reclaim(tree,tree_key,size,i + 1);
        *pend_pos = i;
        top_prefix_tree_free_slots(tree,slots);
    } else {
fail:
        tree_key = (struct top_prefix_tree_key*)slots;
        /** key,slots的内存布局是一样的，这里只需要找到key的长度设置DATA标志即可 */
        for(i = 0 ; i < PREFIX_TREE_NODE_SLOTS_KEY_SIZE; ++i) {
            if(tree_key->key[i] == 0) break;
        }
        top_prefix_tree_key_reclaim(tree,tree_key,PREFIX_TREE_NODE_KEY_MAX_SIZE(tree),i + 1);
        *pend_pos = i;
    }
    return tree_key;
}

static inline void top_prefix_tree_ctx_parent_slots_to_leaf(struct top_prefix_tree_ctx* ctx, struct top_prefix_tree_slots* parent)
{
    if(parent->key[0] == 0 && ctx->parent_prev) {
        ctx->parent_prev->key[ctx->parent_prev_end_pos] = PREFIX_TREE_NODE_KEY_EOF_DATA;
        ctx->parent_prev->next = parent->data;
        top_prefix_tree_free_slots(ctx->tree,parent);
    } else {
        struct top_prefix_tree_key* tree_key;
        unsigned short end_pos;
        tree_key = top_prefix_tree_slots_to_leaf(ctx->tree,parent,0,&end_pos);
        tree_key->key[end_pos] = PREFIX_TREE_NODE_KEY_EOF_DATA;
        if(ctx->parent_prev == 0) {
            *ctx->pparent = PREFIX_TREE_GEN_NODE_KEY(tree_key);
        } else {
            ctx->parent_prev->next = tree_key;
            ctx->parent_prev->key[ctx->parent_prev_end_pos] = PREFIX_TREE_NODE_KEY_EOF_KEY;
        }
    }
}

static inline void top_prefix_tree_ctx_parent_slots_merge_child(struct top_prefix_tree_ctx* ctx,struct top_prefix_tree_slots* parent)
{
    struct top_prefix_tree_key* tree_key;
    unsigned short end_pos;
    unsigned short i;
    unsigned long child;
    unsigned char c;
    assert(parent->data == 0);
    for(i = 0; i < ctx->tree->conf.key_map_size; ++i) {
        if(parent->slots[i]) break;
    }
    assert(i < ctx->tree->conf.key_map_size);
    child = parent->slots[i];
    c = ctx->tree->conf.key_map[i];

    parent->count = 0;
    parent->slots[i] = 0;
    tree_key = top_prefix_tree_slots_to_leaf(ctx->tree,parent,c,&end_pos);

    if(PREFIX_TREE_NODE_IS_KEY(child)) {
        tree_key->key[end_pos] = PREFIX_TREE_NODE_KEY_EOF_KEY;
        tree_key->next = PREFIX_TREE_NODE_KEY(child);
    } else {
        tree_key->key[end_pos] = PREFIX_TREE_NODE_KEY_EOF;
        tree_key->next = (void*)child;
    }
    if(ctx->parent_prev == 0) {
        *ctx->pparent = PREFIX_TREE_GEN_NODE_KEY(tree_key);
    } else {
        ctx->parent_prev->next = tree_key;
        ctx->parent_prev->key[ctx->parent_prev_end_pos] = PREFIX_TREE_NODE_KEY_EOF_KEY;
    }
}

static inline void top_prefix_tree_ctx_remove_self_leaf(struct top_prefix_tree_ctx* ctx)
{
    unsigned long self;
    struct top_prefix_tree_slots* parent;
    assert(PREFIX_TREE_NODE_IS_KEY(*ctx->pself));
    assert(ctx->slots == 0);
    self = *ctx->pself;
    *ctx->pself = 0;
    assert(PREFIX_TREE_NODE_IS_KEY(self));
    top_prefix_tree_key_destroy(ctx->tree,PREFIX_TREE_NODE_KEY(self));
    parent = top_prefix_tree_ctx_get_parent(ctx);
    if(parent) {
        switch(--parent->count) {
        case 0:
            top_prefix_tree_ctx_parent_slots_to_leaf(ctx,parent);
            break;
        case 1:
            if(parent->data == 0) {
                top_prefix_tree_ctx_parent_slots_merge_child(ctx,parent);
            }
            break;
        default:
            break;
        }
    }
}

static inline void* top_prefix_tree_ctx_delete(struct top_prefix_tree_ctx* ctx)
{
    void* data = top_prefix_tree_ctx_find(ctx);
    if(ctx->rlt == MATCH_ALL) {
        if(ctx->slots) {
            ctx->slots->data = 0;
        } else {
            ctx->current->next = 0;
            top_prefix_tree_ctx_remove_self_leaf(ctx);
        }
    }
    return data;
}

void* top_prefix_tree_simple_delete(struct top_prefix_tree* tree,const char* key)
{
    if(tree->root == 0 || 0 == key || *key == 0) return 0;
    void* data;
    struct top_prefix_tree_key_vec key_vec = { key, 0 };
    struct top_prefix_tree_ctx ctx;
    top_prefix_tree_ctx_init(&ctx,tree,&key_vec,1);
    return top_prefix_tree_ctx_delete(&ctx);
}

top_error_t top_prefix_tree_insert(struct top_prefix_tree* tree,const struct top_prefix_tree_key_vec* key,int count,void* data,void** pold_data)
{
    if(key == 0 || count <= 0 || 0 == data) return TOP_OK;
    struct top_prefix_tree_ctx ctx;
    top_prefix_tree_ctx_init(&ctx,tree,key,count);
    return top_prefix_tree_ctx_insert(&ctx,data,pold_data);
}

void* top_prefix_tree_find(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key,int count)
{
    if(key == 0 || count <= 0 || tree->root == 0) return 0;
    struct top_prefix_tree_ctx ctx;
    top_prefix_tree_ctx_init(&ctx,tree,key,count);
    return top_prefix_tree_ctx_find(&ctx);
}


void* top_prefix_tree_delete(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key,int count)
{
    if(tree->root == 0 || 0 == key || count <= 0) return 0;
    struct top_prefix_tree_ctx ctx;
    top_prefix_tree_ctx_init(&ctx,tree,key,count);
    return top_prefix_tree_ctx_delete(&ctx);
}

struct top_prefix_tree_visit_data {
    unsigned long* pself;
    unsigned int key_start;
    unsigned char slot_idx;
};

#define PREFIX_TREE_VISIT_DEEP 64 //栈的最大深度，超过之后采用递归调用的方式完成

static inline int top_prefix_tree_visit_stack_push(struct top_prefix_tree_visit_data* stack,int* size, unsigned long* pself,unsigned int key_start,unsigned char slot_idx)
{
    if(*size == PREFIX_TREE_VISIT_DEEP) return 0;
    stack += (*size)++;
    stack->pself = pself;
    stack->key_start = key_start;
    stack->slot_idx = slot_idx;
    return 1;
}

static inline void top_prefix_tree_visit_ctx_set_suffix(struct top_prefix_tree_visit_ctx* ctx, int pos,char c)
{
    if(pos < ctx->suffix_buf_len) ctx->suffix_buf[pos] = c;
}

static inline void top_prefix_tree_visit_ctx_append_suffix(struct top_prefix_tree_visit_ctx* ctx, int* pos,char c)
{
    int npos = (*pos)++;
    if(npos < ctx->suffix_buf_len) ctx->suffix_buf[npos] = c;
}
static inline int top_prefix_tree_visit_ctx_visit(struct top_prefix_tree_visit_ctx* ctx, void* data,int pos,struct top_prefix_tree* tree)
{
    if(pos < ctx->suffix_buf_len) ctx->suffix_buf[pos] = 0;
    return ctx->visit(ctx,data,pos,tree);
}

static int top_prefix_tree_visit_children(struct top_prefix_tree* tree,struct top_prefix_tree_visit_data* stack,int size,struct top_prefix_tree_visit_ctx* ctx);
static int top_prefix_tree_visit_key(struct top_prefix_tree* tree,struct top_prefix_tree_key* key,int start,struct top_prefix_tree_visit_data* stack,int* size,struct top_prefix_tree_visit_ctx* ctx,int pos);
static int top_prefix_tree_visit_slots(struct top_prefix_tree* tree,struct top_prefix_tree_slots* slots,int start,struct top_prefix_tree_visit_data* stack,int* size,struct top_prefix_tree_visit_ctx* ctx,int pos);

static int top_prefix_tree_visit_node(struct top_prefix_tree* tree,unsigned long node,struct top_prefix_tree_visit_ctx* ctx, int pos)
{
    struct top_prefix_tree_visit_data stack[PREFIX_TREE_VISIT_DEEP];
    int size = 0;
    (void)top_prefix_tree_visit_stack_push(stack,&size,&node,pos,tree->conf.key_map_size);
    return top_prefix_tree_visit_children(tree,stack,size,ctx);
}

static inline int top_prefix_tree_visit_match_continue(struct top_prefix_tree_visit_ctx* ctx, struct top_prefix_tree_ctx* match_ctx)
{
    struct top_prefix_tree_visit_data stack[PREFIX_TREE_VISIT_DEEP];
    int size = 0;
    if(match_ctx->slots) {
        top_prefix_tree_visit_slots(match_ctx->tree,match_ctx->slots,match_ctx->matched_size,stack,&size,ctx,0);
    } else {
        top_prefix_tree_visit_key(match_ctx->tree,match_ctx->current,match_ctx->matched_size,stack,&size,ctx,0);
    }
    return top_prefix_tree_visit_children(match_ctx->tree,stack,size,ctx);
}

static int top_prefix_tree_visit_children(struct top_prefix_tree* tree,struct top_prefix_tree_visit_data* stack,int size,struct top_prefix_tree_visit_ctx* ctx)
{
    int rlt = 1;
    int pos;
    unsigned long node;
    struct top_prefix_tree_visit_data data;
    struct top_prefix_tree_visit_data* current;
    while(rlt && size && (current = &stack[--size])) {
        data = *current;
        if(data.slot_idx < tree->conf.key_map_size - 1) {
            ++current->pself;
            ++current->slot_idx;
            ++size;
        }
        node = *data.pself;
        if(node) {
            pos = data.key_start;
            if(data.slot_idx < tree->conf.key_map_size) {
                top_prefix_tree_visit_ctx_append_suffix(ctx,&pos, tree->conf.key_map[data.slot_idx]);
            }
            if(PREFIX_TREE_NODE_IS_KEY(node)) {
                rlt = top_prefix_tree_visit_key(tree,PREFIX_TREE_NODE_KEY(node),0,stack,&size,ctx,pos);
            } else {
                rlt = top_prefix_tree_visit_slots(tree,PREFIX_TREE_NODE_SLOTS(node),0,stack,&size,ctx,pos);
            }
        }
    }
    return rlt;
}

static int top_prefix_tree_visit_key(struct top_prefix_tree* tree,struct top_prefix_tree_key* tree_key,int start,struct top_prefix_tree_visit_data* stack,int* size,struct top_prefix_tree_visit_ctx* ctx,int pos)
{
    const char* key = tree_key->key + start;
    unsigned char c;
    while(1) {
        switch((c = *key++)) {
        case PREFIX_TREE_NODE_KEY_EOF_KEY:
            tree_key = tree_key->next;
            assert(tree_key);
            key = tree_key->key;
            break;
        case PREFIX_TREE_NODE_KEY_EOF_DATA:
            return top_prefix_tree_visit_ctx_visit(ctx,tree_key->next,pos,tree);
        case PREFIX_TREE_NODE_KEY_EOF:
            return top_prefix_tree_visit_slots(tree,(struct top_prefix_tree_slots*)tree_key->next,0,stack,size,ctx,pos);
        default:
            top_prefix_tree_visit_ctx_append_suffix(ctx,&pos,c);
            break;
        }
    }
}

static int top_prefix_tree_visit_slots(struct top_prefix_tree* tree,struct top_prefix_tree_slots* tree_slots,int start,struct top_prefix_tree_visit_data* stack,int* size,struct top_prefix_tree_visit_ctx* ctx,int pos)
{
    const char* key = tree_slots->key;
    int i;
    int rlt = 1;
    for(i = start; i < PREFIX_TREE_NODE_SLOTS_KEY_SIZE && key[i]; ++i) {
        top_prefix_tree_visit_ctx_append_suffix(ctx,&pos,key[i]);
    }
    if(tree_slots->data) {
        rlt = top_prefix_tree_visit_ctx_visit(ctx,tree_slots->data,pos,tree);
    }
    if(rlt && !top_prefix_tree_visit_stack_push(stack,size,&tree_slots->slots[0],pos,0)) {
        for(i = 0; rlt && i < tree->conf.key_map_size; ++i) {
            if(tree_slots->slots[i]) {
                top_prefix_tree_visit_ctx_set_suffix(ctx,pos,tree->conf.key_map[i]);
                rlt = top_prefix_tree_visit_node(tree,tree_slots->slots[i],ctx,pos + 1);
            }
        }
    }
    return rlt;
}

int top_prefix_tree_simple_visit(struct top_prefix_tree* tree, const char* key,struct top_prefix_tree_visit_ctx* ctx)
{
    if(key == 0) key = "";
    struct top_prefix_tree_key_vec key_vec = { key, 0};
    return top_prefix_tree_visit(tree,&key_vec,1,ctx);
}

int top_prefix_tree_visit(struct top_prefix_tree* tree, const struct top_prefix_tree_key_vec* key_vec,int count,struct top_prefix_tree_visit_ctx* ctx)
{
    struct top_prefix_tree_ctx match_ctx;
    top_prefix_tree_ctx_init(&match_ctx,tree,key_vec,count);
    (void)top_prefix_tree_ctx_find(&match_ctx);
    switch(match_ctx.rlt) {
    case MATCH_ALL:
        if(match_ctx.slots) {
            return top_prefix_tree_visit_node(tree,(unsigned long)match_ctx.slots,ctx,0);
        } else {
            return top_prefix_tree_visit_ctx_visit(ctx,match_ctx.current->next,0,tree);
        }
        break;
    case MATCH_PREFIX:
        top_prefix_tree_visit_match_continue(ctx,&match_ctx);
        break;
    default:
        return;
    }
    return 1;
}

