
#ifndef TOP_CORE_ERROR_H
#define TOP_CORE_ERROR_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct top_error_ {
    int	__errno__;
} top_error_t;

#define top_errno(e) (e.__errno__)

static inline top_error_t top_make_error(int v)
{
    top_error_t ret = {v};
    return ret;
}

static const struct top_error_ TOP_OK = { 0 };

#define TOP_ERROR(e) top_make_error(e)

#ifdef __cplusplus
}
#endif

#endif

