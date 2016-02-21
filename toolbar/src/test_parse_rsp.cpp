
#include <cppunit/extensions/HelperMacros.h>
#include <string.h>
#include "parse.h"
#include <stdio.h>

using namespace std;

class TestParseRsp: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestParseRsp);
    CPPUNIT_TEST( testHtml );
    CPPUNIT_TEST( testHtml2 );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp()
    {
    }
    void tearDown() {}

	void testHtml()
	{
		const char* req = "HTTP/1.1 200 OK \r\n"\
						   "Content-Type: text/html\r\n"\
						   "Content-Length: 10\r\n"\
						  "\r\n"\ 
						  "0123456789ABCDEF";
		struct parse_data data;
		memset(&data,0,sizeof(data));
		data.p = req;data.pe = req + strlen(req) + 1;
		parse_rsp_beg(&data);
		parse_rsp(&data);
		printf("\ndata: true=%d,false=%d,length=%d,p=%s\n",data.true_flag,data.false_flag,data.length,data.p);
		CPPUNIT_ASSERT(data.length == 10);
		CPPUNIT_ASSERT(data.true_flag);
		CPPUNIT_ASSERT(data.false_flag == 0);
		CPPUNIT_ASSERT(data.err == 0);
		CPPUNIT_ASSERT('0' == *data.p);
	}

	void testHtml2()
	{
		const char* req1 = "HTTP/1.1 201 OK \r\n"\
						   "X-Type: text/html\r";

	   const char* req2 = "Content-Type: text/html\r\n"\
						   "Content-Length: 9\r\n"\
						  "\r\n"\ 
						  "123456789ABCDEF";
		struct parse_data data;
		memset(&data,0,sizeof(data));
		data.p = req1;data.pe = req1 + strlen(req1) + 1;
		parse_rsp_beg(&data);
		parse_rsp(&data);
		parse_data_dump(&data,"first parse");
		data.p = req2;data.pe = req2 + strlen(req2) + 1;
		parse_rsp(&data);
		parse_data_dump(&data,"second parse");
		CPPUNIT_ASSERT(data.length == 9);
		CPPUNIT_ASSERT(data.true_flag);
		CPPUNIT_ASSERT(data.false_flag);
		CPPUNIT_ASSERT(data.err == 0);
		CPPUNIT_ASSERT('1' == *data.p);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION( TestParseRsp );
