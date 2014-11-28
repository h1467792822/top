
#include <top/core/stddef.h>
#include <top/core/heap.h>
#include <stdlib.h>
#include <iostream>
#include <top/core/list.h>

struct top_heap heap;

struct heap_node {
	struct top_heap_node node;
	int value;
};

static int less(struct top_heap_node* n1,struct top_heap_node* n2) 
{
	struct heap_node* hn1 = top_container_of(n1,struct heap_node,node);
	struct heap_node* hn2 = top_container_of(n2,struct heap_node,node);
	return hn1->value < hn2->value;
}

int main(int argc,char* argv[]) {
	heap_node nodes[argc - 1];
	top_heap_init(&heap,less);
	for(int i  = 1; i < argc; ++i) {
		nodes[i - 1].value = atoi(argv[i]);
		top_heap_insert(&heap,&nodes[i - 1].node);
	}
	heap_node* hn;
	using namespace std;
	cout << "heap: ";
	while(heap.max) {
		hn = top_container_of(heap.max,heap_node,node);	
		cout << hn->value << " "; 
		top_heap_del(&heap);
	}
	cout << endl;
}
