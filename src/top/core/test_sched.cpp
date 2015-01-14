
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/sched.h>
#include <limits.h>
#include <string.h>

using namespace std;

static void* set_fun(top_task_t* task,void* d) {
	long* pd = (long*)d;
	void* ret = (void*)*pd;
	top_task_yield(task);
	*pd = (long)d;
	printf("\n return ret: %p\n",ret);
	return ret;
}

class TestSched: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestSched);
    CPPUNIT_TEST( testOneTask );
    CPPUNIT_TEST( testChildTask );
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

    void testOneTask()
    {
		long d = 100;
		top_error_t err;
		top_task_t task;
		top_task_init(&task,0,set_fun,&d);
		err = top_task_active(&task,&sched);
		CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
		err = top_task_join(&task);
		CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
		CPPUNIT_ASSERT_EQUAL(d,(long)&d);
		CPPUNIT_ASSERT_EQUAL((void*)task.retval,(void*)100);
    }

	static void* child_run(top_task_t* task,void* data)
	{
		top_task_t child;
		top_task_init(&child,0,set_fun,data);
		top_task_active(&child,task->sched);
		top_task_join(&child);
		return child.retval;
	}

	void testChildTask()
	{
		long d = 100;
		top_task_t task;
		top_task_init(&task,0,child_run,&d);
		top_task_active(&task,&sched);
		top_task_join(&task);
		CPPUNIT_ASSERT_EQUAL(d,(long)&d);
		CPPUNIT_ASSERT_EQUAL((void*)task.retval,(void*)100);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestSched );
