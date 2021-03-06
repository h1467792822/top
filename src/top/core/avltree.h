
#ifndef TOP_CORE_AVLTREE_H
#define TOP_CORE_AVLTREE_H

#ifndef TOP_CORE_STDDEF_H
#include <top/core/stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define top_avl_entry(ptr,type,member) top_container_of(ptr,type,member)

struct top_avltree_node {
    union {
        struct top_avltree_node* children[2];
        struct {
            struct top_avltree_node* left;
            struct top_avltree_node* right;
        };
    };
    unsigned long parent;
    int bf;
};

struct top_avltree {
    struct top_avltree_node* root;
};

static inline struct top_avltree_node* top_avltree_node_parent(const struct top_avltree_node* node)
{
    return (struct top_avltree_node*)(node->parent & ~3);
}

static inline void top_avltree_init(struct top_avltree* tree)
{
    tree->root = 0;
}

void top_avltree_link_node(struct top_avltree* tree,struct top_avltree_node* node,struct top_avltree_node* parent,struct top_avltree_node** link) ;

void top_avltree_erase(struct top_avltree* tree, struct top_avltree_node* node);

struct top_avltree_node* top_avltree_first(const struct top_avltree* tree);

struct top_avltree_node* top_avltree_last(const struct top_avltree* tree);

struct top_avltree_node* top_avltree_node_next(const struct top_avltree_node* node);

struct top_avltree_node* top_avltree_node_prev(const struct top_avltree_node* node);


#ifdef __cplusplus
}
#endif

#endif

