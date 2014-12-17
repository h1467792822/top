
#ifndef TOP_CORE_BASE64_H
#define TOP_CORE_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pf_top_base64_output)(void* user_data, const char* out,int len);

typedef const char top_base64_encode_table[65]; /** include null char */
typedef unsigned char top_base64_decode_table[256];

void top_base64_decode_table_init(top_base64_decode_table dtable,top_base64_encode_table etable);

struct top_base64_ctx {
	unsigned int in;
	unsigned short waiting;
	unsigned short tail;
	pf_top_base64_output output;
	void* user_data;
	union {
	const char* etable;
	const unsigned char* dtable;
	};
	char padding;
};

static inline unsigned int top_base64_encoded_size(unsigned int size)
{
	return ((size + 2) / 3) << 2;
}

static inline unsigned int top_base64_decoded_size(unsigned int size)
{
	return ((size + 3) >> 2) * 3;
}
/**
* etable,padding accept NULL
*/
void top_base64_ctx_encode_init(struct top_base64_ctx* ctx,pf_top_base64_output output,void* user_data, top_base64_encode_table etable,char padding);
void top_base64_encode(struct top_base64_ctx* ctx, const char* in,int len);
void top_base64_encode_string(struct top_base64_ctx* ctx, const char* in);
void top_base64_encode_finish(struct top_base64_ctx* ctx);

/**
* etable,padding accept NULL
*/
void top_base64_ctx_decode_init(struct top_base64_ctx* ctx,pf_top_base64_output output,void* user_data,top_base64_decode_table dtable,char padding);
int top_base64_decode(struct top_base64_ctx* ctx,const char* in,int len);
int top_base64_decode_string(struct top_base64_ctx* ctx,const char* in);

#ifdef __cplusplus
}
#endif


#endif

