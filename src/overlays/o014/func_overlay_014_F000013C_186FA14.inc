#include "PR/ultratypes.h"
#ifndef LEFT_STORAGE
#define LEFT_STORAGE
#endif
typedef struct Rule { u8 left, operation, right, field114; s16 value, field110; } Rule;
extern s32 gOverlay14StateC4, gOverlay14Context10C, gOverlay14Field110, gOverlay14Field114;
extern s16 *gOverlay14AssetF0;
extern void *gOverlay14OffsetsF4;
extern void overlay14ResetReloc(void);
extern void *overlay14AllocateReloc(s32 size);
extern s32 overlay14GetVariantReloc(void);
#ifndef TEST_DECL
#define TEST_DECL extern s32 overlay14TestRule(s32 rule);
#endif
TEST_DECL
extern s32 overlay14ApplyValues(s32 value, s32 mode);
extern void overlay14FinishLoad(void);
#ifndef LOAD_HEADER
#define LOAD_HEADER void func_overlay_014_F000013C_186FA14(s32 group, s32 context)
#endif
LOAD_HEADER {
    s16 *cursor;
    Rule *rule;
    s32 count;
    LEFT_STORAGE s32 left;
    s32 right;
    s32 variant;
    s32 size;
    overlay14ResetReloc();
    if (group < 0) return;
    if (group >= 0) return;
    gOverlay14Context10C = context;
    gOverlay14AssetF0 = overlay14AllocateReloc(8);
    variant = overlay14GetVariantReloc();
    switch (variant) {
        case 1: size = 0xB; break;
        case 2: size = 0xD; break;
        case 3: size = 0xF; break;
        case 5: size = 0x11; break;
        default: size = 9; break;
    }
    gOverlay14OffsetsF4 = overlay14AllocateReloc(size);
    cursor = gOverlay14AssetF0;
#ifndef CURSOR_LOOP
    if (group-- != 0) {
        do {
            cursor = (s16 *)((u8 *)cursor + (*cursor * 8) + 2);
        } while (group-- != 0);
    }
#else
    CURSOR_LOOP
#endif
    rule = (Rule *)(cursor + 1);
    count = *cursor - 1;
#ifndef RULE_INIT
    if (count != 0) {
        count--;
#else
    RULE_INIT
#endif
        do {
            left = overlay14TestRule(rule->left & 0x7F);
            if (rule->left & 0x80) left = (left == 0);
#ifdef TEST_RIGHT_ARG
            right = overlay14TestRule(rule->right & 0x7F, left);
#else
            right = overlay14TestRule(rule->right & 0x7F);
#endif
#ifndef RIGHT_ADJUST
            if (rule->right & 0x80) right = (right == 0);
#else
            RIGHT_ADJUST
#endif
            if (rule->operation == 0) { if (left && right) break; }
            else if (rule->operation == 1) { if (left || right) break; }
            else if (left) break;
            rule++;
        } while (count-- != 0);
    }
    gOverlay14Field110 = rule->field110;
    gOverlay14Field114 = rule->field114;
    if (overlay14ApplyValues(rule->value, 3) != 0) gOverlay14StateC4 = 1;
    overlay14FinishLoad();
}
