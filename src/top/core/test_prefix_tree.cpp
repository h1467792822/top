
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <top/core/prefix_tree.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

static const char* keys[] = {
    "http+//www+huawei+com",
    "http+//w3+huawei+com/info/cn/doc/viewDoc+do/did/5759291/cata/21073",
    "https+//www+huawei+com/w3",
    "http+//3ms+huawei+com/hi/index+php/app/home/mod+Info/act+accountset/type/email",
    "https+//www+baidu+com/index+html",
    "https+//www+google+com/index+html",
#if 1
    "http+//w3+huawei+com/next/indexa+html",
    "http+//w3+huawei+com/next/indexa+htma",
    "http+//w3+huawei+com/next/indexa+htal",
    "http+//w3+huawei+com/next/indexa+haml",
    "http+//w3+huawei+com/next/indexa+atml",
    "http+//w3+huawei+com/next/indexaahtml",
    "http+//w3+huawei+com/next/indexc+html",
    "http+//w3+huawei+com/next/indeaa+html",
    "http+//w3+huawei+com/next/indaxa+html",
    "http+//w3+huawei+com/next/inaexa+html",
    "http+//w3+huawei+com/next/iadexa+html",
    "http+//w3+huawei+com/next/andexa+html",
    "http+//w3+huawei+com/nextaindexa+html",
    "http+//w3+huawei+com/nexa/indexa+html",
    "http+//w3+huawei+com/neat/indexa+html",
    "http+//w3+huawei+com/naxt/indexa+html",
    "http+//w3+huawei+com/aext/indexa+html",
    "http+//w3+huawei+comanext/indexa+html",
    "http+//w3+huawei+coa/next/indexa+html",
    "http+//w3+huawei+cam/next/indexa+html",
    "http+//w3+huawei+aom/next/indexa+html",
    "http+//w3+huaweiacom/next/indexa+html",
    "http+//w3+huawea+com/next/indexa+html",
    "http+//w3+huawpi+com/next/indexa+html",
    "http+//w3+hua0ei+com/next/indexa+html",
    "http+//w3+hu9wei+com/next/indexa+html",
    "http+//w3+hnawei+com/next/indexa+html",
    "http+//w3+auawei+com/next/indexa+html",
    "http+//w3xhuawei+com/next/indexa+html",
    "http+//w2+huawei+com/next/indexa+html",
    "http+//s3+huawei+com/next/indexa+html",
    "http+/nw3+huawei+com/next/indexa+html",
    "http+x/w3+huawei+com/next/indexa+html",
    "httpc//w3+huawei+com/next/indexa+html",
    "httw+//w3+huawei+com/next/indexa+html",
    "htjp+//w3+huawei+com/next/indexa+html",
    "hwtp+//w3+huawei+com/next/indexa+html",
    "wttp+//w3+huawei+com/next/indexa+html",
#endif
};

static const int keys_cnt = sizeof(keys)/ sizeof(keys[0]);

class TestPrefixTree: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestPrefixTree);
    CPPUNIT_TEST( testOne  );
    CPPUNIT_TEST( testTwo  );
    CPPUNIT_TEST( testVec  );
    CPPUNIT_TEST( testInsertFind  );
    CPPUNIT_TEST( testInsertReverseFind  );
    CPPUNIT_TEST( testDelete );
    CPPUNIT_TEST( testDeleteReverse );
	CPPUNIT_TEST( testSelfMap );
    CPPUNIT_TEST( testInsertLimitedCapacity );
    CPPUNIT_TEST( testVisit );
    CPPUNIT_TEST( testVisitWithoutSuffix );
    CPPUNIT_TEST_SUITE_END();
    struct top_prefix_tree tree;
public:
    void setUp()
    {
        top_prefix_tree_init(&tree,0);
    }
    void tearDown()
    {
        top_prefix_tree_fini(&tree);
    }
	void testSelfMap()
	{
		struct top_prefix_tree tree;
		top_prefix_tree_slot_map slot_map;
		const char* key_map = "abcdefghijklmnopqrstuvwxyz0123456789.";
		top_prefix_tree_slot_map_init(slot_map,key_map,-1);
		top_prefix_tree_slot_map_init_more(slot_map,'a',"ABCDEFGHIJKLMNOPQRSTUVWXYZ",-1);
		top_prefix_tree_slot_map_init_more(slot_map,'.',"/",-1);
		top_prefix_tree_slot_map_init_more(slot_map,'.',"-",-1);
		top_prefix_tree_slot_map_init_more(slot_map,'.',"_",-1);
		top_prefix_tree_slot_map_init_more(slot_map,'.',":",-1);
		struct top_prefix_tree_conf conf;
		memset(&conf,0,sizeof(conf));
		conf.key_map = key_map;
		conf.key_map_size = strlen(conf.key_map);
		conf.slot_map = slot_map;
		top_prefix_tree_init(&tree,&conf);	

		top_error_t err;
		err = top_prefix_tree_simple_insert(&tree,"hatp://www.huawei.com",(void*)101,0);
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		err = top_prefix_tree_simple_insert(&tree,"http://wtw.huawei.com",(void*)101,0);
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		err = top_prefix_tree_simple_insert(&tree,"http:n/www.huawei.com",(void*)101,0);
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		err = top_prefix_tree_simple_insert(&tree,"http://www.huawei.com",(void*)101,0);
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		void* old;
		err = top_prefix_tree_simple_insert(&tree,"htTp.-_www.Huawei.cOM",(void*)100,&old);
		CPPUNIT_ASSERT_EQUAL(0,top_errno(err));
		CPPUNIT_ASSERT_EQUAL(old,(void*)101);
		void* found = top_prefix_tree_simple_find(&tree,"HTTP../WWW/huawei.Com");
		CPPUNIT_ASSERT_EQUAL(found,(void*)100);
		void* del = top_prefix_tree_simple_delete(&tree,"HTTP../WWW/huawei.Com");
		CPPUNIT_ASSERT_EQUAL(found,del);
		del = top_prefix_tree_simple_delete(&tree,"hatp://wWw.huawei.com");
		CPPUNIT_ASSERT_EQUAL((void*)101,del);
		del = top_prefix_tree_simple_delete(&tree,"http:/:wTw.huawei.com");
		CPPUNIT_ASSERT_EQUAL((void*)101,del);
		del = top_prefix_tree_simple_delete(&tree,"http:N/www.huawei.com");
		CPPUNIT_ASSERT_EQUAL((void*)101,del);
		CPPUNIT_ASSERT_EQUAL(tree.root,(unsigned long)0);
	}

    void testOne()
    {
        top_error_t err;
        int idx = keys_cnt/2;
        err = top_prefix_tree_simple_insert(&tree,keys[idx],(void*)9,0);
        CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
		void* old;
        err = top_prefix_tree_simple_insert(&tree,keys[idx],(void*)1,&old);
        CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        CPPUNIT_ASSERT_EQUAL((unsigned long)old,9ul);
        void* found = top_prefix_tree_simple_find(&tree,keys[idx]);
        void* del = top_prefix_tree_simple_delete(&tree,keys[idx]);
        CPPUNIT_ASSERT_EQUAL(found,del);
        CPPUNIT_ASSERT_EQUAL((unsigned long)found,1ul);
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
    }
	void testTwo()
	{
		void* found,*del;
        top_error_t err;
        err = top_prefix_tree_simple_insert(&tree,keys[0],(void*)1,0);
        CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        err = top_prefix_tree_simple_insert(&tree,keys[1],(void*)2,0);
        CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        found = top_prefix_tree_simple_find(&tree,keys[0]);
        CPPUNIT_ASSERT_EQUAL((unsigned long)found,1ul);
        found = top_prefix_tree_simple_find(&tree,keys[1]);
        CPPUNIT_ASSERT_EQUAL((unsigned long)found,2ul);
        del = top_prefix_tree_simple_delete(&tree,keys[1]);
        CPPUNIT_ASSERT_EQUAL((unsigned long)del,2ul);
        del = top_prefix_tree_simple_delete(&tree,keys[0]);
        CPPUNIT_ASSERT_EQUAL((unsigned long)del,1ul);
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
	}

    void testInsertLimitedCapacity()
    {
		int count = 0;
		tree.conf.max_capacity = 5 * 1024;
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i],0);
			if(top_errno(err)) {
				if(count == 0) {
					count = i;
				}
			}
        }
		cout << endl << "tree.capacity: " << tree.capacity << endl;
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_find(&tree,keys[i]);
			if(found == 0){
				CPPUNIT_ASSERT(i >= count);
			}
        }
    }
    void testInsertFind()
    {
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i],0);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
        }
    }
	struct counter {
		int count;
		int len;
	};
	static int visit_no_buf(struct top_prefix_tree_visit_ctx* ctx,void* data, int suffix_len, struct top_prefix_tree* tree)
	{
		struct counter * pc = (counter*)ctx->user_data;
		pc->count++;
		pc->len += suffix_len;
		return 1;	
	}

    void testVisitWithoutSuffix()
    {
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i],0);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
		cout << endl << "tree.capacity: " << tree.capacity << endl;
		struct counter counter = { 0,0};
		struct top_prefix_tree_visit_ctx ctx;
		memset(&ctx,0,sizeof(ctx));
		ctx.suffix_buf = 0;
		ctx.suffix_buf_len = 0;
		ctx.visit = visit_no_buf;
		ctx.user_data = &counter;
		top_prefix_tree_simple_visit(&tree,"https",&ctx);
		cout << endl << "-- visit ,total count = " << counter.count << ", Total Len: " << counter.len  << ", keycount: " << keys_cnt << endl;
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
        }
    }
	static int visit(struct top_prefix_tree_visit_ctx* ctx,void* data, int suffix_len, struct top_prefix_tree* tree)
	{
		printf("\nvisit: %*s\n", suffix_len > ctx->suffix_buf_len ? ctx->suffix_buf_len : suffix_len, ctx->suffix_buf);
		return ctx->suffix_buf[suffix_len - 1] != 'm';	
	}

    void testVisit()
    {
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i],0);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
		cout << endl << "tree.capacity: " << tree.capacity << endl;
		char buf[2048];
		struct top_prefix_tree_visit_ctx ctx;
		memset(&ctx,0,sizeof(ctx));
		ctx.suffix_buf = buf;
		ctx.suffix_buf_len = 2048;
		ctx.visit = visit;
		top_prefix_tree_simple_visit(&tree,"http",&ctx);
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
        }
    }
    void testInsertReverseFind()
    {
        top_error_t err;
        for(int i = keys_cnt - 1; i >= 0; --i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i],0);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
		cout << endl << "tree.capacity: " << tree.capacity << endl;
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
        }
    }
    void testDelete()
    {
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i],0);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
        for(int i = 0; i < keys_cnt; ++i) {
            void* found = top_prefix_tree_simple_delete(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
            found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)0);
        }
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
    }
    void testDeleteReverse()
    {
        top_error_t err;
        for(int i = 0; i < keys_cnt; ++i) {
            err = top_prefix_tree_simple_insert(&tree,keys[i],(void*)keys[i],0);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
        }
        for(int i = keys_cnt - 1; i >= 0; --i) {
            void* found = top_prefix_tree_simple_delete(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
            found = top_prefix_tree_simple_find(&tree,keys[i]);
            CPPUNIT_ASSERT_EQUAL(found,(void*)0);
        }
        CPPUNIT_ASSERT_EQUAL((void*)tree.root,(void*)0);
    }
	void testVec() {
		struct top_prefix_tree_key_vec vec[5];
		vec[0].key = (const char*)100;
		vec[0].key_end = vec[0].key;
		vec[2].key = (const char*)1009;
		vec[2].key_end = vec[2].key;
		vec[4].key = (const char*)1001;
		vec[4].key_end = vec[4].key;
		int len;
		top_error_t err;

        for(int i = 0; i < keys_cnt; ++i) {
			len = strlen(keys[i]);
			vec[1].key = keys[i];
			vec[1].key_end = vec[1].key + len / 2;
		    vec[3].key = vec[1].key_end;
			vec[3].key_end = vec[1].key + len;	
            err = top_prefix_tree_insert(&tree,vec,5,(void*)keys[i],0);
            CPPUNIT_ASSERT_EQUAL(top_errno(err),0);
            void* found = top_prefix_tree_find(&tree,vec,5);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
        }
		
        for(int i = keys_cnt - 1; i >= 0; --i) {
			len = strlen(keys[i]);
			vec[1].key = keys[i];
			vec[1].key_end = vec[1].key + len / 2;
		    vec[3].key = vec[1].key_end;
			vec[3].key_end = vec[1].key + len;	
            void* found = top_prefix_tree_delete(&tree,vec,5);
            CPPUNIT_ASSERT_EQUAL(found,(void*)keys[i]);
            found = top_prefix_tree_find(&tree,vec,5);
            CPPUNIT_ASSERT_EQUAL(found,(void*)0);
        }
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestPrefixTree );
