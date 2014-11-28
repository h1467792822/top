
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

static inline void top_heap_init(struct top_heap* heap,top_heap_node_less less) {
	heap->max = 0;
	heap->less = less;
}

void top_heap_merge(top_heap_node_less less,struct top_heap_node** root, struct top_heap_node* node) ;

static inline void top_heap_insert(struct top_heap* heap, struct top_heap_node* node) {
	node->s = 0;
	node->left = node->right = 0;
	top_heap_merge(heap->less,&heap->max,node);
}

static inline struct top_heap_node* top_heap_del(struct top_heap* heap){
	struct top_heap_node* ret = heap->max;
	if(ret) {
		struct top_heap_node* node = heap->max->right;
		heap->max = heap->max->left;
		if(node) top_heap_merge(heap->less,&heap->max,node);
	}
	return ret;
}


#ifdef __cplusplus
}
#endif

#endif

