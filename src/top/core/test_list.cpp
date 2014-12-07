
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/list.h>

#define LIST_COUNT_ASSERT(expression,cnt) \
	do {int n = 0; expression { ++n; } CPPUNIT_ASSERT_EQUAL(n,cnt); } while(0)

using namespace std;

struct list_node { int i; top_list_node node; };
class TestList: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestList);
	CPPUNIT_TEST( testEmpty );
	CPPUNIT_TEST( testAddOne );
	CPPUNIT_TEST( testAddTwo );
	CPPUNIT_TEST( testAddTailMore );
	CPPUNIT_TEST( testAddMore );
	CPPUNIT_TEST( testRemove );
	CPPUNIT_TEST( testRangeRemove );
	CPPUNIT_TEST( testListMove );
	CPPUNIT_TEST( testNodeMove );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp() { 
	}
	void check_size(top_list* list,int cnt) {
		top_list_node* pos,*tmp;
		list_node* tpos,*ttmp;
		LIST_COUNT_ASSERT(top_list_for_each(list,pos),cnt);
		LIST_COUNT_ASSERT(top_list_for_each_reverse(list,pos),cnt);
		LIST_COUNT_ASSERT(top_list_for_each_safe(list,pos,tmp),cnt);
		LIST_COUNT_ASSERT(top_list_for_each_safe_reverse(list,pos,tmp),cnt);
		LIST_COUNT_ASSERT(top_list_for_each_entry(list,tpos,node),cnt);
		LIST_COUNT_ASSERT(top_list_for_each_entry_reverse(list,tpos,node),cnt);
		LIST_COUNT_ASSERT(top_list_for_each_entry_safe(list,tpos,node,ttmp),cnt);
		LIST_COUNT_ASSERT(top_list_for_each_entry_safe_reverse(list,tpos,node,ttmp),cnt);
		pos = list->first;
		LIST_COUNT_ASSERT(top_list_for_each_from(list,pos),cnt);
		pos = list->last;
		LIST_COUNT_ASSERT(top_list_for_each_reverse_from(list,pos),cnt);
		pos = list->first;
		LIST_COUNT_ASSERT(top_list_for_each_safe_from(list,pos,tmp),cnt);
		pos = list->last;
		LIST_COUNT_ASSERT(top_list_for_each_safe_reverse_from(list,pos,tmp),cnt);
		tpos = top_list_entry(list->first,list_node,node);
		LIST_COUNT_ASSERT(top_list_for_each_entry_from(list,tpos,node),cnt);
		tpos = top_list_entry(list->last,list_node,node);
		LIST_COUNT_ASSERT(top_list_for_each_entry_reverse_from(list,tpos,node),cnt);
		tpos = top_list_entry(list->first,list_node,node);
		LIST_COUNT_ASSERT(top_list_for_each_entry_safe_from(list,tpos,node,ttmp),cnt);
		tpos = top_list_entry(list->last,list_node,node);
		LIST_COUNT_ASSERT(top_list_for_each_entry_safe_reverse_from(list,tpos,node,ttmp),cnt);

	}
	void tearDown(){}
	void testEmpty() {
		top_list list;
		top_list_init(&list);
		check_size(&list,0);
		CPPUNIT_ASSERT(top_list_empty(&list));
	}
	void testAddOne() {
		top_list list = TOP_LIST_INIT(list);
		list_node node = {.i = 100, };
		top_list_add(&list,&node.node);
		check_size(&list,1);
		CPPUNIT_ASSERT_EQUAL(list.first,list.last);
		CPPUNIT_ASSERT_EQUAL(list.first,&node.node);
		CPPUNIT_ASSERT(top_list_singular(&list));
	}
	void testAddTwo() {
		top_list list = TOP_LIST_INIT(list);
		list_node nodes[2];
#if 1
		for(int i = 0; i < 2; ++i)
			top_list_add(&list,&nodes[i].node);
		check_size(&list,2);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[1].node);
		CPPUNIT_ASSERT_EQUAL(list.last,&nodes[0].node);
#else
		top_list_init(&list);
		for(int i = 0; i < 2; ++i)
			top_list_add_tail(&list,&nodes[i].node);
		check_size(&list,2);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.last,&nodes[1].node);
#endif
	}
	void testAddMore() {
		int cnt = 100;
		list_node nodes[cnt];
		top_list list = TOP_LIST_INIT(list);
		for(int i = 0; i < cnt; ++i){
			top_list_add(&list,&nodes[i].node);
			nodes[i].i = i;
		}
		check_size(&list,cnt);
		CPPUNIT_ASSERT_EQUAL(list.last,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[cnt - 1].node);
		list_node* n;
		int v = cnt;
		top_list_for_each_entry(&list,n,node){
			CPPUNIT_ASSERT_EQUAL(n->i,--v);
		}
		CPPUNIT_ASSERT_EQUAL(0,v);
	}
	void testAddTailMore() {
		int i;
		top_list list = TOP_LIST_INIT(list);
		list_node nodes[100];
		for(i = 0; i < 100; ++i){
			nodes[i].i = i;
			top_list_add_tail(&list,&nodes[i].node);
		}
		check_size(&list,100);

		list_node* n;
		n = top_list_entry(list.last,list_node,node);
		CPPUNIT_ASSERT_EQUAL(99,n->i);
		n = top_list_entry(list.first,list_node,node);
		CPPUNIT_ASSERT_EQUAL(0,n->i);
		i = 100;
		top_list_for_each_entry_reverse(&list,n,node) {
			CPPUNIT_ASSERT_EQUAL(n->i,--i);
		}	
		CPPUNIT_ASSERT_EQUAL(i,0);
	}
	void testRemove()
	{
		int cnt = 100;
		list_node nodes[cnt];
		top_list list = TOP_LIST_INIT(list);
		for(int i = 0; i < cnt; ++i)
			top_list_add_tail(&list,&nodes[i].node);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		check_size(&list,cnt);
		top_list_node_remove(&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[1].node);
		check_size(&list,cnt - 1);
		top_list_node_remove(&nodes[cnt - 1].node);
		CPPUNIT_ASSERT_EQUAL(list.last,&nodes[cnt - 2].node);
		check_size(&list,cnt - 2);
		for(int i = 1; i < cnt - 1; ++i){
			top_list_node_remove(&nodes[i].node);
		}
		check_size(&list,0);
	}

	void testRangeRemove()
	{
		int cnt = 100;
		list_node nodes[cnt];
		top_list list = TOP_LIST_INIT(list);
		for(int i = 0; i < cnt; ++i)
			top_list_add_tail(&list,&nodes[i].node);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		check_size(&list,cnt);
		top_list_range_remove(&nodes[1].node,&nodes[cnt - 2].node);
		check_size(&list,2);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.last,&nodes[cnt - 1].node);
		top_list_range_remove(&nodes[0].node,&nodes[cnt - 1].node);
		check_size(&list,0);

	}

	void testListMove()
	{
		int cnt = 100;
		list_node nodes[cnt];
		top_list list = TOP_LIST_INIT(list);
		for(int i = 0; i < cnt; ++i)
			top_list_add_tail(&list,&nodes[i].node);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		check_size(&list,cnt);
		top_list list2 = TOP_LIST_INIT(list2);
		top_list_move(&list,&list2);
		CPPUNIT_ASSERT_EQUAL(list2.first,&nodes[0].node);
		check_size(&list2,cnt);
	}
	void testNodeMove()
	{
		int cnt = 100;
		list_node nodes[cnt];
		top_list list = TOP_LIST_INIT(list);
		for(int i = 0; i < cnt; ++i)
			top_list_add_tail(&list,&nodes[i].node);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		check_size(&list,cnt);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);

		top_list list2 = TOP_LIST_INIT(list2);
		top_list_node_move(&nodes[0].node,&list2);
		check_size(&list2,1);
		check_size(&list,cnt - 1);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[1].node);
		CPPUNIT_ASSERT_EQUAL(list2.first,&nodes[0].node);

		top_list_node_move_tail(&nodes[1].node,&list2);
		CPPUNIT_ASSERT_EQUAL(list2.first,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list2.last,&nodes[1].node);
		top_list_node_move(&nodes[2].node,&list2);
		CPPUNIT_ASSERT_EQUAL(list2.first,&nodes[2].node);
		CPPUNIT_ASSERT_EQUAL(list2.last,&nodes[1].node);

		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[3].node);
		check_size(&list2,3);
		check_size(&list,cnt - 3);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestList );
