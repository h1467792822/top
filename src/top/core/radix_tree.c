
#include <top/core/radix_tree.h>
#include <stdlib.h> //for malloc/free
#include <string.h> //for memset
#include <stdio.h>

#ifdef TOP_TEST
#define RADIX_TREE_MAP_SHIFT 1
#else
#define RADIX_TREE_MAP_SHIFT 4
#endif

#define RADIX_TREE_MAP_SIZE (1ul << RADIX_TREE_MAP_SHIFT)

#define RADIX_TREE_MAP_MASK (RADIX_TREE_MAP_SIZE - 1)

#define RADIX_TREE_MAX_KEY(height) ((1ul << (height * RADIX_TREE_MAP_SHIFT)))

struct top_radix_tree_node {
	union {	
	void* slots[RADIX_TREE_MAP_SIZE];
	struct {
		struct top_radix_tree_node* next;
		int count;
	};
	};
};

#define RADIX_TREE_PAGE_SIZE (4 * 1024)
#define RADIX_TREE_NODE_COUNT_PER_PAGE (RADIX_TREE_PAGE_SIZE / sizeof(struct top_radix_tree_node) )

static top_error_t top_radix_tree_def_malloc(void* data,unsigned long size,void** palloc)
{
	void* alloc = malloc(size);
	if(alloc) {
		*palloc = alloc;
		return TOP_OK;
	}
	return TOP_ERROR(-1);
}

static void top_radix_tree_def_free(void* data, void* alloc,unsigned long size)
{
	free(alloc);
}

static const struct top_radix_tree_conf g_def_conf = {
	.pfmalloc = top_radix_tree_def_malloc,
	.pffree = top_radix_tree_def_free,
	.max_capacity = (unsigned long)-1,
};


static top_error_t top_radix_tree_alloc_node(struct top_radix_tree* tree,struct top_radix_tree_node** pnode)
{
	struct top_radix_tree_node* node;
	node = tree->cached;
	if(tree->cached) {
		node = tree->cached;
		tree->cached = node->next;
		node->next = 0;
		*pnode = node;
		return TOP_OK;
	}
	if(tree->bulk_alloc && tree->bulk_alloc->count) {
		node = tree->bulk_alloc + tree->bulk_alloc->count--;
		memset(node,0,sizeof(*node));
		*pnode = node;
		return TOP_OK;
	}
	if(tree->conf.max_capacity - RADIX_TREE_PAGE_SIZE > tree->capacity) {
		top_error_t err = tree->conf.pfmalloc(tree->conf.user_data,RADIX_TREE_PAGE_SIZE,(void**)&node);
		if(top_errno(err))
			return err;
		tree->capacity += RADIX_TREE_PAGE_SIZE;
		node->next = tree->bulk_alloc;
		node->count = RADIX_TREE_NODE_COUNT_PER_PAGE - 2;
		tree->bulk_alloc = node;
		node += RADIX_TREE_NODE_COUNT_PER_PAGE - 1;
		memset(node,0,sizeof(*node));
		*pnode = node;
		return TOP_OK;
	}
	return top_make_error(-1);
}

static inline void top_radix_tree_free_node(struct top_radix_tree* tree,struct top_radix_tree_node* node)
{
	node->next = tree->cached;
	tree->cached = node;
}

void top_radix_tree_init(struct top_radix_tree* tree, const struct top_radix_tree_conf* conf)
{
	memset(tree,0,sizeof(*tree));
	if(conf) {
		tree->conf = *conf;
		if(tree->conf.pfmalloc == 0 || tree->conf.pffree == 0){
			tree->conf.pfmalloc = g_def_conf.pfmalloc;
			tree->conf.pffree = g_def_conf.pffree;
		}
	}else {
		tree->conf = g_def_conf;
	}
}

void top_radix_tree_fini(struct top_radix_tree* tree)
{
	struct top_radix_tree_node* next;
	struct top_radix_tree_node* pos = tree->bulk_alloc;
	for(; pos && (next = pos->next, 1); pos = next) {
		tree->conf.pffree(tree->conf.user_data,pos,RADIX_TREE_PAGE_SIZE);
	}
	//memset(tree,0,sizeof(*tree));
}

static inline void** top_radix_tree_find_slot(struct top_radix_tree* tree,unsigned long key)
{
	int height = tree->height;
	if(key > RADIX_TREE_MAX_KEY(height)) return 0;
	struct top_radix_tree_node** pslots = &tree->root; 
	struct top_radix_tree_node* slots;
	int idx = 0;
	unsigned long shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
	do {
		--height;
		slots = *pslots;
		if(slots == 0) return 0;
		idx =  (key >> shift ) & RADIX_TREE_MAP_MASK;
		pslots = (struct top_radix_tree_node**)&slots->slots[idx];
		shift -= RADIX_TREE_MAP_SHIFT;
	}while(height);	
	return (void**)pslots;
}

