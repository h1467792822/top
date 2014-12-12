
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/prefix_tree.h>
#include <stdlib.h>

using namespace std;

static const char* keys[] = {
    "http://www.huawei.com",
    "http://w3.huawei.com/next/indexa.html",
    "http://w3.huawei.com/info/cn/doc/viewDoc.do?did=5759291&cata=21073",
    "https://www.huawei.com/w3",
    "http://3ms.huawei.com/hi/index.php?app=home&mod=Info&act=accountset&type=email",
    "https://www.baidu.com/index.html",
    "https://www.google.com/index.html",
};

static const int keys_cnt = sizeof(keys)/ sizeof(keys[0]);

class TestPrefixTree: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestPrefixTree);
    CPPUNIT_TEST( testOne  );
    CPPUNIT_TEST( testInsertFind  );
    CPPUNIT_TEST( testDelete );
    CPPUNIT_TEST( testVisit );
    CPPUNIT_TEST( testVisitWithoutSuffix );
    CPPUNIT_TEST_SUITE_END();
    struct top_prefix_tree tree;
public:
    void setUp()
    {
        top_prefix_tree_init(&tree,0);
        std::cout << endl << __func__ << std::endl;
    }
    void tearDown()
    {
        top_prefix_tree_fini(&tree);
    }
    void testOne()
    {
        top_error_t err;
        int idx = keys_cnt/2;
        err = top_prefix_tree_simple_insert(&tree,keys[idx],(void*)1);
        CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        void* found = top_prefix_tree_simple_find(&tree,keys[idx]);
        void* del = top_prefix_tree_simple_delete(&tree,keys[idx]);
        CPPUNIT_ASSERT_EQUAL(found,del);
        CPPUNIT_ASSERT_EQUAL((unsigned long)found,1ul);
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
    }
    void testInsertFind()
    {
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i]);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
        }
    }
    void testDelete()
    {
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i]);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_delete(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
            found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)0);
        }
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
    }
    void testVisit()
    {
    }
    void testVisitWithoutSuffix()
    {
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestPrefixTree );
