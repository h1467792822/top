
#include <top/core/avltree.h>
#include <top/core/stddef.h>
#include <stdio.h>
#include <assert.h>

#define AVL_POS_MASK (0x01)
#define AVL_PARENT_MASK (~AVL_POS_MASK)
#define AVL_RIGHT_CHILD(node) ((node)->parent & AVL_POS_MASK)
#define AVL_LEFT_CHILD(node) (!((node)->parent & AVL_POS_MASK))
#define AVL_GEN_PARENT(p,idx) ((unsigned long)(idx) | (unsigned long)p)

static inline void top_avltree_set_parent(struct top_avltree* tree,struct top_avltree_node* node,unsigned long parent)
{
    struct top_avltree_node* pparent = (struct top_avltree_node*)(parent & AVL_PARENT_MASK);
    node->parent = parent;
    if(pparent)
        pparent->children[parent & AVL_POS_MASK] = node;
    else
        tree->root = node;
}

static inline void top_avltree_ll_rotation(struct top_avltree* tree,struct top_avltree_node* pn,struct top_avltree_node* n)
{
    struct top_avltree_node* rcn = n->right;
    struct top_avltree_node* gn = top_avltree_node_parent(pn);

    n->bf = 0;
    top_avltree_set_parent(tree,n,pn->parent);

    n->right = pn;
    pn->parent = AVL_GEN_PARENT(n,1);
    pn->bf = 0;

    pn->left = rcn;
    if(rcn) rcn->parent = AVL_GEN_PARENT(pn,0);
}

static inline void top_avltree_lr_rotation(struct top_avltree* tree,struct top_avltree_node* pn,struct top_avltree_node* n)
{
    struct top_avltree_node* rcn = n->right;
    struct top_avltree_node* rlcn = rcn->left;
    struct top_avltree_node* rrcn = rcn->right;
    struct top_avltree_node* gn = top_avltree_node_parent(pn);

    top_avltree_set_parent(tree,rcn,pn->parent);

    rcn->left = n;
    n->parent = AVL_GEN_PARENT(rcn,0);

    n->right = rlcn;
    if(rlcn) rlcn->parent = AVL_GEN_PARENT(n,1);

    rcn->right = pn;
    pn->parent = AVL_GEN_PARENT(rcn,1);

    pn->left = rrcn;
    if(rrcn) rrcn->parent = AVL_GEN_PARENT(pn,0);

    switch(rcn->bf) {
    case 0:
        n->bf = pn->bf = 0;
        break;
    case 1:
        n->bf = 0;
        pn->bf = -1;
        break;
    case -1:
        n->bf = 1;
        pn->bf = 0;
        break;
    }
    rcn->bf = 0;
}

static inline void top_avltree_rr_rotation(struct top_avltree* tree,struct top_avltree_node* pn,struct top_avltree_node* n)
{
    struct top_avltree_node* lcn = n->left;
    struct top_avltree_node* gn = top_avltree_node_parent(pn);

    n->bf = 0;
    top_avltree_set_parent(tree,n,pn->parent);

    pn->bf = 0;
    n->left = pn;
    pn->parent = AVL_GEN_PARENT(n,0);

    pn->right = lcn;
    if(lcn) lcn->parent = AVL_GEN_PARENT(pn,1);
}

static inline void top_avltree_rl_rotation(struct top_avltree* tree,struct top_avltree_node* pn,struct top_avltree_node* n)
{
    struct top_avltree_node* lcn = n->left;
    struct top_avltree_node* llcn = lcn->left;
    struct top_avltree_node* lrcn = lcn->right;
    struct top_avltree_node* gn = top_avltree_node_parent(pn);

    top_avltree_set_parent(tree,lcn,pn->parent);

    lcn->right = n;
    n->parent = AVL_GEN_PARENT(lcn,1);

    n->left = lrcn;
    if(lrcn) lrcn->parent = AVL_GEN_PARENT(n,0);

    lcn->left = pn;
    pn->parent = AVL_GEN_PARENT(lcn,0);

    pn->right = llcn;
    if(llcn) llcn->parent = AVL_GEN_PARENT(pn,1);

    switch(lcn->bf) {
    case 0:
        n->bf = pn->bf = 0;
        break;
    case 1:
        pn->bf = 0;
        n->bf = -1;
        break;
    case -1:
        pn->bf = 1;
        n->bf = 0;
        break;
    }
    lcn->bf = 0;
}

void top_avltree_link_node(struct top_avltree* tree,struct top_avltree_node* node,struct top_avltree_node* parent,struct top_avltree_node** link)
{
    node->left = node->right = 0;
    node->bf = 0;
    *link = node;
    if(parent) {
        unsigned long idx =  link == &parent->left ? 0 : 1;
        node->parent = AVL_GEN_PARENT(parent,idx);
    } else {
        node->parent = 0;
    }
    while(parent) {
        if(AVL_RIGHT_CHILD(node)) {
            parent->bf -= 1;
        } else {
            parent->bf += 1;
        }
        switch(parent->bf) {
        case 2:
            if(node->bf == 1) {
                top_avltree_ll_rotation(tree,parent,node);
            } else {
                top_avltree_lr_rotation(tree,parent,node);
            }
            return;
        case -2:
            if(node->bf == 1) {
                top_avltree_rl_rotation(tree,parent,node);
            } else {
                top_avltree_rr_rotation(tree,parent,node);
            }
            return;
        case 0:
            return;
        case 1:
        case -1:
            node = parent;
            parent = top_avltree_node_parent(node);
            break;
        default:
            assert(0);
            break;
        }
    }
}

static inline struct top_avltree_node* top_avltree_node_first(const struct top_avltree_node* node)
{
    while(node->left) node = node->left;
    return (struct top_avltree_node*)node;
}

static inline struct top_avltree_node* top_avltree_node_last(const struct top_avltree_node* node)
{
    while(node->right) node = node->right;
    return (struct top_avltree_node*)node;
}

static inline void top_avltree_replace_node(struct top_avltree* tree,struct top_avltree_node* pos,struct top_avltree_node* mnode)
{
    struct top_avltree_node* c;
    assert(mnode->left == 0 || 0 == mnode->right);
    top_avltree_set_parent(tree,mnode,pos->parent);
    mnode->bf = pos->bf;
    c = pos->left;
    if(c) {
        mnode->left = c;
        c->parent = AVL_GEN_PARENT(mnode,0);
    }
    c = pos->right;
    if(c ) {
        mnode->right = c;
        c->parent = AVL_GEN_PARENT(mnode,1);
    }
}

static inline struct top_avltree_node* top_avltree_node_unlink(struct top_avltree_node* node,struct top_avltree_node* node_child)
{
    struct top_avltree_node* parent = (struct top_avltree_node*)(node->parent & AVL_PARENT_MASK);
    if(node->parent & 1) {
        parent->right = node_child;
        parent->bf += 1;
        if(node_child) node_child->parent = AVL_GEN_PARENT(parent,1);
    } else {
        parent->left = node_child;
        parent->bf -= 1;
        if(node_child) node_child->parent = AVL_GEN_PARENT(parent,0);
    }
    return parent;
}

static inline struct top_avltree_node* top_avltree_right_rotation(struct top_avltree* tree, struct top_avltree_node* parent)
{
    struct top_avltree_node* node = parent->left;
    assert(node);
    switch(node->bf) {
    case 0:
        top_avltree_ll_rotation(tree,parent,node);
        node->bf = -1;
        parent->bf = 1;
        return node;
    case 1:
        top_avltree_ll_rotation(tree,parent,node);
        return node;
    case -1:
        top_avltree_lr_rotation(tree,parent,node);
        return top_avltree_node_parent(node);
    default:
        assert(0);
        return 0;
    }
}

static inline struct top_avltree_node* top_avltree_left_rotation(struct top_avltree* tree, struct top_avltree_node* parent)
{
    struct top_avltree_node* node = parent->right;
    assert(node);
    switch(node->bf) {
    case 0:
        top_avltree_rr_rotation(tree,parent,node);
        node->bf = 1;
        parent->bf = -1;
        return node;
    case -1:
        top_avltree_rr_rotation(tree,parent,node);
        return node;
    case 1:
        top_avltree_rl_rotation(tree,parent,node);
        return top_avltree_node_parent(node);
    default:
        assert(0);
        return 0;
    }
}

void top_avltree_erase(struct top_avltree* tree, struct top_avltree_node* node)
{
    struct top_avltree_node* mnode;
    struct top_avltree_node* n;
    struct top_avltree_node* p;
    if(node->left == node->right) {
        assert(node->left == 0);
        if(tree->root == node) {
            tree->root = 0;
            return;
        }
        mnode = node;
        p = top_avltree_node_unlink(mnode,0);
    } else if(node->bf == -1)	{
        mnode = top_avltree_node_first(node->right);
        p = top_avltree_node_unlink(mnode,mnode->right);
        top_avltree_replace_node(tree,node,mnode);
    } else {
        mnode = top_avltree_node_last(node->left);
        p = top_avltree_node_unlink(mnode,mnode->left);
        top_avltree_replace_node(tree,node,mnode);
    }
    if(p == node) p = mnode;
    assert(p);
    while(1) {
        switch(p->bf) {
        case 1:
        case -1:
            return;
        case 0:
            n = p;
            break;
        case 2:
            n = top_avltree_right_rotation(tree,p);
            assert(n);
            if(n->bf == -1) return;
            assert(n->bf == 0);
            break;
        case -2:
            n = top_avltree_left_rotation(tree,p);
            assert(n);
            if(n->bf == 1) return;
            assert(n->bf == 0);
            break;
        default:
            assert(0);
            return;
        }
        p = top_avltree_node_parent(n);
        if(!p) return;
        if(AVL_RIGHT_CHILD(n)) {
            p->bf += 1;
        } else {
            p->bf -= 1;
        }
    }
}

struct top_avltree_node* top_avltree_first(const struct top_avltree* tree)
{
    if(tree->root)
        return top_avltree_node_first(tree->root);
    else
        return 0;
}

struct top_avltree_node* top_avltree_node_next(const struct top_avltree_node* node)
{
    struct top_avltree_node* parent;

    if(node->right) {
        node = node->right;
        while(node->left) node=node->left;
        return (struct top_avltree_node*)node;
    }

    while((parent = top_avltree_node_parent(node)) && AVL_RIGHT_CHILD(node)) {
        node = parent;
    }
    return parent;
}

struct top_avltree_node* top_avltree_last(const struct top_avltree* tree)
{
    if(tree->root)
        return top_avltree_node_last(tree->root);
    else
        return 0;
}

struct top_avltree_node* top_avltree_node_prev(const struct top_avltree_node* node)
{
    struct top_avltree_node* parent;

    if(node->left) {
        node = node->left;
        while(node->right) node=node->right;
        return (struct top_avltree_node*)node;
    }

    while((parent = top_avltree_node_parent(node)) && AVL_LEFT_CHILD(node)) {
        node = parent;
    }
    return parent;
}
