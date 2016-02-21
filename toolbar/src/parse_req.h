

#ifndef TB_PARSE_REQ_H
#define TB_PARSE_REQ_H

#ifdef __cplusplus
extern "C" {
#endif

struct parse_data {
	const char* p;
	const char* pe;
	int cs;
	int length;
	int flag;
	int err;
};

void parse_req_beg(struct parse_data* data);
void parse_req(struct parse_data* data);

#ifdef __cplusplus
}
#endif

#endif

