#include "PR/ultratypes.h"

typedef struct Overlay58OrderEntry {
    u8 pad00[3];
    u8 gap03;
    s32 value04;
    u8 pad08[0x14];
    u8 progress1C;
    u8 pad1D[5];
    u16 rank22;
    u8 pad24[4];
} Overlay58OrderEntry;

typedef struct Overlay58OrderState {
    u8 mode;
    u8 active;
    u8 player;
    u8 countdown;
    Overlay58OrderEntry entries[6];
} Overlay58OrderState;

typedef struct Overlay58TagSource {
    u8 pad00[0x2A];
    s8 tag2A;
    u8 pad2B[9];
} Overlay58TagSource;

typedef struct Overlay58SnapshotSource {
    u8 pad00[0xC];
    s32 value0C;
    s32 value10;
    s32 value14;
    u8 pad18[0x1D4];
    s32 value1EC;
} Overlay58SnapshotSource;

typedef struct Overlay58GameState {
    u8 pad00[0x50];
    s32 value50;
    u8 pad54[0x3C];
    s32 value90;
    s32 value94;
    s32 value98;
    u8 pad9C[0x124];
    s32 value1C0;
} Overlay58GameState;

extern Overlay58SnapshotSource gOverlay58SnapshotSourceReloc;
extern Overlay58TagSource gOverlay58TagSourcesReloc[];
extern Overlay58TagSource gOverlay58TagSourcesEndReloc[];
extern Overlay58GameState *gOverlay58GameStateReloc;
extern u8 gOverlay58EntryCountReloc;
extern u8 gOverlay58UpdateGateReloc;
extern u16 gOverlay58PackedFlagsReloc;
extern s32 gOverlay58ModeReloc;
extern s32 gOverlay58FrameCounterReloc;

extern s32 D_18[4];
extern s32 D_28;
extern s32 D_2C;
extern s32 D_30;
extern s32 D_6C;
extern s32 D_74;
extern Overlay58OrderEntry *D_78[6];
extern Overlay58OrderEntry *D_90[6];
extern s32 D_A8[6];
extern s32 D_C0[6];
extern u8 D_C8[];
extern s8 D_F8[6];
extern s32 D_108[];
extern s32 D_120;
extern s32 D_140;
extern s32 D_160;
extern s32 D_180;
extern s32 D_D8;
extern s32 D_DC;
extern s32 D_E0;
extern u8 D_E8[];

extern Overlay58OrderState *overlay58GetOrderStateReloc(void);
extern void overlay58ResetOrderStateReloc(void *state);
extern void overlay58BeginOrderUpdateReloc(void);
extern void overlay58SplitOrderValueReloc(s32 value, s32 *part0, s32 *part1,
                                          s32 *part2);
extern void overlay58BuildOrderReloc(void *state, s32 count, s32 gap);
extern void overlay58FinalizePackedStatus(void);

/* Every Reloc name is provisional; normalized assembly does not bind it. */
/* Workbench: structure-mismatch; 333 words differ, 351 versus 368 instructions, first mismatch +0x0.
 * Lever: constant-audit found only frame offsets; scalar, array, and aggregate split-results compile equivalently.
 * Remains: 24-byte non-save frame surplus and missing 17-instruction loop/relocation structure. */
