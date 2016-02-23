
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/co_thread.h>
#include <limits.h>
#include <string.h>

using namespace std;

static void* test_run(top_co_task_t* task,void* d) {
	long* pd = (long*)d;
	long v = *pd;
	int i = 0;
	for(; i < 100; ++i) {
		CPPUNIT_ASSERT_EQUAL(v,*pd);
		*pd += 1;
		v += 2;
		top_co_task_yield(task);
	}
	return pd;
}

class TestCoTask: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestCoTask);
    CPPUNIT_TEST( testTwoTask );
    CPPUNIT_TEST_SUITE_END();
	top_sched_t sched;
public:
    void setUp()
    {
		top_sched_init(&sched,0);
    }
    void tearDown()
    {
		top_sched_terminate(&sched);
		top_sched_join(&sched);
		top_sched_fini(&sched);
    }

    void testTwoTask()
    {
		long d = 100;
		top_error_t err;
		top_co_task_t task1;
		top_co_task_init(&task1,0,test_fun,&d);
		top_co_task_t task2;
		top_co_task_init(&task2,0,test_fun,&d);
		err = top_task_active(&task1,&sched);
		CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
		err = top_task_join(&task);
		CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
		CPPUNIT_ASSERT_EQUAL(d,(long)&d);
		CPPUNIT_ASSERT_EQUAL((void*)task.retval,(void*)100);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestCoTask );
