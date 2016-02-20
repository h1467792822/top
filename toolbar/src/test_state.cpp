
#include <cppunit/extensions/HelperMacros.h>
#include <string.h>
#include <inttypes.h>
#include "state.h"
#include "tb_stddef.h"

using namespace std;

enum num_state {
	num_st_init = 0,
	num_st_digit,
	num_st_0,
	num_st_8,
	num_st_16,
	num_st_null,
};

struct number {
	int64_t value;
	int error;
	struct state_machine sm;
};

static void num_on_leave(struct state_machine* sm,int to_state)
{
	struct number* num = container_of(sm,struct number,sm);
	if(to_state == num_st_null) {
		num->error = 1;
	}
}

static int num_init_on_event(struct state_machine*sm,int event,void*data)
{
	char* pc = (char*)data;
	struct number* num = container_of(sm,struct number,sm);
	switch(*pc){
		case '0':
		return num_st_0;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		num->value = *pc;
		return num_st_digit;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		num->value = 10 + *pc - 'A';
		return num_st_16;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		num->value = 10 + *pc - 'a';
		return num_st_16;
		default:
		return num_st_null;
	}
}
static int num_digit_on_event(struct state_machine*sm,int event,void*data){
	struct number* num = container_of(sm,struct number,sm);
	char* pc = (char*)data;
	switch(*pc){
		case '0': case '1': case '2':case '3': case '4':case '5':case '6':case '7':case '8':case '9':
			num->value *= 10;
			num->value += *pc - '0';
			return num_st_digit;
		default:
			return num_st_null;
	}
}

static void num_digit_on_enter(struct state_machine*sm,int from_state) {
	CPPUNIT_ASSERT_EQUAL(from_state,(int)num_st_init);
	struct number* num = container_of(sm,struct number,sm);
	num->value = num->value - '0';
}

static int num_0_on_event(struct state_machine*sm,int event,void*data){
	struct number* num = container_of(sm,struct number,sm);
	char* pc = (char*)data;
	switch(*pc) {
		case 'x': case 'X':
			return num_st_16;
		case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':
			num->value = *pc - '0';
			return num_st_8;
		default:
			return num_st_null;
	}
	
}

static int num_8_on_event(struct state_machine*sm,int event,void*data)
{
	struct number* num = container_of(sm,struct number,sm);
	char* pc = (char*)data;
	switch(*pc) {
		case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':
			num->value <<= 3;
			num->value += *pc - '0';
			return num_st_8;
		default:
			return num_st_null;
	}
}

static int num_16_on_event(struct state_machine*sm,int event,void*data)
{
	struct number* num = container_of(sm,struct number,sm);
	char* pc = (char*)data;
	int val;
	switch(*pc) {
		case '0': case '1': case '2':case '3': case '4':case '5':case '6':case '7':case '8':case '9':
			val = *pc - '0';
			break;
		case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
			val = *pc - 'A' + 10;
			break;
		case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
			val = *pc - 'a' + 10;
			break;
		default:
			return num_st_null;
	}
	num->value <<= 4;
	num->value += val;
	return num_st_16;
}
static int num_null_on_event(struct state_machine*sm,int event,void*data)
{
	return num_st_null;
}

struct state g_num_states[] = {
	{num_st_init,num_init_on_event,0,num_on_leave},
	{num_st_digit,num_digit_on_event,num_digit_on_enter,num_on_leave},
	{num_st_0,num_0_on_event,0,num_on_leave},
	{num_st_8,num_8_on_event,0,num_on_leave},
	{num_st_16,num_16_on_event,0,num_on_leave},
	{num_st_null,num_null_on_event,0,0}
};

static void number_init(struct number* num)
{
	num->value = 0;
	num->error = 0;
	num->sm.cur_st = num_st_init;
	num->sm.states = g_num_states;
}

class TestState: public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TestState);
    CPPUNIT_TEST( test_10 );
    CPPUNIT_TEST( test_10_error );
   	CPPUNIT_TEST( test_8 );
   	CPPUNIT_TEST( test_8_error );
   	CPPUNIT_TEST( test_16_0x );
   	CPPUNIT_TEST( test_16 );
   	CPPUNIT_TEST( test_16_error );
    CPPUNIT_TEST_SUITE_END();
	struct number number;
public:
    void setUp()
    {
		number_init(&number);
    }
    void tearDown() {}

	void parse(const char* pstr) {
		while(*pstr) {
			sm_notify(&number.sm,0,(void*)pstr);
			++pstr;
		}
	}

	void test_10()
	{
		parse("9876543210");
		CPPUNIT_ASSERT(number.value == 9876543210);
	}

	void test_10_error()
	{
		parse("987abc");
		CPPUNIT_ASSERT(number.error == 1);
		CPPUNIT_ASSERT(number.value == 987);
	}

	void test_8()
	{
		parse("076543210");
		CPPUNIT_ASSERT(number.value == 0076543210);
	}

	void test_8_error()
	{
		parse("07658");
		CPPUNIT_ASSERT(number.value = 0765);
		CPPUNIT_ASSERT(number.error == 1);
	}

	void test_16_0x()
	{
		parse("0x0123456789ABCDEF");
		CPPUNIT_ASSERT(number.value == 0x0123456789ABCDEF);
	}

	void test_16_error()
	{
		parse("EFG");
		CPPUNIT_ASSERT(number.value == 0xEF);
		CPPUNIT_ASSERT(number.error == 1);
	}

	void test_16()
	{
		parse("EF23456789ABCD01");
		CPPUNIT_ASSERT(number.value == 0xEF23456789ABCD01u);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION( TestState );
