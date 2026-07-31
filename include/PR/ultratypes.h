#ifndef _ULTRATYPES_H_
#define _ULTRATYPES_H_

/*
 * Minimal Ultra64 scalar type set, written from scratch for this project.
 *
 * These typedefs are not a design choice: they are dictated by the o32 ABI the
 * ROM was built against (32-bit `long`, 64-bit `long long`), and every N64
 * decompilation ends up with the same spellings because the compiler's codegen
 * depends on them.  Only the names/types actually needed by matched code live
 * here; extend as new translation units require more.
 */

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef short s16;
typedef long s32;
typedef long long s64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned long vu32;
typedef volatile unsigned long long vu64;

typedef volatile signed char vs8;
typedef volatile short vs16;
typedef volatile long vs32;
typedef volatile long long vs64;

typedef float f32;
typedef double f64;

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif /* _LANGUAGE_C */

#endif /* _ULTRATYPES_H_ */
