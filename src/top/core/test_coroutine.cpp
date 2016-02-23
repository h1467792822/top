
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/sched.h>
#include <top/core/sync.h>
#include <limits.h>
#include <string.h>

using namespace std;

#define TEST_VALS_CNT 100

typedef struct test_ctx
{
	top_coroutine_t co;
	int val;
	int count;
	int vals[TEST_VALS_CNT];
	int* padded;
}test_ctx;



static void* co_runner(top_coroutine_t* co) {
	long* pd = (long*)d;
	void* ret = (void*)*pd;
	top_co_yield(co);
	*pd = (long)d;
	printf("\n return ret: %p\n",ret);
	int i = 0;
	for(; i < 100; ++i) top_co_yield(co);
	printf("\n ********** exit set_fun \n");
	return ret;
}

class TestCoroutine: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestCoroutine);
    CPPUNIT_TEST( testOne );
    CPPUNIT_TEST( testMore );
    CPPUNIT_TEST( testChild );
    CPPUNIT_TEST_SUITE_END();
public:
	static void* test_runner(top_coroutine_t* co) {
		test_ctx* ctx = top_container_of(co,test_ctx,co);
		int i = 0;
		for(; i < ctx->count; ++i) {
			ctx->vals[i] = ctx->val + *ctx->padded;
			++ctx->padded;
			top_co_yield(data);
		}
		return ctx->padded;
	}	

    void testOne()
    {
		long ctx = 0;
		long in = 100;
		top_error_t err;
		test_ctx ctx;
		top_co_init(&ctx,test_runner);
		top_sch_
		err = top_task_active(&task,&in,0);
		CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
		err = top_task_join(&task);
		CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
		CPPUNIT_ASSERT_EQUAL(d,(long)&d);
		CPPUNIT_ASSERT_EQUAL((void*)task.retval,(void*)100);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestSched );
