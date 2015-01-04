
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/sched.h>
#include <limits.h>
#include <string.h>

using namespace std;

static void* set_fun(void* d) {
	long* pd = (long*)d;
	void* ret = (void*)*pd;
	*pd = (long)d;
	return ret;
}

class TestSched: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestSched);
    CPPUNIT_TEST( testOneTask );
    CPPUNIT_TEST_SUITE_END();
	top_sched_t sched;
public:
    void setUp()
    {
		top_sched_init(&sched);
    }
    void tearDown()
    {
		top_sched_join(&sched);
    }

    void testOneTask()
    {
		long d = 100;
		top_task_t task;
		top_task_init(&task,set_fun,&d);
		top_task_attach(&task,&sched);
		void* ret = top_task_join(&task);
		CPPUNIT_ASSERT_EQUAL(d,(long)&d);
		CPPUNIT_ASSERT_EQUAL((long)ret,100);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestSched );
