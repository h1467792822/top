
#include <top/core/rbtree.h>
#include <top/core/stddef.h>
#include <stdio.h>
#include <assert.h>

#define RB_RIGHT_CHILD(node) ((node)->parent & 0x01)
#define RB_LEFT_CHILD(node) (!((node)->parent & 0x01))

#define RB_COLOR_MASK (0x02)
#define RB_RED (0x02)
#define RB_BLACK (0x00)

#define RB_POS_MASK (0x01)

#define RB_PARENT_MASK (~(RB_POS_MASK | RB_COLOR_MASK))

#define RB_GEN_PARENT(p,idx,color) ((unsigned long)(color) | (unsigned long)(idx) | (unsigned long)(p))

#define RB_COLOR(n) ( (n)->parent & RB_COLOR_MASK)

#define RB_SET_RED(node) ((node)->parent |= RB_RED)
#define RB_SET_BLACK(node) ((node)->parent &= ~RB_RED)

#define RB_NEW_PARENT(old,new) (((old) & ~RB_PARENT_MASK) | (unsigned long)(new))
#define RB_NEW_POS_PARENT(old,new,idx) (((old) & RB_COLOR_MASK) | (unsigned long)(new) | (idx))

static inline void top_rbtree_set_parent(struct top_rbtree* tree,struct top_rbtree_node* node,unsigned long parent) 
{
	struct top_rbtree_node* pu = (struct top_rbtree_node*)(parent & RB_PARENT_MASK);
	node->parent = parent;
	if(pu) 
		pu->children[parent & RB_POS_MASK] = node;
	else
		tree->root = node;
}

static inline void top_rbtree_ll_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu) 
{
	struct top_rbtree_node* pur = pu->children[1];
	unsigned long color = RB_COLOR(pu);
	top_rbtree_set_parent(tree,pu,gu->parent);

	pu->children[1] = gu;
	gu->parent = RB_GEN_PARENT(pu,1,color);

	gu->children[0] = pur;
	if(pur) pur->parent = RB_NEW_POS_PARENT(pur->parent,gu,0);
}

static inline void top_rbtree_lr_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu,struct top_rbtree_node* w,struct top_rbtree_node* u) 
{
	struct top_rbtree_node* ul = u->children[0];
	struct top_rbtree_node* ur = u->children[1];

	unsigned long color = RB_COLOR(u);
	top_rbtree_set_parent(tree,u,gu->parent);

	u->children[0] = pu;
	pu->parent = RB_NEW_PARENT(pu->parent,u);

	u->children[1] = gu;
	gu->parent = RB_GEN_PARENT(u,1,color);

	w->children[1] = ul;
	if(ul) ul->parent = RB_NEW_POS_PARENT(ul->parent,w,1);

	gu->children[0] = ur;
	if(ur) ur->parent = RB_NEW_POS_PARENT(ur->parent,gu,0);
}

static inline void top_rbtree_rr_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu) 
{
	struct top_rbtree_node* pul = pu->children[0];
	unsigned long color = RB_COLOR(pu);

	top_rbtree_set_parent(tree,pu,gu->parent);

	pu->children[0] = gu;
	gu->parent = RB_GEN_PARENT(pu,0,color);

	gu->children[1] = pul;
	if(pul) pul->parent = RB_NEW_POS_PARENT(pul->parent,gu,1);
}

static inline void top_rbtree_rl_rotation(struct top_rbtree* tree,struct top_rbtree_node* gu,struct top_rbtree_node* pu,struct top_rbtree_node* w,struct top_rbtree_node* u) 
{
	struct top_rbtree_node* ul = u->children[0];
	struct top_rbtree_node* ur = u->children[1];
	
	unsigned long color = RB_COLOR(u);

	top_rbtree_set_parent(tree,u,gu->parent);

	u->children[0] = gu;
	gu->parent = RB_GEN_PARENT(u,0,color);

	u->children[1] = pu;
	pu->parent = RB_NEW_PARENT(pu->parent,u);

	w->children[0] = ur;
	if(ur) ur->parent = RB_NEW_POS_PARENT(ur->parent,w,0);

	gu->children[1] = ul;
	if(ul) ul->parent = RB_NEW_POS_PARENT(ul->parent,gu,1);
}

void top_rbtree_link_node(struct top_rbtree* tree,struct top_rbtree_node* node,struct top_rbtree_node* parent,struct top_rbtree_node** link) 
{
	struct top_rbtree_node* gu,*guc;
	node->children[0] = node->children[1] = 0;
	*link = node;
	if(parent){
		unsigned long idx =  link == &parent->children[0] ? 0 : 1;
		node->parent = RB_GEN_PARENT(parent,idx,RB_RED);
	}else{
		node->parent = 0;
		return;
	}
	while(parent && RB_COLOR(parent) == RB_RED) {
		gu = top_rbtree_node_parent(parent);
		assert(RB_COLOR(gu) == RB_BLACK);
		if(RB_RIGHT_CHILD(parent)) {
			guc = gu->children[0];
			if(guc && RB_COLOR(guc) == RB_RED) {
				RB_SET_BLACK(parent);			
				RB_SET_BLACK(guc);			
				if(tree->root == gu) return;
				RB_SET_RED(gu);			
				node = gu;
				parent = top_rbtree_node_parent(node);
			}else {
				if(RB_RIGHT_CHILD(node)){
					top_rbtree_rr_rotation(tree,gu,parent);
				}else {
					top_rbtree_rl_rotation(tree,gu,parent,parent,node);
				}
				return;
			}
		}else {
			guc = gu->children[1];
			if(guc && RB_COLOR(guc) == RB_RED) {
				RB_SET_BLACK(parent);			
				RB_SET_BLACK(guc);			
				if(tree->root == gu) return;
				RB_SET_RED(gu);			
				node = gu;
				parent = top_rbtree_node_parent(node);
			}else {
				if(RB_LEFT_CHILD(node)) {
					top_rbtree_ll_rotation(tree,gu,parent);
				}else {
					top_rbtree_lr_rotation(tree,gu,parent,parent,node);
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

//check null
static inline int top_rbtree_node_is_red(struct top_rbtree_node* node) {
	return node && (RB_COLOR(node) == RB_RED);
}

static inline int top_rbtree_xb0(struct top_rbtree* tree,struct top_rbtree_node* py,struct top_rbtree_node* v)
{
	RB_SET_RED(v);
	if(py == tree->root) return 1;
	if(RB_COLOR(py) == RB_RED) {
		RB_SET_BLACK(py);
		return 1;
	}
	return 0;
}


void top_rbtree_erase(struct top_rbtree* tree, struct top_rbtree_node* node)
{
	struct top_rbtree_node* py,*y,*v,*w;
	int erase_left;
	int erase_color;
	if(node->children[0] == 0) {
		if(node->children[1]) {
			py = node->children[1];
			erase_color = RB_COLOR(py);
			erase_left = 0;
			top_rbtree_set_parent(tree,py,node->parent);
		}else if(node == tree->root){
			assert(tree->root == node);
			tree->root = 0;
			return;	
		}else{
			erase_color = RB_COLOR(node);
			erase_left = RB_LEFT_CHILD(node);
			py = top_rbtree_node_parent(node);
			py->children[node->parent & RB_POS_MASK] = 0;
		}	
	}else if(node->children[1] == 0){
		py = node->children[0];
		erase_color = RB_COLOR(py);
		erase_left = 1;
		top_rbtree_set_parent(tree,py,node->parent);
	}else{
		y = top_rbtree_node_last(node->children[0]);
		py = top_rbtree_node_parent(y);
		erase_color = RB_COLOR(y);
		if(py == node) {
			erase_left = 1;
			py = y;
			//replace node with y, and node->children[0] == y && y->children[1] == 0
			top_rbtree_set_parent(tree,y,node->parent);
			w = node->children[1];
			y->children[1] = w;
			w->parent = RB_NEW_PARENT(w->parent,y);
		}else {
			erase_left = 0;
			//unlink y, y is the last,so y->children[1] == 0 && py->children[1] == y
			w = y->children[0];
			py->children[1] = w;
			if(w) w->parent = RB_NEW_POS_PARENT(w->parent,py,1);
			//replace node with y, node->children[1] != 0
			top_rbtree_set_parent(tree,y,node->parent);
			y->children[0] = node->children[0];
			y->children[0]->parent = RB_NEW_PARENT(y->children[0]->parent,y);
			y->children[1] = node->children[1];
			y->children[1]->parent = RB_NEW_PARENT(y->children[1]->parent,y);
		}
	}
	if(erase_color == RB_RED || !py) return;
	//erased node is black, so py->children[erase_left] != 0
color:
	if(erase_left){
		v = py->children[1];
		if(RB_COLOR(v) == RB_BLACK) {
			if(top_rbtree_node_is_red(v->children[0])){
				RB_SET_BLACK(v->children[0]);
				top_rbtree_rl_rotation(tree,py,v,v,v->children[0]);
			}else if(top_rbtree_node_is_red(v->children[1])){
				RB_SET_BLACK(v->children[1]);
				top_rbtree_rr_rotation(tree,py,v);
			}else{
				if(top_rbtree_xb0(tree,py,v)) return;
				erase_left = RB_LEFT_CHILD(py);
				py = top_rbtree_node_parent(py);
				goto color;
			}
		}else {
			// v->children[0] != 0 && v->children[1] != 0
			w = v->children[0];
			if(top_rbtree_node_is_red(w->children[0]) ){
				RB_SET_BLACK(w->children[0]);
				top_rbtree_rl_rotation(tree,py,v,w,w->children[0]);		
			}else if(top_rbtree_node_is_red(w->children[1])){
				RB_SET_BLACK(w->children[1]);
				top_rbtree_rl_rotation(tree,py,v,v,w);
			}else{
				RB_SET_BLACK(v);
				RB_SET_RED(w);
				top_rbtree_rr_rotation(tree,py,v);
			}
		}
	}else{
		v = py->children[0];	
		if(RB_COLOR(v) == RB_BLACK) {
			if(top_rbtree_node_is_red(v->children[1])){
				RB_SET_BLACK(v->children[1]);
				top_rbtree_lr_rotation(tree,py,v,v,v->children[1]);
			}else if(top_rbtree_node_is_red(v->children[0])){
				RB_SET_BLACK(v->children[0]);
				top_rbtree_ll_rotation(tree,py,v);
			}else{
				if(top_rbtree_xb0(tree,py,v)) return;
				erase_left = RB_LEFT_CHILD(py);
				py = top_rbtree_node_parent(py);
				goto color;
			}
		}else{
			w = v->children[1];
			if(top_rbtree_node_is_red(w->children[1])){
				RB_SET_BLACK(w->children[1]);
				top_rbtree_lr_rotation(tree,py,v,w,w->children[1]);		
			}else if(top_rbtree_node_is_red(w->children[0])){
				RB_SET_BLACK(w->children[0]);
				top_rbtree_lr_rotation(tree,py,v,v,w);
			}else {
				RB_SET_BLACK(v);
				RB_SET_RED(w);
				top_rbtree_ll_rotation(tree,py,v);
			}
		}
	}
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

	while((parent = top_rbtree_node_parent(node)) && RB_RIGHT_CHILD(node)) {
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

	while((parent = top_rbtree_node_parent(node)) && RB_LEFT_CHILD(node)) {
		 node = parent;
	}
	return parent;
}
