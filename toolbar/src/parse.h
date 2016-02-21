

#ifndef TB_PARSE_H
#define TB_PARSE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct parse_data {
	const char* p;
	const char* pe;
	int cs;
	int act;
	int stack[16];
	int top;
	const char* ts;
	const char* te;
	int length;
	int true_flag;
	int false_flag;
	int err;
};

void parse_req_beg(struct parse_data* data);
void parse_req(struct parse_data* data);
void parse_rsp_beg(struct parse_data* data);
void parse_rsp(struct parse_data* data);

static inline void parse_data_dump(struct parse_data* data,const char* msg)
{
	printf("\n+++++++++++ parse_data dump +++++++++:\n"\
			"msg=%s\n"\
			"err=%d\n"\
			"length=%d\n"\
			"true=%d\n"\
			"false=%d\n"\
			"cs=%d\n"\
			"act=%d\n"\
			"top=%d\n"\
			"ts=%p\n"\
			"te=%p\n"\
			"p=[%s]\n",
			msg,
			data->err,data->length,data->true_flag,data->false_flag,
			data->cs,data->act,data->top,data->ts,data->te,data->p);
}
#ifdef __cplusplus
}
#endif

#endif

