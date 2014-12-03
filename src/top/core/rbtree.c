
#include <top/core/rbtree.h>
#include <top/core/stddef.h>
#include <stdio.h>
#include <assert.h>

#define RBTREE_RIGHT_CHILD(node) ((node)->parent & 0x01)
#define RBTREE_LEFT_CHILD(node) (!((node)->parent & 0x01))

#define RB_RED (0x02)
#define RB_BLACK (0x00)

#define RBTREE_GEN_PARENT(p,idx,color) ((unsigned long)color | (unsigned long)(idx) | (unsigned long)p)

#define RBTREE_COLOR(n) ( n->parent & 2)

#define RBTREE_SET_RED(node) (node->parent |= RB_RED)
#define RBTREE_SET_BLACK(node) (node->parent &= ~RB_RED)

static inline void top_rbtree_set_parent(struct top_rbtree* tree,struct top_rbtree_node* node,unsigned long parent) 
{
	struct top_rbtree_node* pparent = (struct top_rbtree_node*)(parent & ~0x3);
	node->parent = parent;
	if(pparent) 
		pparent->children[parent & 0x01] = node;
	else
		tree->root = node;
}

static inline void top_rbtree_ll_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu) 
{
	struct top_rbtree_node* pur = pu->children[1];

	assert(RBTREE_COLOR(gu) == RB_BLACK);
	top_rbtree_set_parent(tree,pu,gu->parent);

	pu->children[1] = gu;
	gn->parent = RBTREE_GEN_PARENT(pu,1,RB_RED);

	gu->children[0] = pur;
	if(gur) gur->parent = RBTREE_GEN_PARENT(gu,0,RB_BLACK);
}

static inline void top_rbtree_lr_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu,struct top_rbtree_node* u) 
{
	struct top_rbtree_node* ul = u->children[0];
	struct top_rbtree_node* ur = u->children[1];

	assert(RBTREE_COLOR(gu) == RB_BLACK);
	top_rbtree_set_parent(tree,u,gu->parent);

	u->children[0] = pu;
	pu->parent = RBTREE_GEN_PARENT(u,0,RB_RED);

	u->children[1] = gu;
	gu->parent = RBTREE_GEN_PARENT(u,1,RB_RED);

	pu->children[1] = ul;
	if(ul) ul->parent = RBTREE_GEN_PARENT(pu,1,RB_BLACK);

	gu->children[0] = ur;
	if(ur) ur->parent = RBTREE_GEN_PARENT(gu,0,RB_BLACK);
}

static inline void top_rbtree_rr_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu) 
{
	struct top_rbtree_node* pul = pu->children[0];

	assert(RBTREE_COLOR(gu) == RB_BLACK);
	top_rbtree_node_set_parent(tree,pu,gu->parent);

	pu->children[0] = gu;
	gu->parent = RBTREE_GEN_PARENT(pu,0,RB_RED);

	gu->children[1] = pul;
	if(pul) pul->parent = RBTREE_GEN_PARENT(gu,1,RB_BLACK);
}

static inline void top_rbtree_rl_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu,struct top_rbtree_node* u) 
{
	struct top_rbtree_node* ul = u->children[0];
	struct top_rbtree_node* ur = u->children[1];

	assert(RBTREE_COLOR(gu) == RB_BLACK);
	top_rbtree_set_parent(tree,u,gu->parent);

	u->children[0] = gu;
	gu->parent = RBTREE_GEN_PARENT(u,0,RB_RED);

	u->children[1] = pu;
	pu->parent = RBTREE_GEN_PARENT(u,1,RB_RED);

	pu->children[0] = ur;
	if(ur) ur->parent = RBTREE_GEN_PARENT(pu,0,RB_BLACK);

	gu->children[1] = ul;
	if(ul) ul->parent = RBTREE_GEN_PARENT(gu,1,RB_BLACK);
}

void top_rbtree_link_node(struct top_rbtree* tree,struct top_rbtree_node* node,struct top_rbtree_node* parent,struct top_rbtree_node** link) 
{
	struct top_rbtree_node* gu,*guc;
	node->children[0] = node->children[1] = 0;
	*link = node;
	if(parent){
		unsigned long idx =  link == &parent->children[0] ? 0 : 1;
		node->parent = RBTREE_GEN_PARENT(parent,idx,RB_RED);
	}else{
		node->parent = 0;
		return;
	}
	while(parent && RBTREE_COLOR(parent) == RB_RED) {
		gu = top_rbtree_node_parent(parent);
		assert(RBTREE_COLOR(gu) == RB_BLACK);
		if(RBTREE_RIGHT_CHILD(parent)) {
			guc = gu->children[0];
			if(RBTREE_COLOR(guc) == RB_RED) {
				RBTREE_SET_BLACK(parent);			
				RBTREE_SET_BLACK(guc);			
				if(tree->root == gu) return;
				RBTREE_SET_RED(gu);			
				node = parent;
				parent = gu;
			}else {
				if(RBTREE_RIGHT_CHILD(node)){
					top_rbtree_rr_rotation(tree,gu,parent);
				}else {
					top_rbtree_rl_rotation(tree,gu,parent,node);
				}
				return;
			}
		}else {
			guc = gu->children[1];
			if(RBTREE_COLOR(guc) == RB_RED) {
				RBTREE_SET_BLACK(parent);			
				RBTREE_SET_BLACK(guc);			
				if(tree->root == gu) return;
				RBTREE_SET_RED(gu);			
				node = parent;
				parent = gu;
			}else {
				if(RBTREE_LEFT_CHILD(node)) {
					top_rbtree_ll_rotation(tree,gu,parent);
				}else {
					top_rbtree_lr_rotation(tree,gu,parent,node);
				}
				return;
			}
		}
	}
}

static inline struct top_rbtree_node* top_rbtree_node_first(const struct top_rbtree_node* node)
{
	while(node->children[0]) node = node->children[0];
	return (struct top_rbtree_node*)node;
}

static inline struct top_rbtree_node* top_rbtree_node_last(const struct top_rbtree_node* node)
{
	while(node->children[1]) node = node->children[1];
	return (struct top_rbtree_node*)node;
}

static inline void top_rbtree_replace_node(struct top_rbtree* tree,struct top_rbtree_node* pos,struct top_rbtree_node* mnode)
{
	struct top_rbtree_node* c;
	assert(mnode->children[0] == 0 || 0 == mnode->children[1]);
	top_rbtree_set_parent(tree,mnode,pos->parent);
	c = pos->children[0];	
	if(c) {
		mnode->children[0] = c;
		c->parent = (unsigned long)mnode | (c->parent & 3);
	}
	c = pos->children[1];
	if(c ) {
		mnode->children[1] = c;
		c->parent = (unsigned long)mnode | (c->parent & 3);
	}
}

static inline struct top_rbtree_node* top_rbtree_node_unlink(struct top_rbtree_node* node,struct top_rbtree_node* node_child)
{
	struct top_rbtree_node* parent = (struct top_rbtree_node*)(node->parent & ~3);
	if(node->parent & 1) {
		parent->children[1] = node_child;
		if(node_child) node_child->parent = RBTREE_GEN_PARENT(parent,1,RBTREE_COLOR(node_child->parent));
	}else {
		parent->children[0] = node_child;
		if(node_child) node_child->parent = RBTREE_GEN_PARENT(parent,0,RBTREE_COLOR(node_child->parent));
	}
	return parent;
}

static inline struct top_rbtree_node* top_rbtree_right_rotation(struct top_rbtree* tree, struct top_rbtree_node* parent)
{
	struct top_rbtree_node* node = parent->children[0];
	assert(node);	
	switch(node->bf) {
	case 0:
		top_rbtree_ll_rotation(tree,parent,node);
		node->bf = -1;
		parent->bf = 1;
		return node;
	case 1:
		top_rbtree_ll_rotation(tree,parent,node);
		return node;
	case -1:
		top_rbtree_lr_rotation(tree,parent,node);
		return top_rbtree_node_parent(node);
	default:
		assert(0);
		return 0;
	}
}

static inline struct top_rbtree_node* top_rbtree_left_rotation(struct top_rbtree* tree, struct top_rbtree_node* parent)
{
	struct top_rbtree_node* node = parent->children[1];
	assert(node);
	switch(node->bf){
	case 0:
		top_rbtree_rr_rotation(tree,parent,node);
		node->bf = 1;
		parent->bf = -1;
		return node;
	case -1:
		top_rbtree_rr_rotation(tree,parent,node);
		return node;
	case 1:
		top_rbtree_rl_rotation(tree,parent,node);
		return top_rbtree_node_parent(node);
	default:
		assert(0);
		return 0;
	}
}

void top_rbtree_erase(struct top_rbtree* tree, struct top_rbtree_node* node)
{
	struct top_rbtree_node* mnode;
	struct top_rbtree_node* n;
	struct top_rbtree_node* py,*y,*v;
	int erase_color;
	if(node->children[0] == node->children[1]){
		assert(node->children[0] == 0);
		if(tree->root == node) {
			tree->root = 0;
			return;
		}
		mnode = node;
		erase_color = RBTREE_COLOR(mnode);
		py = top_rbtree_node_unlink(mnode,0);
		y = 0;
	}else if(node->children[1])
		mnode = top_rbtree_node_first(node->children[1]);
		erase_color = RBTREE_COLOR(mnode);
		y = mnode->children[1];
		py = top_rbtree_node_unlink(mnode,y);
		top_rbtree_replace_node(tree,node,mnode);
	}else {
		mnode = top_rbtree_node_last(node->children[0]);
		erase_color = RBTREE_COLOR(mnode);
		y = mnode->children[0];
		py = top_rbtree_node_unlink(mnode,y);
		top_rbtree_replace_node(tree,node,mnode);
	}
	if(py == node) py = mnode;
	if(erase_color == RB_RED || py == tree->root) return;
				
}

struct top_rbtree_node* top_rbtree_first(const struct top_rbtree* tree)
{
	if(tree->root)
		return top_rbtree_node_first(tree->root);
	else
		return 0;
}

struct top_rbtree_node* top_rbtree_node_next(const struct top_rbtree_node* node)
{
	struct top_rbtree_node* parent;

	if(node->children[1]) {
		node = node->children[1];
		while(node->children[0]) node=node->children[0];
		return (struct top_rbtree_node*)node;
	}	 

	while((parent = top_rbtree_node_parent(node)) && RBTREE_RIGHT_CHILD(node)) {
		 node = parent;
	}
	return parent;
}

struct top_rbtree_node* top_rbtree_last(const struct top_rbtree* tree)
{
	if(tree->root)
		return top_rbtree_node_last(tree->root);
	else
		return 0;
}

struct top_rbtree_node* top_rbtree_node_prev(const struct top_rbtree_node* node)
{
	struct top_rbtree_node* parent;

	if(node->children[0]) {
		node = node->children[0];
		while(node->children[1]) node=node->children[1];
		return (struct top_rbtree_node*)node;
	}	 

	while((parent = top_rbtree_node_parent(node)) && RBTREE_LEFT_CHILD(node)) {
		 node = parent;
	}
	return parent;
}
