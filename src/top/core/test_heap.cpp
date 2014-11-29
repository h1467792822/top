
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/heap.h>
#include <top/core/stddef.h>
#include <limits.h>

struct heap_node {
	struct top_heap_node node;
	int value;
	int idx;
};

static int less(struct top_heap_node* n1,struct top_heap_node* n2) 
{
	struct heap_node* hn1 = top_container_of(n1,struct heap_node,node);
	struct heap_node* hn2 = top_container_of(n2,struct heap_node,node);
	return hn1->value < hn2->value;
}


static void print_heap(top_heap* heap) {
	struct top_heap tmp;
	top_heap_init(&tmp,heap->less);
	heap_node* hn;
	using namespace std;
	cout << "heap: ";
	while(heap->max) {
		hn = top_container_of(heap->max,heap_node,node);	
		cout << hn->value << " "; 
		top_heap_del_max(heap);
		top_heap_insert(&tmp,&hn->node);
	}
	heap->max = tmp.max;
	cout << endl;
}


class TestHeap: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestHeap);
	CPPUNIT_TEST( testInsertAndDelMax );
	CPPUNIT_TEST( testInsertAndDelAny );
	CPPUNIT_TEST_SUITE_END();
	struct top_heap heap;
public:
	void setUp() { top_heap_init(&heap,less); std::cout << "setUp()" << std::endl; }
	void tearDown(){}
	void testValues(int* values,int cnt,const char* msg) {
		struct heap_node nodes[cnt];
		int max = INT_MIN;
		for(int i = 0; i < cnt; ++i) {
			if(max < values[i]) max = values[i];
			nodes[i].idx = i;
			nodes[i].value = values[i];
			top_heap_insert(&heap,&nodes[i].node);	
		}
		while(heap.max) {
		struct heap_node* node = top_container_of(top_heap_del_max(&heap),heap_node,node);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(msg,node->value,values[node->idx]);
		CPPUNIT_ASSERT_MESSAGE(msg,node->value <= max);	
		max = node->value;
		}
	}
	void testInsertAndDelMax() {
		int values[] = { 0,1,2,3,4,5,6,10,9,8,7,6,5,2,100,0,-1,99,98,-2,50,66,40 };
		testValues(values,sizeof(values)/sizeof(values[0]),"values");
	}
	void testInsertAndDelAny(int* values,int cnt,int* del_idx,int del_cnt,const char* msg) {
		struct heap_node nodes[cnt];
		int max = INT_MIN;
		for(int i = 0; i < cnt; ++i) {
			if(max < values[i]) max = values[i];
			nodes[i].idx = i;
			nodes[i].value = values[i];
			top_heap_insert(&heap,&nodes[i].node);	
		}

		for(int i = 0; i < del_cnt; ++i) top_heap_del(&heap,&nodes[del_idx[i]].node);

		while(heap.max) {
		struct heap_node* node = top_container_of(top_heap_del_max(&heap),heap_node,node);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(msg,node->value,values[node->idx]);
		CPPUNIT_ASSERT_MESSAGE(msg,node->value <= max);	
		max = node->value;
		}
	}
	void testInsertAndDelAny() {
		int values[] = { 0,1,2,3,4,5,6,10,9,8,7,6,5,2,100,0,-1,99,98,-2,50,66,40 };
		int del_idx[] = { 4,9,10,5,0,sizeof(values)/sizeof(values[0]) - 1,15 };
		testInsertAndDelAny(values,sizeof(values)/sizeof(int),del_idx,sizeof(del_idx)/sizeof(int),"del_values");
		
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestHeap );
