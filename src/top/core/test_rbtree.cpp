
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/avltree.h>
#include <top/core/stddef.h>
#include <limits.h>

using namespace std;

struct tree_node {
	struct top_avltree_node node;
	int value;
	int idx;
};

static void check_tree_node_next(top_avltree_node* node,int max) {
	tree_node* tnode = (tree_node*)node;
	if(tnode){
	CPPUNIT_ASSERT(tnode->value > max);
	CPPUNIT_ASSERT(node->bf >= -1 && node->bf <= 1);
	check_tree_node_next(top_avltree_node_next(node),tnode->value);
	}
}

static void check_tree_node_prev(top_avltree_node* node,int min) {
	tree_node* tnode = (tree_node*)node;
	if(tnode){
	CPPUNIT_ASSERT(tnode->value < min);
	CPPUNIT_ASSERT(node->bf >= -1 && node->bf <= 1);
	check_tree_node_prev(top_avltree_node_prev(node),tnode->value);
	}
}

static void check_tree(top_avltree* tree) {
	check_tree_node_next(top_avltree_first(tree),INT_MIN);	
	check_tree_node_prev(top_avltree_last(tree),INT_MAX);	
}

static void printf_tree(tree_node* root,int tabs,const char* msg) {
	if(tabs == 0) cout << endl;
	for(int i = 0; i < tabs; ++i) cout << "\t";
	cout << tabs << ":" << msg << ":";
	if(!root) { cout << " NULL" ;return;}
	cout << root->value;
	cout << endl;
	printf_tree((tree_node*)root->node.children[0], tabs + 1,"left");	
	cout << endl;
	printf_tree((tree_node*)root->node.children[1], tabs + 1,"right");	
}

static void print_tree(top_avltree* tree) {
	struct top_avltree_node* node = top_avltree_first(tree);
	struct tree_node* tnode;
	printf_tree(top_container_of(tree->root,tree_node,node),0,"root");	
}

tree_node* tree_find(struct top_avltree* tree,tree_node* node)
{
	struct top_avltree_node* parent = tree->root;
	struct tree_node* tnode;
	while(parent) {
		tnode = top_container_of(parent,tree_node,node);
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

tree_node* tree_insert(struct top_avltree* tree,tree_node* node)
{
	struct top_avltree_node** p = &tree->root;
	struct top_avltree_node* parent = *p;
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
	top_avltree_link_node(tree,&node->node,parent,p);
	return node;
}

class TestAvlTree: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestAvlTree);
	CPPUNIT_TEST( testInsertOne );
	CPPUNIT_TEST( testInsertTwo );
	CPPUNIT_TEST( testInsertMore );
	CPPUNIT_TEST( testInsertOneDel );
	CPPUNIT_TEST( testInsertTwoDel );
	CPPUNIT_TEST( testInsertMoreDel );
	CPPUNIT_TEST( testInsertMoreDelReverse );
	CPPUNIT_TEST_SUITE_END();
	struct top_avltree tree;
public:
	void setUp() { tree.root = 0; }
	void tearDown(){}
	void testValues(int* values,tree_node* nodes,int cnt,const char* msg) {
		for(int i = 0; i < cnt; ++i) {
			nodes[i].idx = i;
			nodes[i].value = values[i];
			tree_insert(&tree,&nodes[i]);	
			check_tree(&tree);
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
				top_avltree_erase(&tree,&found->node);
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
	void testInsertMore() {
		int values[] = { 0,1,2,3,4,5,6,10,9,8,7,6,5,2,100,0,-1,99,98,-2,50,66,40 };
		int cnt = sizeof(values)/sizeof(values[0]);
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
		int values[] = { 0,1,2,3,4,5,6,10,9,8,7,6,5,2,100,0,-1,99,98,-2,50,66,40 };
		int cnt = sizeof(values)/sizeof(values[0]);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
		testDel(values,nodes,cnt,cnt - 2,-2,"del values");
	}
	void testInsertMoreDel() {
		int values[] = { 0,1,2,3,4,5,6,10,9,8,7,6,5,2,100,0,-1,99,98,-2,50,66,40 };
		int cnt = sizeof(values)/sizeof(values[0]);
		struct tree_node nodes[cnt];
		testValues(values,nodes,cnt,"values");
		testDel(values,nodes,cnt,1,3,"del values");
		testDel(values,nodes,cnt,0,2,"del values");
		testDel(values,nodes,cnt,0,1,"del values");
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestAvlTree );
