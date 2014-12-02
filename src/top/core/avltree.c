
#include <top/core/avltree.h>
#include <top/core/stddef.h>
#include <stdio.h>
#include <assert.h>

#define AVLTREE_RIGHT_CHILD(node) ((node)->parent & 0x01)
#define AVLTREE_LEFT_CHILD(node) (!((node)->parent & 0x01))
#define AVLTREE_GEN_PARENT(p,idx) ((unsigned long)(idx) | (unsigned long)p)

static inline void top_avltree_node_set_parent(struct top_avltree* tree,struct top_avltree_node* node,unsigned long parent) 
{
	struct top_avltree_node* pparent = (struct top_avltree_node*)(parent & ~0x01);
	node->parent = parent;
	if(pparent) 
		pparent->children[parent & 0x01] = node;
	else
		tree->root = node;
}

static inline void top_avltree_ll_rotation(struct top_avltree* tree,struct top_avltree_node* pn,struct top_avltree_node* n) 
{
	struct top_avltree_node* rcn = n->children[1];
	struct top_avltree_node* gn = top_avltree_node_parent(pn);

	n->bf = 0;
	top_avltree_node_set_parent(tree,n,pn->parent);

	n->children[1] = pn;
	pn->parent = AVLTREE_GEN_PARENT(n,1);
	pn->bf = 0;

	pn->children[0] = rcn;
	if(rcn) rcn->parent = AVLTREE_GEN_PARENT(pn,0);
}

static inline void top_avltree_lr_rotation(struct top_avltree* tree,struct top_avltree_node* pn,struct top_avltree_node* n) 
{
	struct top_avltree_node* rcn = n->children[1];
	struct top_avltree_node* rlcn = rcn->children[0];
	struct top_avltree_node* rrcn = rcn->children[1];
	struct top_avltree_node* gn = top_avltree_node_parent(pn);

	top_avltree_node_set_parent(tree,rcn,pn->parent);

	rcn->children[0] = n;
	n->parent = AVLTREE_GEN_PARENT(rcn,0);

	n->children[1] = rlcn;
	if(rlcn) rlcn->parent = AVLTREE_GEN_PARENT(n,1);

	rcn->children[1] = pn;
	pn->parent = AVLTREE_GEN_PARENT(rcn,1);

	pn->children[0] = rrcn;
	if(rrcn) rrcn->parent = AVLTREE_GEN_PARENT(pn,0);

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
	struct top_avltree_node* lcn = n->children[0];
	struct top_avltree_node* gn = top_avltree_node_parent(pn);

	n->bf = 0;
	top_avltree_node_set_parent(tree,n,pn->parent);

	pn->bf = 0;
	n->children[0] = pn;
	pn->parent = AVLTREE_GEN_PARENT(n,0);

	pn->children[1] = lcn;
	if(lcn) lcn->parent = AVLTREE_GEN_PARENT(pn,1);
}

static inline void top_avltree_rl_rotation(struct top_avltree* tree,struct top_avltree_node* pn,struct top_avltree_node* n) 
{
	struct top_avltree_node* lcn = n->children[0];
	struct top_avltree_node* llcn = lcn->children[0];
	struct top_avltree_node* lrcn = lcn->children[1];
	struct top_avltree_node* gn = top_avltree_node_parent(pn);

	top_avltree_node_set_parent(tree,lcn,pn->parent);

	lcn->children[1] = n;
	n->parent = AVLTREE_GEN_PARENT(lcn,1);

	n->children[0] = lrcn;
	if(lrcn) lrcn->parent = AVLTREE_GEN_PARENT(n,0);

	lcn->children[0] = pn;
	pn->parent = AVLTREE_GEN_PARENT(lcn,0);

	pn->children[1] = llcn;
	if(llcn) llcn->parent = AVLTREE_GEN_PARENT(pn,1);

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
	node->children[0] = node->children[1] = 0;
	if(parent){
		unsigned long idx =  link == &parent->children[0] ? 0 : 1;
		node->parent = AVLTREE_GEN_PARENT(parent,idx);
	}else{
		node->parent = 0;
	}
	node->bf = 0;
	*link = node;
	while(parent) {
		if(AVLTREE_RIGHT_CHILD(node)){
			parent->bf -= 1;
		}else {
			parent->bf += 1;
		}
		switch(parent->bf) {
		case 2:
			if(node->bf == 1) {
				top_avltree_ll_rotation(tree,parent,node);
			}else {
				top_avltree_lr_rotation(tree,parent,node);
			}
			return;
		case -2:
			if(node->bf == 1) {
				top_avltree_rl_rotation(tree,parent,node);
			}else {
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

struct top_avltree_node* top_avltree_first(const struct top_avltree* tree)
{
	struct top_avltree_node* node = tree->root;
	if(node) {
		while(node->children[0]) node = node->children[0];
	}
	return (struct top_avltree_node*)node;
}

struct top_avltree_node* top_avltree_node_next(const struct top_avltree_node* node)
{
	struct top_avltree_node* parent;

	if(node->children[1]) {
		node = node->children[1];
		while(node->children[0]) node=node->children[0];
		return (struct top_avltree_node*)node;
	}	 

	while((parent = top_avltree_node_parent(node)) && AVLTREE_RIGHT_CHILD(node)) {
		 node = parent;
	}
	return parent;
}

struct top_avltree_node* top_avltree_last(const struct top_avltree* tree)
{
	struct top_avltree_node* node = tree->root;
	if(node) {
		while(node->children[1]) node = node->children[1];
	}
	return (struct top_avltree_node*)node;
}

struct top_avltree_node* top_avltree_node_prev(const struct top_avltree_node* node)
{
	struct top_avltree_node* parent;

	if(node->children[0]) {
		node = node->children[0];
		while(node->children[1]) node=node->children[1];
		return (struct top_avltree_node*)node;
	}	 

	while((parent = top_avltree_node_parent(node)) && AVLTREE_LEFT_CHILD(node)) {
		 node = parent;
	}
	return parent;
}
