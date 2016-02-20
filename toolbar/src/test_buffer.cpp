
#include <cppunit/extensions/HelperMacros.h>
#include <sys/uio.h>
#include "buffer.h"

using namespace std;

class TestBuffer: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestBuffer);
    CPPUNIT_TEST( testNewBuffer_1 );
    CPPUNIT_TEST( testNewBuffer_100 );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp()
    {
    }
    void tearDown() {}

	void testNewBuffer(int n)
	{
		int i = 0; 
		int m;
		char* p;
		struct iovec vec[n];
		m = buff_vec_alloc(vec,n);
        CPPUNIT_ASSERT_EQUAL(m,n);
		for(; i < n; ++i){
			CPPUNIT_ASSERT(vec[i].iov_base != 0);
			CPPUNIT_ASSERT_EQUAL((int)vec[i].iov_len,BUF_LEN);
			p = (char*)vec[i].iov_base;
			p[BUF_LEN - 1] = ((size_t)p) & 0xFF;
		}
		buff_vec_free(vec,n);
	}

	void testNewBuffer_1()
	{
		testNewBuffer(1);
	}

	void testNewBuffer_100()
	{
		testNewBuffer(100);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestBuffer );
