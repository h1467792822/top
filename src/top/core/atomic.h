#ifndef TOP_CORE_ATOMIC_H
#define TOP_CORE_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#define top_add_and_fetch(var,v) __sync_add_and_fetch(var,v)
#define top_sub_and_fetch(var,v) __sync_sub_and_fetch(var,v)
#define top_xor_and_fetch(var,v) __sync_xor_and_fetch(var,v)
#define top_and_and_fetch(var,v) __sync_and_and_fetch(var,v)
#define top_or_and_fetch(var,v) __sync_or_and_fetch(var,v)
#define top_nand_and_fetch(var,v) __sync_nand_and_fetch(var,v)


#define top_fetch_and_add(var,v) __sync_fetch_and_add(var,v)
#define top_fetch_and_sub(var,v) __sync_fetch_and_sub(var,v)
#define top_fetch_and_xor(var,v) __sync_fetch_and_xor(var,v)
#define top_fetch_and_and(var,v) __sync_fetch_and_and(var,v)
#define top_fetch_and_or(var,v) __sync_fetch_and_or(var,v)
#define top_fetch_and_nand(var,v) __sync_fetch_and_nand(var,v)

#define top_synchronize() __sync_synchronize()

#define top_bool_cas(pv,ov,nv) __sync_bool_compare_and_swap(pv,ov,nv)
#define top_val_cas(pv,ov,nv) __sync_val_compare_and_swap(pv,ov,nv)

#define top_atomic_set(pv,v) (*(pv) = (v))

#define top_atomic_inc(pv) ((void)top_add_and_fetch(pv,1))

#define top_atomic_dec(pv) ((void)top_sub_and_fetch(pv,1))

#define top_atomic_add(pv,v) ((void)top_sub_and_fetch(pv,v))

#define top_atomic_sub(pv,v) ((void)top_sub_and_fetch(pv,v))

#ifdef __cplusplus
}
#endif


#endif

