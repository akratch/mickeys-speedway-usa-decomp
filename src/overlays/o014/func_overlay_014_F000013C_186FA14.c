#define CURSOR_LOOP \
    right = group--; \
    if (right != 0) { \
        do { cursor = (s16 *)((u8 *)cursor + (*cursor * 8) + 2); right = group--; } \
        while (right != 0); \
    }
#define RULE_INIT right = count--; if (right != 0) {
#include "func_overlay_014_F000013C_186FA14.inc"
