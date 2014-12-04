#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/rbtree.h>
#include <top/core/stddef.h>
#include <limits.h>
#include <stdlib.h>

using namespace std;

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

static void check_tree_node_color(top_rbtree_node* node,int* black_color,int red_parent,int* black_num)
{
	if(!node) {
		if(*black_num == 0) {
			*black_num = *black_color;
		}else{
			CPPUNIT_ASSERT_EQUAL(*black_num,*black_color);
		}
		return;
	}
	int color = node->parent & COLOR_MASK;
	if(red_parent) {
		CPPUNIT_ASSERT_EQUAL(color,BLACK);
	}
	if(color == BLACK) {
		++*black_color;
	}
	check_tree_node_color(node->children[0],black_color,color == RED,black_num);
	check_tree_node_color(node->children[1],black_color,color == RED,black_num);
	if(color == BLACK) {
		--*black_color;
	}
}

static void check_tree(top_rbtree* tree) {
	int black_color = 1;
	int black_num = 0;
	check_tree_node_color(tree->root,&black_color,1,&black_num);
	check_tree_node_next(top_rbtree_first(tree),INT_MIN);
	check_tree_node_prev(top_rbtree_last(tree),INT_MAX);
}

static void printf_tree(tree_node* root,int tabs,const char* msg) {
	if(tabs == 0) cout << endl;
	for(int i = 0; i < tabs; ++i) cout << "\t";
	cout << tabs << ":" << msg << ":";
	if(!root) { cout << " NULL" ;return;}
	cout << root->value << ": " << ((root->node.parent & 2) ? "RED" : "BLACK");
	cout << endl;
	printf_tree((tree_node*)root->node.children[0], tabs + 1,"left");	
	cout << endl;
	printf_tree((tree_node*)root->node.children[1], tabs + 1,"right");	
}

static void print_tree(top_rbtree* tree) {
	struct top_rbtree_node* node = top_rbtree_first(tree);
	struct tree_node* tnode;
	printf_tree(top_container_of(tree->root,tree_node,node),0,"root");	
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
			parent = parent->children[0];	
		else
			parent = parent->children[1];	
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
			p = &parent->children[0];
		}else if(tnode->value < node->value){
			p = &parent->children[1];
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
	CPPUNIT_TEST( testInsertMoreDelReverse );
	CPPUNIT_TEST_SUITE_END();
	struct top_rbtree tree;
public:
	void setUp() { tree.root = 0; }
	void tearDown(){}
	void testValues(int* values,tree_node* nodes,int cnt,const char* msg) {
		for(int i = 0; i < cnt; ++i) {
			nodes[i].idx = i;
			nodes[i].value = values[i];
			print_tree(&tree);
			cout << endl << " +++ insert : " << i << ": " << values[i] << endl;
			tree_insert(&tree,&nodes[i]);	
			check_tree(&tree);
			cout << " --- END OF insert : " << i << ": " << values[i] << endl;
			print_tree(&tree);
		}
		print_tree(&tree);
	}
	void testDel(int* values,tree_node* unused,int cnt,int start,int step,const char* msg) {
		struct tree_node* found;
		struct tree_node nodes[cnt];
		for(int i = start; i < cnt && i >= 0; i += step) {
			nodes[i] .value = values[i];
			found = tree_find(&tree,&nodes[i]);
			if(found) {
				print_tree(&tree);
				cout << endl << " +++ DEL: " << values[i] << endl;	
				top_rbtree_erase(&tree,&found->node);
				cout << " --- END of DEL: " << values[i] << endl;	
				check_tree(&tree);
			}
		} 
		print_tree(&tree);
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
		static long seed = (long)values;
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
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestRbTree );
