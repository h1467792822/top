
#ifndef TOP_CORE_HEAP_H
#define TOP_CORE_HEAP_H

#ifdef __cplusplus
extern "C" {
#endif

struct top_heap_node {
    int s;
    struct top_heap_node* left;
    struct top_heap_node* right;
    struct top_heap_node* parent;
};

typedef int (*top_heap_node_less)(struct top_heap_node* n1, struct top_heap_node* n2);

struct top_heap {
    struct top_heap_node* max;
    top_heap_node_less less;
};

static inline void top_heap_init(struct top_heap* heap,top_heap_node_less less)
{
    heap->max = 0;
    heap->less = less;
}

static inline void top_heap_node_init(struct top_heap_node* node)
{
    node->s = 0;
    node->parent = node->left = node->right = 0;
}

void top_heap_merge(top_heap_node_less less,struct top_heap_node* parent,struct top_heap_node** root, struct top_heap_node* node) ;
void top_heap_update_s(struct top_heap_node* parent);

static inline void top_heap_insert(struct top_heap* heap, struct top_heap_node* node)
{
    node->s = 0;
    node->parent = node->left = node->right = 0;
    top_heap_merge(heap->less,0,&heap->max,node);
}

static inline struct top_heap_node* top_heap_del_max(struct top_heap* heap)
{
    struct top_heap_node* ret = heap->max;
    if(ret) {
        struct top_heap_node* node = heap->max->right;
        heap->max = heap->max->left;
        if(node) top_heap_merge(heap->less,0,&heap->max,node);
    }
    return ret;
}

static inline void top_heap_del(struct top_heap* heap,struct top_heap_node* node)
{
    struct top_heap_node* parent = node->parent;
    if(parent == 0) {
        if(node != heap->max) return;
        (void)top_heap_del_max(heap);
        return;
    }
    struct top_heap_node** sub_root = parent->left == node ? &parent->left : &parent->right;
    *sub_root = node->left;
    if(node->left) node->left->parent = parent;
    if(node->right) top_heap_merge(heap->less,parent,sub_root,node->right);
    top_heap_update_s(parent);
}

#ifdef __cplusplus
}
#endif

#endif

