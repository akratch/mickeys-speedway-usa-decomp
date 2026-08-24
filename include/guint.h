#ifndef _GUINT_H_
#define _GUINT_H_

typedef union {
    struct {
        unsigned int hi;
        unsigned int lo;
    } word;
    double d;
} du;

typedef union {
    unsigned int i;
    float f;
} fu;

extern float __libm_qnan_f;

#define ROUND(d) (int)(((d) >= 0.0) ? ((d) + 0.5) : ((d) - 0.5))

#endif
