#include "PR/ultratypes.h"

typedef struct Overlay57SelectionChild {
    u8 reserved00[0x28];
    f32 value28;
    u8 reserved2C[0x0F];
    s8 selector3B;
} Overlay57SelectionChild;

typedef struct Overlay57SelectionResult {
    u8 reserved00[8];
    Overlay57SelectionChild *child;
} Overlay57SelectionResult;

typedef union Overlay57LocalWord {
    s32 word;
    f32 value;
} Overlay57LocalWord;

extern s32 gO57SelectionList8Reloc[];
extern s32 gO57SelectionList30Reloc[];
extern s32 gO57SelectionCurrent100Reloc[];
extern s32 gO57SelectionPending104Reloc;
extern s32 gO57SelectionPrevious108Reloc;
extern Overlay57LocalWord gO57SelectionValue10CReloc;
extern s32 gO57SelectionState118Reloc;
extern s32 gO57SelectionMode11CReloc;
extern s32 gO57SelectionTimer120Reloc;
extern s32 gO57SelectionIds134Reloc[];
extern s32 gO57SelectionActive144Reloc;
extern s32 gO57SelectionValues158Reloc[];
extern s32 gO57SelectionValues17CReloc[];
extern s32 gO57SelectionPrimary4E8Reloc;
extern s32 gO57SelectionSecondary4ECReloc;
extern s32 gO57SelectionChanging4F0Reloc;
extern s32 gO57SelectionDistance50CReloc;

extern u32 gOverlay57Flags37ECReloc;
extern s32 gOverlay57Mode38C4Reloc;
extern s16 gOverlay57Threshold38D8Reloc;
extern s16 gOverlay57Threshold3970Reloc;
extern s32 gOverlay57PublishedIndex3A28Reloc;

extern s32 overlay57Call35F8Reloc(void);
extern void overlay57Call3618Reloc(s32 *primary, s32 *secondary);
extern Overlay57SelectionResult *overlay57Call3684Reloc(u8 id);
extern void *overlay57Call36F4Reloc(u8 id);
extern Overlay57SelectionResult *overlay57Call3758Reloc(u8 id);
extern s32 overlay57Call37E0Reloc(void);
extern void overlay57Call3880Reloc(s32 command, s32 argument);
extern void overlay57Call3888Reloc(s32 argument);
extern s32 overlay57Call38ACReloc(void);
extern void overlay57Call3920Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call3940Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call3960Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call39B0Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call39D0Reloc(void *item, s32 x, s32 y, s32 z);
extern void overlay57Call39F0Reloc(void *item, s32 x, s32 y, s32 z);
extern void o57SelectionSetNodeValueReloc(s32 id, s32 argument, f32 value);

/* Overlay 57 text +0x35E0..+0x3A4C. */
#ifdef NON_MATCHING
void overlay57UpdateSelection(s32 ignored) {
    s32 newPrimary;
    s32 newSecondary;
    volatile s32 *primaryState;
    Overlay57SelectionResult *result;
    s32 sentinel;
    volatile s32 *list;
    s32 entry;

    (void)ignored;
    if (overlay57Call35F8Reloc() != 2) {
        return;
    }

    gO57SelectionActive144Reloc = 1;
    overlay57Call3618Reloc(&newPrimary, &newSecondary);
    primaryState = &gO57SelectionPrimary4E8Reloc;
    sentinel = -1;

    if ((newPrimary != *primaryState) ||
        (newSecondary != gO57SelectionSecondary4ECReloc)) {
        if ((*primaryState != 0xFF) &&
            (newPrimary != *primaryState)) {
            list = gO57SelectionIds134Reloc;
            while ((entry = *list) != sentinel) {
                if (entry == *primaryState) {
                    result = overlay57Call3684Reloc((u8)entry);
                    if (result != 0) {
                        if ((result->child != 0) &&
                            (newSecondary != result->child->selector3B)) {
                            o57SelectionSetNodeValueReloc(
                                entry, gO57SelectionValues17CReloc[entry],
                                0.007f);
                        }
                    }
                }
                list++;
            }
        }

        if (newPrimary != 0xFF) {
            if (overlay57Call36F4Reloc((u8)newPrimary) != 0) {
                o57SelectionSetNodeValueReloc(newPrimary, newSecondary, 0.01f);
                gO57SelectionChanging4F0Reloc = 1;
            }
        }
        *primaryState = newPrimary;
        gO57SelectionSecondary4ECReloc = newSecondary;
    }

    if ((gO57SelectionChanging4F0Reloc != 0) &&
        (*primaryState != 0xFF)) {
        result = overlay57Call3758Reloc((u8)*primaryState);
        if (result != 0) {
            if (gO57SelectionValue10CReloc.value < result->child->value28) {
                gO57SelectionChanging4F0Reloc = 0;
                list = gO57SelectionIds134Reloc;
                while ((entry = *list) != sentinel) {
                    if (entry == *primaryState) {
                        o57SelectionSetNodeValueReloc(
                            entry, gO57SelectionValues17CReloc[entry], 0.007f);
                    }
                    list++;
                }
            }
        }
    }

    if (overlay57Call37E0Reloc() != sentinel) {
        list = gO57SelectionIds134Reloc;
        while ((entry = *list) != sentinel) {
            o57SelectionSetNodeValueReloc(
                entry, gO57SelectionValues158Reloc[entry], 0.01f);
            list++;
        }
        gO57SelectionState118Reloc = 4;
        gO57SelectionTimer120Reloc = 10;
        return;
    }

    if (((gOverlay57Flags37ECReloc & 0x4000) != 0) &&
        (gO57SelectionDistance50CReloc == 0)) {
        overlay57Call3880Reloc(0xD, 0);
        overlay57Call3888Reloc(0);
        gO57SelectionState118Reloc = 5;
        gO57SelectionMode11CReloc = 0;
        return;
    }

    entry = overlay57Call38ACReloc();
    primaryState = gO57SelectionCurrent100Reloc;
    if (entry == *primaryState) {
        return;
    }

    if (gOverlay57Mode38C4Reloc == 1) {
        if ((gOverlay57Threshold38D8Reloc < -16) &&
            (gO57SelectionDistance50CReloc == 0)) {
            overlay57Call3920Reloc(
                ((void **)gO57SelectionList8Reloc)[entry], -160, 190, 4);
        } else {
            overlay57Call3940Reloc(
                ((void **)gO57SelectionList8Reloc)[entry], 480, 190, 4);
        }
        overlay57Call3960Reloc(
            ((void **)gO57SelectionList8Reloc)[*primaryState], 160, 260, 260);
    } else {
        if ((gOverlay57Threshold3970Reloc < -16) &&
            (gO57SelectionDistance50CReloc == 0)) {
            overlay57Call39B0Reloc(
                ((void **)gO57SelectionList30Reloc)[entry], -160, 190, 4);
        } else {
            overlay57Call39D0Reloc(
                ((void **)gO57SelectionList30Reloc)[entry], 480, 190, 4);
        }
        overlay57Call39F0Reloc(
            ((void **)gO57SelectionList30Reloc)[*primaryState], 160, 260, 260);
    }

    gO57SelectionPrevious108Reloc = *primaryState;
    *primaryState = entry;
    gO57SelectionValue10CReloc.word = gO57SelectionPending104Reloc;
    gO57SelectionPending104Reloc = 0xFF;
    gOverlay57PublishedIndex3A28Reloc = entry;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/overlay57UpdateSelection/func_overlay_057_F00035E0_18A71D8.s")
#endif
