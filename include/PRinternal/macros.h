#ifndef __MACROS_H__
#define __MACROS_H__

#define ALIGNED(x) __attribute__((aligned(x)))
#define ARRLEN(x) ((s32)(sizeof(x) / sizeof(x[0])))
#define STACK(stack, size) u64 stack[(((size) + 7) & ~7) / sizeof(u64)]
#define STACK_START(stack) ((u8 *)(stack) + sizeof(stack))

#ifndef __GNUC__
#define __attribute__(x)
#endif

#endif
