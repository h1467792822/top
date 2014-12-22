
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/pool.h>
#include <top/core/alloc.h>
#include <top/core/stddef.h>
#include <limits.h>
#include <string.h>

using namespace std;

static const struct top_pool_conf conf = {
	top_glibc_malloc,
	top_glibc_memalign,
	top_glibc_free,
	0,
	(unsigned long)-1,
	4 * 1024u
};

class TestPool: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestPool);
    CPPUNIT_TEST( testAlloc );
    CPPUNIT_TEST( testLarge );
	CPPUNIT_TEST( testMore );
    CPPUNIT_TEST_SUITE_END();
    struct top_pool pool;
public:
    void setUp()
    {
        top_pool_init(&pool,&conf);
    }
    void tearDown() { top_pool_fini(&pool); }

	void testAlloc() {
		top_error_t err;
		void* p1 = 0,*p2 = 0;
		err = top_pool_malloc(&pool,100,&p1);	
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		CPPUNIT_ASSERT(p1);
		memset(p1,23,100);
		CPPUNIT_ASSERT_EQUAL(pool.capacity,(unsigned long)conf.page_size);
		top_pool_free(&pool,p1);


		err = top_pool_malloc(&pool,1000,&p2);	
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		CPPUNIT_ASSERT(p2);
		memset(p2,23,1000);
		top_pool_free(&pool,p2);
		CPPUNIT_ASSERT_EQUAL(pool.capacity,(unsigned long)conf.page_size);
		CPPUNIT_ASSERT(p1 == p2);

		CPPUNIT_ASSERT_EQUAL(2ul, pool.page_reuse_count);
		CPPUNIT_ASSERT_EQUAL(2ul, pool.alloc_count);
		CPPUNIT_ASSERT_EQUAL(2ul, pool.free_count);
	}
	void testLarge() {
		void* p ;
		top_error_t err;
		err = top_pool_malloc(&pool,5 * 1024,&p);
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		CPPUNIT_ASSERT_EQUAL(1ul,pool.large_alloc_count);
		CPPUNIT_ASSERT_EQUAL(1ul,pool.alloc_count);
		CPPUNIT_ASSERT_EQUAL(pool.capacity ,5 * 1024ul + conf.page_size);
		top_pool_free(&pool,p);
		CPPUNIT_ASSERT_EQUAL(pool.capacity ,(unsigned long)conf.page_size);
		CPPUNIT_ASSERT_EQUAL(1ul,pool.large_alloc_count);
		CPPUNIT_ASSERT_EQUAL(1ul,pool.large_free_count);
	}
	void testMore() {
		void* ps[5000];
		top_error_t err;
		unsigned long size = 0;
		for(int i = 0; i < 5000; ++i) {
			err = top_pool_malloc(&pool,i + 1,&ps[i]);
			size += i + 1;
			CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
			CPPUNIT_ASSERT(ps[i]);
		}
		cout << endl << "pages_count: " << pool.pages_count << ": large_count: " << pool.large_count << ": capacity: " << pool.capacity << ": user space: " << (1 + 5000) * 5000 / 2  << " : size: " << size <<  " : " << 1.0f * size / pool.capacity  << endl;	
		for(int i = 0; i < 5000; ++i) {
			memset(ps[i],0xff,i + 1);
		}
		for(int i = 4999; i >= 0; --i){
			top_pool_free(&pool,ps[i]);
		}
		top_pool_free_cached(&pool);
		CPPUNIT_ASSERT_EQUAL(0u,pool.pages_count);
		CPPUNIT_ASSERT_EQUAL(0u,pool.large_count);
		CPPUNIT_ASSERT_EQUAL(0ul,pool.capacity);
		cout << endl << "alloc_count: " << pool.alloc_count << ": free_count: " << pool.free_count << ": large_alloc_count: " << pool.large_alloc_count << " : large_free_count: " << pool.large_free_count << endl;	
		CPPUNIT_ASSERT_EQUAL(5000ul,pool.alloc_count);
		CPPUNIT_ASSERT_EQUAL(5000ul,pool.free_count);
		CPPUNIT_ASSERT_EQUAL(pool.large_alloc_count,pool.large_free_count);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestPool );
