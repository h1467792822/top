
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/stack.h>
#include <top/core/stddef.h>
#include <limits.h>

struct TestStackNode {
    struct top_stack_node node;
    int i ;
};

class TestStack: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestStack);
    CPPUNIT_TEST( testInit );
    CPPUNIT_TEST( testAll );
    CPPUNIT_TEST_SUITE_END();
    struct top_stack stack;
public:
    void setUp()
    {
        top_stack_init(&stack);
    }
    void tearDown() {}
    void testInit()
    {
        CPPUNIT_ASSERT(top_stack_empty(&stack));
    }

    void testAll()
    {
        CPPUNIT_ASSERT(0 == top_stack_top(&stack));
        TestStackNode node1;
        TestStackNode node2;
        top_stack_push(&stack,&node1.node);
        top_stack_push(&stack,&node2.node);
        top_stack_node* top = top_stack_top(&stack);
        CPPUNIT_ASSERT_EQUAL(top,&node2.node);
        TestStackNode* top_node = top_stack_entry(top,TestStackNode,node);
        CPPUNIT_ASSERT_EQUAL(top_node,&node2);
        top_stack_node* top2 = top_stack_pop(&stack);
        CPPUNIT_ASSERT_EQUAL(top,top2);
        top = top_stack_top(&stack);
        CPPUNIT_ASSERT_EQUAL(top,&node1.node);
        top2 = top_stack_pop(&stack);
        CPPUNIT_ASSERT_EQUAL(top,top2);
        CPPUNIT_ASSERT(top_stack_empty(&stack));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestStack );
