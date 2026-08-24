#ifndef _LIBC_STDLIB_H_
#define _LIBC_STDLIB_H_

/* Minimal SDK-compatible division result types used by libultra/ldiv.c. */

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

ldiv_t ldiv(long num, long denom);
lldiv_t lldiv(long long num, long long denom);

#endif /* _LIBC_STDLIB_H_ */
