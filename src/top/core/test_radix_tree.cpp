
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/radix_tree.h>
#include <stdlib.h>

using namespace std;

class TestRadixTree: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestRadixTree);
    CPPUNIT_TEST( testOne  );
    CPPUNIT_TEST( testInsertFind  );
    CPPUNIT_TEST( testDelete );
    CPPUNIT_TEST( testMoreData );
    CPPUNIT_TEST_SUITE_END();
    struct top_radix_tree tree;
public:
    void setUp()
    {
        top_radix_tree_init(&tree,0);
        std::cout << endl << __func__ << std::endl;
    }
    void tearDown()
    {
        top_radix_tree_fini(&tree);
    }
    void gen_values(unsigned long* values,int cnt)
    {
        srandom((unsigned long)values);
        for(int i = 0; i < cnt; ++i)
            values[i] = random();
    }
    void testOne()
    {
        top_error_t err;
        err = top_radix_tree_insert(&tree,10240,(void*)10240);
        CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        void* found = top_radix_tree_find(&tree,10240);
        void* del = top_radix_tree_delete(&tree,10240);
        CPPUNIT_ASSERT_EQUAL(found,del);
        CPPUNIT_ASSERT_EQUAL((unsigned long)found,10240ul);
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
        CPPUNIT_ASSERT_EQUAL(tree.height,0);
    }
    void testInsertFind()
    {
        unsigned long values[10];
        gen_values(values,10);
        top_error_t err;
        for(int i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            err = top_radix_tree_insert(&tree,values[i],(void*)values[i]);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
        for(int i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            void* found = top_radix_tree_find(&tree,values[i]);
            CPPUNIT_ASSERT_EQUAL((unsigned long)found,values[i]);
        }
    }
    void testDelete()
    {
        unsigned long values[10];
        gen_values(values,10);
        top_error_t err;
        for(int i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            err = top_radix_tree_insert(&tree,values[i],(void*)values[i]);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
        for(int i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            void* found = top_radix_tree_delete(&tree,values[i]);
            CPPUNIT_ASSERT_EQUAL((unsigned long)found,values[i]);
            found = top_radix_tree_find(&tree,values[i]);
            CPPUNIT_ASSERT_EQUAL((unsigned long)found,(unsigned long)0);
        }
        CPPUNIT_ASSERT_EQUAL(tree.height,0);
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
    }
    void testMoreData()
    {
        int count = 100;
        top_error_t err;
        void** data = (void**)malloc(count * sizeof(*data));
        srandom((unsigned long)data);
        for(int i = 0; i < count; ++i) {
            do {
                data[i] = (void*)random();
            } while(top_radix_tree_find(&tree,(unsigned long)data[i]) != 0);
            err = top_radix_tree_insert(&tree,(unsigned long)data[i],data[i]);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
            void* found = top_radix_tree_find(&tree,(unsigned long)data[i]);
            CPPUNIT_ASSERT_EQUAL((unsigned long)found,(unsigned long)data[i]);
        }
        for(int i = 0; i < count; ++i) {
            void* found = top_radix_tree_delete(&tree,(unsigned long)data[i]);
            CPPUNIT_ASSERT_EQUAL((unsigned long)found,(unsigned long)data[i]);
            found = top_radix_tree_find(&tree,(unsigned long)data[i]);
            CPPUNIT_ASSERT_EQUAL((unsigned long)found,(unsigned long)0);
        }
        CPPUNIT_ASSERT_EQUAL(tree.height,0);
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestRadixTree );
