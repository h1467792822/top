
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/bcache.h>
#include <top/core/stddef.h>
#include <limits.h>
#include <string.h>

using namespace std;

static struct top_bcache_conf conf = {
    0,
    8 * 1024ul,
    4 * 1024ul,
    511u,
};

class TestBCache: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestBCache);
    CPPUNIT_TEST( testAlloc );
    CPPUNIT_TEST( testFree );
    CPPUNIT_TEST_SUITE_END();
    struct top_bcache cache;
public:
    void setUp()
    {
        top_bcache_init(&cache,&conf);
    }
    void tearDown()
    {
        top_bcache_fini(&cache);
    }
    void testAlloc()
    {
        top_error_t err;
        void* p = 0;
        err = top_bcache_alloc(&cache,&p);
        CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
        CPPUNIT_ASSERT(p);
        CPPUNIT_ASSERT_EQUAL(cache.capacity,conf.page_size);
        CPPUNIT_ASSERT_EQUAL(1u,cache.pages_count);
        CPPUNIT_ASSERT_EQUAL(1u,cache.partial_cached_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.full_cached_count);
        top_bcache_free(&cache,p);
        CPPUNIT_ASSERT_EQUAL(1u,cache.pages_count);
        CPPUNIT_ASSERT_EQUAL(1u,cache.partial_cached_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.full_cached_count);
        void* ps[cache.block_count_per_page];
        for(int i = 0; i < cache.block_count_per_page; ++i)
            top_bcache_alloc(&cache,&ps[i]);
        CPPUNIT_ASSERT_EQUAL(1u,cache.pages_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.partial_cached_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.full_cached_count);
        for(int i = 0; i < cache.block_count_per_page; ++i)
            top_bcache_free(&cache,ps[i]);
        CPPUNIT_ASSERT_EQUAL(1u,cache.pages_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.partial_cached_count);
        CPPUNIT_ASSERT_EQUAL(1u,cache.full_cached_count);
    }

    void testFree()
    {
        void* ps[100];
        int i = 0;
        top_error_t err;
        int j = 0;
        for(j = 0; j < 3; ++j) {
            memset(&ps,0,sizeof(ps));
            for(i = 0; i < 100; ++i) {
                err = top_bcache_alloc(&cache,&ps[i]);
                if(top_errno(err)) {
                    CPPUNIT_ASSERT_EQUAL(conf.max_capacity,cache.capacity);
                } else {
                    CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
                    CPPUNIT_ASSERT(ps[i]);
                }
            }
            CPPUNIT_ASSERT_EQUAL(2u,cache.pages_count);
            CPPUNIT_ASSERT_EQUAL(0u,cache.partial_cached_count);
            CPPUNIT_ASSERT_EQUAL(0u,cache.full_cached_count);
            for(i = 0; i < 100; ++i) {
                if(ps[i]) {
                    top_bcache_free(&cache,ps[i]);
                }
            }
        }
        CPPUNIT_ASSERT_EQUAL(2u,cache.pages_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.partial_cached_count);
        CPPUNIT_ASSERT_EQUAL(2u,cache.full_cached_count);
        top_bcache_free_cached(&cache);
        CPPUNIT_ASSERT_EQUAL(0u,cache.pages_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.partial_cached_count);
        CPPUNIT_ASSERT_EQUAL(0u,cache.full_cached_count);
        CPPUNIT_ASSERT(cache.capacity == 0 || cache.capacity == conf.page_size);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestBCache );
