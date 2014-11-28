

#include <top/core/heap.h>

void top_heap_merge(top_heap_node_less less,struct top_heap_node** root, struct top_heap_node* node) {
	if(!*root ) {
		*root = node;
		return;
	}
	if(less(*root,node)) {
		struct top_heap_node* tmp = *root;
		*root = node;
		node = tmp;
	}
	if(node) top_heap_merge(less,&(*root)->right,node);
	if(!(*root)->left) {
		(*root)->left = (*root)->right;
		(*root)->right = 0;
		(*root)->s = 1;
	}else {
		if((*root)->left->s < (*root)->right->s){
			struct top_heap_node* tmp = (*root)->right;
			(*root)->right = (*root)->left;
			(*root)->left = tmp;	
		}
		(*root)->s = (*root)->right->s + 1;
	}
}

