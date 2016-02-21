
#include <cppunit/extensions/HelperMacros.h>
#include <string.h>
#include "parse_req.h"
#include <stdio.h>

using namespace std;

class TestParseReq: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestParseReq);
    CPPUNIT_TEST( testHtml );
    CPPUNIT_TEST( testJS );
    CPPUNIT_TEST( testJS2 );
    CPPUNIT_TEST( testHtml2 );
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp()
    {
    }
    void tearDown() {}

	void testHtml()
	{
		const char* req = "GET /index.js1 HTTP/1.1\r\n"\
						   "Host: www.baidu.com\r\n"\
						   "Accept: */*\r\n"\
						   "Content-Length: 10\r\n"\
						   "Accept-Encoding: gzip:q=1.0,identiy:q=0.5\r\n"\
						  "\r\n"\ 
						  "0123456789ABCDEF";
		struct parse_data data;
		memset(&data,0,sizeof(data));
		data.p = req;data.pe = req + strlen(req) + 1;
		parse_req_beg(&data);
		parse_req(&data);
		printf("\ndata: flag=%d,length=%d,p=%s\n",data.flag,data.length,data.p);
		CPPUNIT_ASSERT(data.length == 10);
		CPPUNIT_ASSERT(data.flag);
		CPPUNIT_ASSERT(data.err == 0);
		CPPUNIT_ASSERT('0' == *data.p);
	}
	void testJS()
	{
		const char* req = "GET /index.js HTTP/1.1\r\n"\
						   "Host: www.baidu.com\r\n"\
						   "Accept: */*\r\n"\
						   "Content-Length: 10\r\n"\
						   "Accept-Encoding: gzip:q=1.0,identiy:q=0.5\r\n"\
						  "\r\n"\ 
						  "0123456789ABCDEF";
		struct parse_data data;
		memset(&data,0,sizeof(data));
		data.p = req;data.pe = req + strlen(req) + 1;
		parse_req_beg(&data);
		parse_req(&data);
		printf("\ndata: flag=%d,length=%d,p=%s\n",data.flag,data.length,data.p);
		CPPUNIT_ASSERT(data.length == 10);
		CPPUNIT_ASSERT(data.flag == 0);
		CPPUNIT_ASSERT(data.err == 0);
		CPPUNIT_ASSERT('0' == *data.p);
	}
	void testJS2()
	{
		const char* req = "GET /index.js?xx=yy@i=1 HTTP/1.1\r\n"\
						   "Host: www.baidu.com\r\n"\
						   "Accept: */*\r\n"\
						   "Content-Length: 10\r\n"\
						   "Accept-Encoding: gzip:q=1.0,identiy:q=0.5\r\n"\
						  "\r\n"\ 
						  "0123456789ABCDEF";
		struct parse_data data;
		memset(&data,0,sizeof(data));
		data.p = req;data.pe = req + strlen(req) + 1;
		parse_req_beg(&data);
		parse_req(&data);
		printf("\ndata: flag=%d,length=%d,p=%s\n",data.flag,data.length,data.p);
		CPPUNIT_ASSERT(data.length == 10);
		CPPUNIT_ASSERT(data.flag == 0);
		CPPUNIT_ASSERT(data.err == 0);
		CPPUNIT_ASSERT('0' == *data.p);
	}


	void testHtml2()
	{
		const char* req1 = "GET /index.html HTTP/1.1\r\n"\
						   "Host: www.baidu.com\r\n"\
						   "Accept:";
	   const char* req2 = "	*/*\r\n"\
						   "Content-Length: 9\r\n"\
						   "Accept-Encoding: gzip:q=1.0,identiy:q=0.5\r\n"\
						  "\r\n"\ 
						  "123456789ABCDEF";
		struct parse_data data;
		memset(&data,0,sizeof(data));
		data.p = req1;data.pe = req1 + strlen(req1) + 1;
		parse_req_beg(&data);
		parse_req(&data);
		data.p = req2;data.pe = req2 + strlen(req2) + 1;
		parse_req(&data);
		printf("\ndata: flag=%d,length=%d,p=%s\n",data.flag,data.length,data.p);
		CPPUNIT_ASSERT(data.length == 9);
		CPPUNIT_ASSERT(data.flag);
		CPPUNIT_ASSERT(data.err == 0);
		CPPUNIT_ASSERT('1' == *data.p);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION( TestParseReq );