#ifdef NON_MATCHING
void func_overlay_058_F0000000_18AF1E8(void) {
    register s32 i;
    s32 left0;
    s32 right0;
    s32 left1;
    s32 right1;
    s32 left2;
    s32 right2;
    Overlay58OrderState *state;
    Overlay58OrderEntry *entry;
    Overlay58OrderEntry *next;
    register Overlay58OrderEntry **orderCursor;
    register Overlay58OrderEntry **rankCursor;
    Overlay58TagSource *tagSource;
    s8 *tag;
    register s32 count;
    register s32 limit;
    register s32 swapped;
    register s32 gap;

    state = overlay58GetOrderStateReloc();
    overlay58ResetOrderStateReloc(D_C8);
    D_120 = gOverlay58SnapshotSourceReloc.value0C;
    D_140 = gOverlay58SnapshotSourceReloc.value10;
    D_160 = gOverlay58SnapshotSourceReloc.value14;
    D_180 = gOverlay58SnapshotSourceReloc.value1EC;
    D_30 = 0;
    overlay58BeginOrderUpdateReloc();

    tag = D_F8;
    tagSource = gOverlay58TagSourcesReloc;
    do {
        *tag++ = tagSource->tag2A;
        tagSource++;
    } while (tagSource < gOverlay58TagSourcesEndReloc);

    D_6C = state->active;
    count = gOverlay58EntryCountReloc;
    if (count > 0) {
        orderCursor = D_78;
        rankCursor = D_90;
        entry = &state->entries[0];
        do {
            rankCursor++;
            *orderCursor = entry;
            rankCursor[-1] = entry;
            entry++;
            orderCursor++;
        } while (rankCursor < &D_90[count]);
    }

    limit = count - 1;
    if (state->mode == 5) {
        do {
            swapped = 0;
            orderCursor = D_78;
            rankCursor = &D_78[limit];
            while (orderCursor < rankCursor) {
                entry = orderCursor[0];
                next = orderCursor[1];
                if (entry->value04 < next->value04) {
                    orderCursor[0] = next;
                    orderCursor[1] = entry;
                    swapped = 1;
                }
                orderCursor++;
            }
        } while (swapped != 0);
    } else {
        do {
            swapped = 0;
            orderCursor = D_78;
            rankCursor = &D_78[limit];
            while (orderCursor < rankCursor) {
                entry = orderCursor[0];
                next = orderCursor[1];
                if (next->value04 < entry->value04) {
                    orderCursor[0] = next;
                    orderCursor[1] = entry;
                    swapped = 1;
                }
                orderCursor++;
            }
        } while (swapped != 0);
    }

    D_A8[0] = 0;
    for (i = 1; i < gOverlay58EntryCountReloc; i++) {
        overlay58SplitOrderValueReloc(D_78[i - 1]->value04, &left0, &left1,
                                      &left2);
        overlay58SplitOrderValueReloc(D_78[i]->value04, &right0, &right1,
                                      &right2);
        if ((left0 == right0) && (left1 == right1) && (left2 == right2)) {
            D_A8[i] = D_A8[i - 1];
        } else {
            D_A8[i] = i;
        }
    }

    for (i = 0; i < gOverlay58EntryCountReloc; i++) {
        if (&state->entries[0] == D_78[i]) {
            D_74 = i;
            i = 6;
        }
    }
    i = 0;

    if (((D_74 < 4) || (gOverlay58UpdateGateReloc != 0)) &&
        (state->mode != 1)) {
        for (i = 0; i < gOverlay58EntryCountReloc; i++) {
            entry = D_78[i];
            ((u8 *)entry)[D_A8[i] + 0x1C]++;
            entry->rank22 += D_108[D_A8[i]];
            if (entry->rank22 >= 10000) {
                entry->rank22 = 9999;
            }
        }
        i = 0;
    } else if ((state->active != 0) &&
               ((gOverlay58PackedFlagsReloc & 0x100) == 0)) {
        state->countdown--;
    }

    limit = gOverlay58EntryCountReloc - 1;
    do {
        swapped = 0;
        for (i = 0; i < limit; i++) {
            entry = D_90[i];
            next = D_90[i + 1];
            if (entry->rank22 < next->rank22) {
                D_90[i] = next;
                D_90[i + 1] = entry;
                swapped = 1;
            }
        }
        i = 0;
    } while (swapped != 0);

    {
        Overlay58OrderEntry **cursor;

        cursor = D_90;
        for (i = 0; i < gOverlay58EntryCountReloc; i++, cursor++) {
            entry = *cursor;
            gap = D_90[0]->rank22 - entry->rank22;
            if (gap >= 101) {
                gap = 100;
            }
            entry->gap03 = gap;
        }
    }

    D_C0[0] = 0;
    for (i = 1; i < gOverlay58EntryCountReloc; i++) {
        entry = D_90[i - 1];
        next = D_90[i];
        if (entry->rank22 == next->rank22) {
            D_C0[i] = D_C0[i - 1];
        } else {
            D_C0[i] = i;
        }
    }

    D_D8 = 0;
    overlay58BuildOrderReloc(D_E8, 10, gap);
    D_2C = 0;
    if (gOverlay58ModeReloc == 3) {
        D_28 = 3;
    } else {
        D_28 = 0;
    }

    if (state->mode != 5) {
        D_18[0] = gOverlay58GameStateReloc->value90;
    } else {
        D_18[0] = gOverlay58GameStateReloc->value1C0;
    }
    D_18[1] = gOverlay58GameStateReloc->value94;
    D_18[2] = gOverlay58GameStateReloc->value98;
    D_18[3] = gOverlay58GameStateReloc->value50;

    overlay58FinalizePackedStatus();
    D_DC = 1;
    D_E0 = 0;
    gOverlay58FrameCounterReloc++;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/func_overlay_058_F0000000_18AF1E8/func_overlay_058_F0000000_18AF1E8.s")
#endif