static inline void top_radix_tree_clear(struct top_radix_tree* tree,unsigned long key,void* data)
{
	void** pdata = top_radix_tree_find_slot(tree,key);
	if(pdata) *pdata = data;
}

static inline top_error_t top_radix_tree_extend(struct top_radix_tree* tree, int height)
{
	struct top_radix_tree_node* root;
	struct top_radix_tree_node* slots;
	struct top_radix_tree_node** pslots;
	top_error_t err;
	root = tree->root;
	pslots = &tree->root;
	for(; tree->height < height; ++tree->height) {
		err = top_radix_tree_alloc_node(tree,&slots);
		if(top_errno(err)) {
			goto fail;
		}
		*pslots = slots;
		pslots = (struct top_radix_tree_node**)&slots->slots[0];	
	}
	*pslots = root;
	err = TOP_OK;
out:
	return err;
fail:
	if(tree->root != root) {
		slots = tree->root;
		tree->root = root;
		do{
			root = slots->slots[0];
			slots->slots[0] = 0;
			top_radix_tree_free_node(tree,slots);
			slots = root;
		}while(slots);
	}
	goto out;
}

static inline void top_radix_tree_shrink_from_top(struct top_radix_tree* tree);

top_error_t top_radix_tree_insert(struct top_radix_tree* tree,unsigned long key,void* data)
{
	top_error_t err;
	if(0 == data) {
		top_radix_tree_clear(tree,key,data);
		return TOP_OK;
	}
	struct top_radix_tree_node** pslots,*slots;
	int idx = 0;
	int height = tree->height;
	while(key >= RADIX_TREE_MAX_KEY(height)) {
		++height;
	}
	if(height > tree->height) {
		if(tree->root) {
			err = top_radix_tree_extend(tree,height);
			if(top_errno(err)) return err;
		}else{
			tree->height = height;
		}
	}

	struct top_radix_tree_node** rollback[height];
	memset(rollback,0,sizeof(*rollback) * height);
	pslots = (struct top_radix_tree_node**)&tree->root;
	unsigned long shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
	do{
		--height;
		slots = *pslots;
		if(slots == 0) {
			err = top_radix_tree_alloc_node(tree,&slots);
			if(top_errno(err)) {
				goto rollback;
			}
			*pslots = slots;
			rollback[height] = pslots;
		}
		idx =  (key >> shift) & RADIX_TREE_MAP_MASK;
		pslots = (struct top_radix_tree_node**)&slots->slots[idx];
		shift -= RADIX_TREE_MAP_SHIFT;
	}while(height);
	*pslots = data;
	return TOP_OK;
rollback:
	for(++height; height < tree->height && rollback[height]; ++height) {
		top_radix_tree_free_node(tree,*rollback[height]);
		*rollback[height] = 0;
	}
	top_radix_tree_shrink_from_top(tree);
	return err;
}

void* top_radix_tree_find(struct top_radix_tree* tree,unsigned long key)
{
	void** pdata = top_radix_tree_find_slot(tree,key);
	return pdata ? *pdata : 0;
}

static inline void top_radix_tree_shrink(struct top_radix_tree* tree, struct top_radix_tree_node*** nodes)
{
	struct top_radix_tree_node* slots;	
	int i,j;
	for(i = 0; i < tree->height; ++i) {
		slots = *nodes[i];
		for(j = 0; j < RADIX_TREE_MAP_SIZE; ++j) {
			if(slots->slots[j]) return;
		}
		*nodes[i] = 0;
		top_radix_tree_free_node(tree,slots);
	}
	top_radix_tree_shrink_from_top(tree);
}

static inline void top_radix_tree_shrink_from_top(struct top_radix_tree* tree)
{
	struct top_radix_tree_node* slots;	
	int j;
	if(0 == tree->root) {
		tree->height = 0;
		return;
	}
	slots = tree->root;
	do {
		for(j = 1; j < RADIX_TREE_MAP_SIZE; ++j) {
			if(slots->slots[j]) return;
		}
		if(tree->height > 1 || slots->slots[0] == 0) {
			tree->root = slots->slots[0];
			--tree->height;
			top_radix_tree_free_node(tree,slots);
		}else {
			return;
		}
	}while(slots);
}

void* top_radix_tree_delete(struct top_radix_tree* tree,unsigned long key)
{
	void* data;
	int height = tree->height;
	if(key >= RADIX_TREE_MAX_KEY(height)) return 0;
	struct top_radix_tree_node** nodes[height];
	struct top_radix_tree_node** pslots = &tree->root;
	struct top_radix_tree_node* slots;
	int idx = 0;
	unsigned long shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
	do {
		--height;
		slots = *pslots;
		if(slots == 0) return 0;
		nodes[height] = pslots;
		idx =  (key >> shift ) & RADIX_TREE_MAP_MASK;
		pslots = (struct top_radix_tree_node**)&slots->slots[idx];
		shift -= RADIX_TREE_MAP_SHIFT;
	}while(height);	
	data = *pslots;
	*pslots = 0;
	top_radix_tree_shrink(tree,nodes);
	return data;
}

