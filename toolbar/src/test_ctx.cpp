
#include <cppunit/extensions/HelperMacros.h>
#include <string.h>
#include "ctx.h"

using namespace std;

class TestCtx: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestCtx);
    CPPUNIT_TEST( testProxyCtx_1 );
   	CPPUNIT_TEST( testProxyCtx_100 );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp()
    {
    }
    void tearDown() {}

	void testProxyCtx(int n)
	{
		int i = 0;
		struct proxy_ctx zeroCtx;
		proxy_ctx* pctx[n];
		memset(&zeroCtx,0,sizeof(zeroCtx));
		for(; i < n; ++i){
			pctx[i] = proxy_ctx_alloc(); 
			CPPUNIT_ASSERT(pctx[i]);
			CPPUNIT_ASSERT_EQUAL((int)0,memcmp(pctx[i],&zeroCtx,sizeof(zeroCtx)));
		}
		for(i = 0; i < n; ++i){
			proxy_ctx_free(pctx[i]);
		}
	}

	void testProxyCtx_1()
	{
		testProxyCtx(1);
	}

	void testProxyCtx_100()
	{
		testProxyCtx(100);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestCtx );
