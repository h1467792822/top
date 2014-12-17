
#include <top/core/base64.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>

static const char g_base64_encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char g_base64_decode_table[256] = {
    255,255,255,255,255,255,255,255,255,255, // 0-9
    255,255,255,255,255,255,255,255,255,255, // 10-19
    255,255,255,255,255,255,255,255,255,255, // 20-29
    255,255,255,255,255,255,255,255,255,255, // 30-39
    255,255,255,62 ,255,255,255,63 ,52 ,53 , // 40-49
    54 ,55 ,56 ,57 ,58 ,59 ,60 ,61 ,255,255, // 50-59
    255,255,255,255,255,0  ,1  ,  2,  3,  4, // 60-69
    5  ,6  ,7  ,8  ,9  ,10 , 11, 12, 13, 14, // 70-79
    15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,24 , // 80-89
    25 ,255,255,255,255,255,255, 26, 27, 28, // 90-99
    29 ,30 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,38 , //100- 109
    39 ,40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,48 , //110 -119
    49 ,50 ,51 ,255,255,255,255,255,255,255, //120 - 129
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255
};


void top_base64_decode_table_init(top_base64_decode_table dtable,top_base64_encode_table etable)
{
    int i;
    for(i = 0; i < 256; ++i) {
        dtable[i] = 255;
    }
    printf("\nset dtable: etable: %s\n",etable);
    for(i = 0; i < 64; ++i) {
        dtable[(unsigned char)etable[i]] = i;
    }
}


#define BASE64_ENC_MAP_SHIFT 6
#define BASE64_ENC_MAP_MASK ((1ul << BASE64_ENC_MAP_SHIFT) - 1)
#define BASE64_ENC_UNIT_SIZE 3

#define BASE64_DEC_MAP_SHIFT 8
#define BASE64_DEC_MAP_MASK ((1ul << BASE64_DEC_MAP_SHIFT) - 1)
#define BASE64_DEC_UNIT_SIZE 4

void top_base64_ctx_encode_init(struct top_base64_ctx* ctx,pf_top_base64_output output,void* userdata,top_base64_encode_table etable,char padding)
{
    ctx->in = 0;
    ctx->waiting = BASE64_ENC_UNIT_SIZE;
    ctx->output = output;
    ctx->user_data = userdata;
    ctx->etable = etable ? etable : g_base64_encode_table;
    ctx->padding = padding ? padding : '=';
}

void top_base64_ctx_decode_init(struct top_base64_ctx* ctx,pf_top_base64_output output,void* userdata,top_base64_decode_table dtable,char padding)
{
    ctx->in = 0;
    ctx->waiting = BASE64_DEC_UNIT_SIZE;
    ctx->tail = 0;
    ctx->output = output;
    ctx->user_data = userdata;
    ctx->dtable = dtable ? dtable : g_base64_decode_table;
    ctx->padding = padding ? padding : '=';
}

static inline void top_base64_encode_char(struct top_base64_ctx* ctx, unsigned char c,char* out)
{
    ctx->in <<= BASE64_DEC_MAP_SHIFT;
    ctx->in |= c;
    if(--ctx->waiting == 0) {
        out[3] = ctx->etable[ctx->in & BASE64_ENC_MAP_MASK];
        out[2] = ctx->etable[(ctx->in >> BASE64_ENC_MAP_SHIFT) & BASE64_ENC_MAP_MASK];
        out[1] = ctx->etable[(ctx->in >> (2 * BASE64_ENC_MAP_SHIFT)) & BASE64_ENC_MAP_MASK];
        out[0] = ctx->etable[(ctx->in >> (3 * BASE64_ENC_MAP_SHIFT)) & BASE64_ENC_MAP_MASK];
        ctx->in = 0;
        ctx->waiting = BASE64_ENC_UNIT_SIZE;
        ctx->output(ctx->user_data,out,4);
    }
}

void top_base64_encode(struct top_base64_ctx* ctx, const char* in,int len)
{
    char out[4];
    int i;
    for(i = 0; i < len; ++i) {
        top_base64_encode_char(ctx,in[i],out);
    }
}

void top_base64_encode_string(struct top_base64_ctx* ctx, const char* in)
{
    char out[4];
    while(*in) {
        top_base64_encode_char(ctx,*in++,out);
    }
}

void top_base64_encode_finish(struct top_base64_ctx* ctx)
{
    uint32_t encoded;
    char out[4];
    switch(ctx->waiting) {
    case BASE64_ENC_UNIT_SIZE:
        return;
    case BASE64_ENC_UNIT_SIZE - 2:
        out[3] = ctx->padding;
        encoded = ctx->in << BASE64_DEC_MAP_SHIFT;
        out[2] = ctx->etable[(encoded >> BASE64_ENC_MAP_SHIFT) & BASE64_ENC_MAP_MASK];
        out[1] = ctx->etable[(encoded >> (2 * BASE64_ENC_MAP_SHIFT)) & BASE64_ENC_MAP_MASK];
        out[0] = ctx->etable[(encoded >> (3 * BASE64_ENC_MAP_SHIFT)) & BASE64_ENC_MAP_MASK];
        break;
    case BASE64_ENC_UNIT_SIZE - 1:
        out[3] = ctx->padding;
        out[2] = ctx->padding;
        encoded = ctx->in << (BASE64_DEC_MAP_SHIFT * 2);
        out[1] = ctx->etable[(encoded >> (2 * BASE64_ENC_MAP_SHIFT)) & BASE64_ENC_MAP_MASK];
        out[0] = ctx->etable[(encoded >> (3 * BASE64_ENC_MAP_SHIFT)) & BASE64_ENC_MAP_MASK];
        break;
    default:
        assert(0);
    }
    ctx->in = 0;
    ctx->waiting = BASE64_ENC_UNIT_SIZE;
    ctx->output(ctx->user_data,out,4);
}

static inline int top_base64_decode_char(struct top_base64_ctx* ctx,unsigned char c,char* out)
{
    uint32_t val = ctx->dtable[c];
    ctx->in <<= BASE64_ENC_MAP_SHIFT;
    if(val == 255) {
        ctx->tail = ctx->waiting;
    } else {
        assert(val < 64);
        ctx->in |= val;
    }
    if(--ctx->waiting == 0) {
        out[2] = ctx->in & BASE64_DEC_MAP_MASK;
        out[1] = (ctx->in >> BASE64_DEC_MAP_SHIFT) & BASE64_DEC_MAP_MASK;
        out[0] = (ctx->in >> (2 * BASE64_DEC_MAP_SHIFT)) & BASE64_DEC_MAP_MASK;
        ctx->output(ctx->user_data,out,3 - ctx->tail);
        ctx->in = 0;
        ctx->waiting = BASE64_DEC_UNIT_SIZE;
        ctx->tail = 0;
    }
}

int top_base64_decode(struct top_base64_ctx* ctx,const char* in,int len)
{
    char out[3];
    int i;
    for(i = 0; i < len; ++i) {
        top_base64_decode_char(ctx,in[i],out);
    }
    return ctx->waiting == BASE64_DEC_UNIT_SIZE;
}

int top_base64_decode_string(struct top_base64_ctx* ctx,const char* in)
{
    char out[3];
    while(*in) {
        top_base64_decode_char(ctx,*in++,out);
    }
    return ctx->waiting == BASE64_DEC_UNIT_SIZE;
}

