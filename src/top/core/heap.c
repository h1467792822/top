

#include <top/core/heap.h>

static inline int top_heap_node_s(struct top_heap_node* node) {
	return node ? node->s : 0;
}

static inline void top_heap_node_swap(struct top_heap_node** pn1, struct top_heap_node** pn2) {
	struct top_heap_node* tmp = *pn1;
	*pn1 = *pn2;
	*pn2 = tmp;
}

void top_heap_update_s(struct top_heap_node* parent) {
	int rs,ls,s;
	do{
		rs = top_heap_node_s(parent->right);
		ls = top_heap_node_s(parent->left);
		if(ls < rs) {
			top_heap_node_swap(&parent->left,&parent->right);
			rs = ls;
		}
		s = rs + 1;
		if(s == parent->s) break;
		parent->s = s;
	}while((parent = parent->parent));
}

void top_heap_merge(top_heap_node_less less,struct top_heap_node* parent,struct top_heap_node** root, struct top_heap_node* node) {
	int rs,ls,s;
	do {
		if(0 == *root) {
			*root = node;
			(*root)->parent = parent;
			break;
		}
		if(less(*root,node)) {
			top_heap_node_swap(root,&node);
			(*root)->parent = parent;
		}
		parent = *root;
		root = &(*root)->right;
	}while(1);
	if(parent) top_heap_update_s(parent);
}
