
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
	cout << endl << "root.parent: " << tree->root->parent << endl;
	printf_tree(top_container_of(tree->root,tree_node,node),0,"root");	
	cout << endl << "ordered value: "; 
	while(node) {
		tnode = top_container_of(node,struct tree_node,node);
		cout << " " << tnode->value;
		node = top_avltree_node_next(node);
	}
	cout << endl;

	node = top_avltree_last(tree);
	cout << "reverse ordered values: " ;
	while(node) {
		tnode = top_container_of(node,struct tree_node,node);
		cout << " " << tnode->value;
		node = top_avltree_node_prev(node);
	}
	cout << endl;
}

void tree_insert(struct top_avltree* tree,tree_node* node)
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
			cout << endl << "...exist... !" << endl;
			return;
		}
	}
	cout << endl << "...inserted... !" << endl;
	top_avltree_link_node(tree,&node->node,parent,p);
}

class TestAvlTree: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TestAvlTree);
	CPPUNIT_TEST( testInsertOne );
	CPPUNIT_TEST( testInsertTwo );
	CPPUNIT_TEST( testInsertMore );
	CPPUNIT_TEST_SUITE_END();
	struct top_avltree tree;
public:
	void setUp() { tree.root = 0; }
	void tearDown(){}
	void testValues(int* values,int cnt,const char* msg) {
		struct tree_node nodes[cnt];
		for(int i = 0; i < cnt; ++i) {
			nodes[i].idx = i;
			nodes[i].value = values[i];
			tree_insert(&tree,&nodes[i]);	
		}
		print_tree(&tree);
	}
	void testInsertOne() {
		int values[] = { 100 };
		testValues(values,sizeof(values)/sizeof(values[0]),"values");
	}
	void testInsertTwo() {
		int values[] = { 99,100,101 };
		testValues(values,sizeof(values)/sizeof(values[0]),"values");
	}
	void testInsertMore() {
		int values[] = { 0,1,2,3,4,5,6,10,9,8,7,6,5,2,100,0,-1,99,98,-2,50,66,40 };
		testValues(values,sizeof(values)/sizeof(values[0]),"values");
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestAvlTree );
