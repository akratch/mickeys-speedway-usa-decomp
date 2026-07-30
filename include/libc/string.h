#ifndef _STRING_H_
#define _STRING_H_

#include "PR/ultratypes.h"

void *memcpy(void *dst, const void *src, size_t size);
size_t strlen(const char *str);
char *strchr(const char *str, int ch);

#endif /* _STRING_H_ */
