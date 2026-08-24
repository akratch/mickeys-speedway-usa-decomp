#define CURSOR_LOOP \
    right = group--; \
    if (right != 0) { \
        do { cursor = (s16 *)((u8 *)cursor + (*cursor * 8) + 2); right = group--; } \
        while (right != 0); \
    }
#define RULE_INIT right = count--; if (right != 0) {
#ifdef NON_MATCHING
#include "func_overlay_014_F000013C_186FA14.inc"
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/func_overlay_014_F000013C_186FA14/func_overlay_014_F000013C_186FA14.s")
#endif
