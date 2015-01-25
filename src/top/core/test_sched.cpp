
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/sched.h>
#include <top/core/sync.h>
#include <limits.h>
#include <string.h>

using namespace std;

static void* set_fun(top_task_t* task,void* d) {
	long* pd = (long*)d;
	void* ret = (void*)*pd;
	top_task_yield(task);
	*pd = (long)d;
	printf("\n return ret: %p\n",ret);
	int i = 0;
	for(; i < 100; ++i) top_task_yield(task);
	printf("\n ********** exit set_fun \n");
	return ret;
}

class TestSched: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestSched);
#if 1
    CPPUNIT_TEST( testOneTask );
    //CPPUNIT_TEST( testChildTask );
    //CPPUNIT_TEST( testSignal );
    //CPPUNIT_TEST( testLock );
    //CPPUNIT_TEST( testCond );
#else
    CPPUNIT_TEST( testLock );
#endif
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

	static void sig_0(top_task_t* task,void* data,int sig,void* sig_data)
	{
		switch(sig) {
			case 0:
				printf("\n .0 signal\n");
				top_task_signal(task,1);
				break;
			case 1:
				printf("\n .1 signal\n");
				top_task_signal(task,2);
				break;
			case 2:
				printf("\n .2 signal\n");
				*(int*)data = 1;
				top_task_resume(task);
				break;
		}
	int i = 0;
	for(; i < 100; ++i) top_task_yield(task);
	}

	static void* signal_run(top_task_t* task,void* data)
	{
		top_task_sigaction(task,0,sig_0);
		top_task_sigaction(task,1,sig_0);
		top_task_sigaction(task,2,sig_0);

		int* pe = (int*)data;
		while(*pe == 0) {
			top_task_suspend(task);
		}
		return 0;
	}

	void testSignal()
	{
		long d = 0;
		top_task_t task;
		top_task_init(&task,0,signal_run,&d);
		top_task_active(&task,&sched);
		top_task_signal(&task,0);
		top_task_join(&task);
		CPPUNIT_ASSERT_EQUAL(d,1l);
	}

	static void* lock_run_fake(top_task_t* task,void* data)
	{
		for(;;) top_task_yield(task);
	}
	static void* lock_run(top_task_t* task,void* data)
	{
		void** paras = (void**)data;
		top_mutex_t* lock = (top_mutex_t*)paras[0];
		long* pd = (long*)paras[1];
		for(;*pd < 100;){
			//top_mutex_lock(lock);
			++*pd;
			printf("\n lock_run: task: %p, %ld\n", task,*pd);
			top_task_yield(task);

			//top_mutex_unlock(lock);
		}
		printf("\n exit task: %p, %ld\n", task,*pd);
		return 0;
	}

	void testLock()
	{
		top_mutex_t lock;
		long d = 0 ;
		void* paras[] = { &lock,&d };
		top_task_t task1,task2;
		top_mutex_init(&lock);
		top_task_init(&task1,0,lock_run_fake,paras);
		top_task_init(&task2,0,lock_run_fake,paras);
		printf("\n !!!!! taask : %p ,main_context:%p,sig_context: %p\n", &task1,&task1.main_context,&task1.sig_context);
		printf("\n !!!!! taask : %p ,main_context:%p,sig_context: %p\n", &task2,&task2.main_context,&task2.sig_context);
		top_task_active(&task2,&sched);
		top_task_active(&task1,&sched);
		top_task_join(&task2);
		top_task_join(&task1);
	}

	static void* cond_signal_run(top_task_t* task, void* data)
	{
		void** paras = (void**)data;
		top_cond_t* cond = (top_cond_t*)paras[0];
		long* pd = (long*)paras[1];
		for(; *pd < 10; ++*pd) {
			if(*pd % 2 == 0) {
				top_cond_signal(cond);
			}
			top_task_yield(task);
		}
		return 0;
	}

	static void* cond_wait_run(top_task_t* task, void* data)
	{
		void** paras = (void**)data;
		top_cond_t* cond = (top_cond_t*)paras[0];
		long* pd = (long*)paras[1];
		while(*pd < 5) {
			printf("\n wait: %ld\n", *pd);
			top_cond_wait(cond);
			printf("\n wait return: %ld\n", *pd);
		}
		printf("\n exit wait\n");
		return 0;
	}

	void testCond()
	{
		top_cond_t cond;
		long d = 0;
		void* paras[] = { &cond, &d };
		top_task_t task1,task2;
		top_cond_init(&cond);
		top_task_init(&task1,0,cond_wait_run,paras);
		top_task_init(&task2,0,cond_signal_run,paras);
		top_task_active(&task1,&sched);
		top_task_active(&task2,&sched);
		top_task_join(&task1);
		top_task_join(&task2);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION( TestSched );
