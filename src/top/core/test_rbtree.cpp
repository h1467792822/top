#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/rbtree.h>
#include <top/core/stddef.h>
#include <limits.h>
#include <stdlib.h>
#include "test_timer.hpp"

using namespace std;

static int check_print = 0;
struct tree_node {
	struct top_rbtree_node node;
	int value;
	int idx;
};

static void check_tree_node_next(top_rbtree_node* node,int max ) {
	tree_node* tnode = (tree_node*)node;
	if(tnode){
	CPPUNIT_ASSERT(tnode->value > max);
	check_tree_node_next(top_rbtree_node_next(node),tnode->value);
	}
}

static void check_tree_node_prev(top_rbtree_node* node,int min) {
	tree_node* tnode = (tree_node*)node;
	if(tnode){
	CPPUNIT_ASSERT(tnode->value < min);
	check_tree_node_prev(top_rbtree_node_prev(node),tnode->value);
	}
}

#define COLOR_MASK 2
#define BLACK 0
#define RED 2

static void check_tree_node_color(top_rbtree_node* node,int* black_color,int red_parent,int* black_num,int tabs,const char* msg)
{
	tree_node* tnode = (tree_node*)node;
	if(check_print) {
		if(tabs == 0) cout << endl;
		for(int i = 0; i < tabs; ++i) cout << "\t";
		cout << tabs << ":" << msg << ":";
	}
	if(!node) {
		if(check_print) cout << "NULL" << endl;
		if(*black_num == 0) {
			*black_num = *black_color;
		}else{
			CPPUNIT_ASSERT_EQUAL(*black_num,*black_color);
		}
		return;
	}
	if(check_print) {
		cout << tnode->value << ": " << ((node->parent & 2) ? "RED" : "BLACK");
		cout << endl;
	}
	int color = node->parent & COLOR_MASK;
	if(red_parent) {
		CPPUNIT_ASSERT_EQUAL(color,BLACK);
	}
	if(color == BLACK) {
		++*black_color;
	}
	check_tree_node_color(node->left,black_color,color == RED,black_num,tabs + 1,"L");
	if(check_print) cout << endl;
	check_tree_node_color(node->right,black_color,color == RED,black_num,tabs + 1,"R");
	if(color == BLACK) {
		--*black_color;
	}
}

static void check_tree(top_rbtree* tree) {
	int black_color = 1;
	int black_num = 0;
	check_tree_node_color(tree->root,&black_color,1,&black_num,0,"ROOT");
	check_tree_node_next(top_rbtree_first(tree),INT_MIN);
	check_tree_node_prev(top_rbtree_last(tree),INT_MAX);
}

tree_node* tree_find(struct top_rbtree* tree,tree_node* node)
{
	struct top_rbtree_node* parent = tree->root;
	struct tree_node* tnode;
	while(parent) {
		tnode = top_rb_entry(parent,tree_node,node);
		if(tnode->value == node->value){
			 return tnode;
		}
		if(tnode->value > node->value)
			parent = parent->left;	
		else
			parent = parent->right;	
	}
	return 0;
}

tree_node* tree_insert(struct top_rbtree* tree,tree_node* node)
{
	struct top_rbtree_node** p = &tree->root;
	struct top_rbtree_node* parent = *p;
	struct tree_node* tnode;
	while(*p) {
		parent = *p;
		tnode = top_container_of(parent,tree_node,node);
		if(tnode->value > node->value) {
			p = &parent->left;
		}else if(tnode->value < node->value){
			p = &parent->right;
		}else{
			return tnode;
		}
	}
	top_rbtree_link_node(tree,&node->node,parent,p);
	return node;
}

class TestRbTree: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestRbTree);
	CPPUNIT_TEST( testInsertOne );
	CPPUNIT_TEST( testInsertTwo );
	CPPUNIT_TEST( testInsertMore );
	CPPUNIT_TEST( testInsertOneDel );
	CPPUNIT_TEST( testInsertTwoDel );
	CPPUNIT_TEST( testInsertMoreDel );
	CPPUNIT_TEST( testInsertMoreDelRepeat );
	CPPUNIT_TEST( testInsertMoreDelReverse );
	CPPUNIT_TEST( testInsertFindRepeat );
	CPPUNIT_TEST_SUITE_END();
	struct top_rbtree tree;
