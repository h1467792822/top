
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/hlist.h>

#define LIST_COUNT_ASSERT(expression,cnt) \
	do {int n = 0; expression { ++n; } CPPUNIT_ASSERT_EQUAL(n,cnt); } while(0)

using namespace std;

struct list_node { int i; top_hlist_node node; };
class TestHList: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestHList);
	CPPUNIT_TEST( testEmpty );
	CPPUNIT_TEST( testAddOne );
	CPPUNIT_TEST( testAddTwo );
	CPPUNIT_TEST( testAddBefore );
	CPPUNIT_TEST( testAddAfter );
	CPPUNIT_TEST( testDel );
	CPPUNIT_TEST( testMove );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp() { 
	}
	void check_size(top_hlist* list,int cnt) {
		top_hlist_node* pos,*tmp;
		list_node* tpos;
		LIST_COUNT_ASSERT(top_hlist_for_each(list,pos),cnt);
		LIST_COUNT_ASSERT(top_hlist_for_each_safe(list,pos,tmp),cnt);
		pos = list->first;
		LIST_COUNT_ASSERT(top_hlist_for_each_from(pos),cnt);
		pos = list->first;
		LIST_COUNT_ASSERT(top_hlist_for_each_safe_from(pos,tmp),cnt);

		LIST_COUNT_ASSERT(top_hlist_for_each_entry(list,tpos,node,pos),cnt);
		LIST_COUNT_ASSERT(top_hlist_for_each_entry_safe(list,tpos,node,pos,tmp),cnt);

		if(cnt > 0){
			tpos = top_hlist_entry(list->first,list_node,node);
			LIST_COUNT_ASSERT(top_hlist_for_each_entry_continue(tpos,node,pos),cnt - 1);
			tpos = top_hlist_entry(list->first,list_node,node);
			LIST_COUNT_ASSERT(top_hlist_for_each_entry_safe_continue(tpos,node,pos,tmp),cnt - 1);
		}
	}
	void tearDown(){}
	void testEmpty() {
		top_hlist list;
		top_hlist_init(&list);
		check_size(&list,0);
		CPPUNIT_ASSERT(top_hlist_empty(&list));
	}
	void testAddOne() {
		top_hlist list = TOP_HLIST_INIT();
		list_node node = {.i = 100, };
		top_hlist_add_head(&list,&node.node);
		CPPUNIT_ASSERT_EQUAL(node.node.pprev,&list.first);
		CPPUNIT_ASSERT_EQUAL(list.first,&node.node);
		CPPUNIT_ASSERT_EQUAL(node.node.next,(struct top_hlist_node*)0);
		check_size(&list,1);
	}
	void testAddTwo() {
		top_hlist list = TOP_HLIST_INIT();
		list_node nodes[2];
		for(int i = 0; i < 2; ++i)
			top_hlist_add_head(&list,&nodes[i].node);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[1].node);
		CPPUNIT_ASSERT_EQUAL(nodes[1].node.next,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(nodes[0].node.next,(struct top_hlist_node*)0);
		CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		check_size(&list,2);
	}
	void testAddAfter() {
		top_hlist list = TOP_HLIST_INIT();
		list_node nodes[2];
		top_hlist_add_head(&list,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		top_hlist_add_after(&nodes[1].node,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(nodes[0].node.next,&nodes[1].node);
		CPPUNIT_ASSERT_EQUAL(nodes[1].node.next,(struct top_hlist_node*)0);
		check_size(&list,2);
	}
	void testAddBefore() {
		top_hlist list = TOP_HLIST_INIT();
		list_node nodes[3];
		top_hlist_add_head(&list,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		top_hlist_add_after(&nodes[1].node,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		check_size(&list,2);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(nodes[0].node.next,&nodes[1].node);
		CPPUNIT_ASSERT_EQUAL(nodes[1].node.next,(struct top_hlist_node*)0);
		top_hlist_add_before(&nodes[2].node,&nodes[1].node);
		check_size(&list,3);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(nodes[0].node.next,&nodes[2].node);
		CPPUNIT_ASSERT_EQUAL(nodes[2].node.next,&nodes[1].node);
		CPPUNIT_ASSERT_EQUAL(nodes[1].node.next,(struct top_hlist_node*)0);
	}
	void testDel() {
		int i;
		int cnt = 100;
		top_hlist list = TOP_HLIST_INIT();
		list_node nodes[cnt];
		for(i = 0; i < cnt; ++i){
			nodes[i].i = i;
			top_hlist_add_head(&list,&nodes[i].node);
			CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		}
		check_size(&list,cnt);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[cnt - 1].node);
		top_hlist_node_del(&nodes[cnt - 1].node);
		CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[cnt - 2].node);
		check_size(&list,cnt - 1);
		CPPUNIT_ASSERT_EQUAL(list.first,&nodes[cnt - 2].node);
		top_hlist_node_del(&nodes[0].node);
		CPPUNIT_ASSERT_EQUAL(list.first->pprev,&list.first);
		check_size(&list,cnt - 2);
		CPPUNIT_ASSERT_EQUAL(nodes[1].node.next,(struct top_hlist_node*)0);
		
		for(i = 1; i < cnt - 1; ++i)
			top_hlist_node_del(&nodes[i].node);
		check_size(&list,0);
	}
	void testMove() {
		int i;
		int cnt = 100;
		top_hlist list = TOP_HLIST_INIT();
		list_node nodes[cnt];
		for(i = 0; i < cnt; ++i){
			nodes[i].i = i;
			top_hlist_add_head(&list,&nodes[i].node);
		}
		check_size(&list,cnt);
		
		TOP_HLIST(list2);
		top_hlist_move(&list,&list2);
		check_size(&list,0);
		check_size(&list2,cnt);

		top_hlist_node_del(list2.first);
		check_size(&list2,cnt - 1);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestHList );
