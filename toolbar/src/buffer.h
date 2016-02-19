
#ifndef TB_BUFFER_H
#define TB_BUFFER_H


#ifdef __cplusplus
extern "C" {
#endif


#define BUF_LEN (4 * 1024)

struct iovec;

int buff_vec_alloc(struct iovec* vec,int n);
void buff_vec_free(struct iovec* vec,int n);



#ifdef __cplusplus
}
#endif

#endif