public:
	void setUp() { tree.root = 0; }
	void tearDown(){}
	void testValues(int* values,tree_node* nodes,int cnt,const char* msg) {
		for(int i = 0; i < cnt; ++i) {
			nodes[i].idx = i;
			nodes[i].value = values[i];
			check_tree(&tree);
			if(check_print) {
				cout << endl << " +++ insert : " << i << ": " << values[i] << endl;
			}
			tree_insert(&tree,&nodes[i]);	
			check_tree(&tree);
			if(check_print) {
				cout << " --- END OF insert : " << i << ": " << values[i] << endl;
			}
		}
		check_tree(&tree);
	}
	void testDel(int* values,tree_node* unused,int cnt,int start,int step,const char* msg) {
		struct tree_node* found;
		struct tree_node nodes[cnt];
		for(int i = start; i < cnt && i >= 0; i += step) {
			nodes[i] .value = values[i];
			found = tree_find(&tree,&nodes[i]);
			if(found) {
				check_tree(&tree);
				if(check_print) {
					cout << endl << " +++ DEL: " << values[i] << endl;	
				}
				top_rbtree_erase(&tree,&found->node);
				if(check_print) {
					cout << " --- END of DEL: " << values[i] << endl;	
				}
				check_tree(&tree);
			}
		} 
		check_tree(&tree);
	}
	void testInsertOne() {
		int values[] = { 100 };
		int cnt = sizeof(values)/sizeof(values[0]);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
	}
	void testInsertTwo() {
		int values[] = { 99,100,101 };
		int cnt = sizeof(values)/sizeof(values[0]);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
	}
	void gen_values(int * values,int cnt) {
		static long seed = 0;
		seed += 31;
		srandom(seed);
		for(int i = 0; i < cnt; ++i) {
			values[i] = (int)random();
		}
	}
	void testInsertMore() {
		int values[100];
		int cnt = sizeof(values)/sizeof(values[0]);
		gen_values(values,cnt);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
	}
	void testInsertOneDel() {
		int values[] = { 100 };
		int cnt = sizeof(values)/sizeof(values[0]);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
		testDel(values,nodes,cnt,0,1,"del values");
	}
	void testInsertTwoDel() {
		int values[] = { 99,100,101 };
		int cnt = sizeof(values)/sizeof(values[0]);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
		testDel(values,nodes,cnt,0,1,"del values");
	}
	void testInsertMoreDelReverse() {
		int values[99];
		int cnt = sizeof(values)/sizeof(values[0]);
		gen_values(values,cnt);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
		testDel(values,nodes,cnt,cnt - 2,-2,"del values");
	}
	void testInsertMoreDel() {
		int values[101];
		int cnt = sizeof(values)/sizeof(values[0]);
		gen_values(values,cnt);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
		testDel(values,nodes,cnt,1,3,"del values");
		testDel(values,nodes,cnt,0,2,"del values");
		testDel(values,nodes,cnt,0,1,"del values");
	}
	void testInsertMoreDelRepeat() {
		int times = 100;
		int check = check_print;
		check_print = 0;
		{
		top::test::Timer tm;
		for(int i = 0; i < times; ++i) {
			setUp();
			testInsertMoreDel();
			tearDown();
		}
		}
		check_print = check;
	}
        void testFind(int * values,int cnt)  {
                tree_node nodes[cnt];
                tree_node* found;
                for(int i = 0; i < cnt; ++i) {
                        nodes[i].value = values[i];
                        found = tree_find(&tree,&nodes[i]);
                        CPPUNIT_ASSERT(found);
                        CPPUNIT_ASSERT(found != &nodes[i]);
                }
        }
        void testInsertFindRepeat() {
                int times = 100;
                int values[1000];
                int cnt = sizeof(values)/sizeof(values[0]);
		tree_node nodes[cnt];
                gen_values(values,cnt);
                testValues(values,nodes,cnt,"values");
                {
                top::test::Timer tm;
                for(int i = 0; i < times; ++i) {
                        testFind(values,cnt);
                }
                }
        }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestRbTree );
