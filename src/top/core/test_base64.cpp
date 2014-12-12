
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/base64.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace std;

void on_base64(void* data, const char* out,int len)
{
    char* buf = (char*)data;
    memcpy(buf,out,len);
    buf[len] = 0;
}

void on_base64_append(void* data, const char* out,int len)
{
    char** pbuf = (char**)data;
    memcpy(*pbuf,out,len);
    *pbuf += len;
}

class TestBase64: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestBase64);
    CPPUNIT_TEST( testSimple );
    CPPUNIT_TEST( testSelfTable );
    CPPUNIT_TEST( testBase64 );
    CPPUNIT_TEST_SUITE_END();
public:
    void print_dtable(const unsigned int* dtable)
    {
        return ;
        const char* etable = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ0123456789+/";
        cout << endl << "dtable: " ;
        while(*etable) {
            cout << endl << *etable << " : " << dtable[*etable];
            ++etable;
        }
        cout << endl;
    }
    void testSimple()
    {
        top_base64_ctx ctx;
        char coded[5];
        top_base64_ctx_encode_init(&ctx,on_base64,coded,0,0);
        top_base64_encode_string(&ctx,"A");
        top_base64_encode_finish(&ctx);
        CPPUNIT_ASSERT(strcmp(coded,"QQ==") == 0);
        top_base64_ctx_decode_init(&ctx,on_base64,coded,0,0);
        print_dtable(ctx.dtable);
        int n = top_base64_decode_string(&ctx,coded);
        CPPUNIT_ASSERT_EQUAL(n,1);
        CPPUNIT_ASSERT(strcmp(coded,"A") == 0);
    }
    void testSelfTable()
    {
        top_base64_ctx ctx;
        const char* etable = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ0123456789+/";
        top_base64_decode_table dtable;
        memset(dtable,0,sizeof(dtable));
        top_base64_decode_table_init(dtable,etable);
        print_dtable(dtable);
        char coded[5];
        top_base64_ctx_encode_init(&ctx,on_base64,coded,etable,'.');
        top_base64_encode_string(&ctx,"A");
        top_base64_encode_finish(&ctx);
        CPPUNIT_ASSERT(strcmp(coded,"qq..") == 0);
        top_base64_ctx_decode_init(&ctx,on_base64,coded,dtable,'.');
        int n = top_base64_decode_string(&ctx,coded);
        CPPUNIT_ASSERT_EQUAL(n,1);
        CPPUNIT_ASSERT(strcmp(coded,"A") == 0);
    }
    void testBase64()
    {
        int cnt = 100;
        char in[cnt];
        char decoded[cnt];
        char encoded[top_base64_encoded_size(cnt)];
        char* pbuf = encoded;
        srandom((unsigned long)&in);
        for(int i= 0; i < cnt; ++i) {
            in[i] = (random() >> 12) & ((1 << 8) - 1);
        }

        top_base64_ctx ctx;
        top_base64_ctx_encode_init(&ctx,on_base64_append,&pbuf,0,0);
        top_base64_encode(&ctx,in,cnt);
        top_base64_encode_finish(&ctx);
        pbuf = decoded;
        top_base64_ctx_decode_init(&ctx,on_base64_append,&pbuf,0,0);
        int n = top_base64_decode(&ctx,encoded,(cnt + 2) / 3 * 4);
        printf("\nencoded: %*s\n",(cnt + 2) / 3 * 4,encoded);
        CPPUNIT_ASSERT_EQUAL(n,1);
        for(int i = 0; i < cnt; ++i) {
            CPPUNIT_ASSERT_EQUAL(in[i],decoded[i]);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestBase64 );
