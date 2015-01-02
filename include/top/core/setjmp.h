
#ifndef TOP_CORE_SETJMP_H
#define TOP_CORE_SETJMP_H

#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef jmp_buf top_jmp_buf;
#define top_setjmp(ctx) _setjmp(ctx)
#define top_longjmp(ctx) _longjmp(ctx)


#ifdef __cplusplus
}
#endif

#endif

