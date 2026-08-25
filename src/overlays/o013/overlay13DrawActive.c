#include "PR/ultratypes.h"

typedef struct O13Record {
    u8 pad00[6];
    u8 active;
    u8 pad07[5];
    f32 x;
    f32 z;
    f32 y;
    u8 pad18[0x68];
} O13Record;

typedef struct O13View {
    u8 pad00[0x0C];
    f32 x;
    f32 z;
    f32 y;
} O13View;

extern s32 D_1000;
extern s32 D_2C;
extern O13Record D_0[];
extern O13View *o13GetView(void);
extern void func_overlay_013_F0000580_186F098(O13Record *, s32, s32, s32);

/* Plateau (2026-08-25): 165/166 words, 163 differ, first +0x0; frame 0x180 vs 0x168.
 * Reversed arrays reproduce target stack order; the lattice and ten ABI/lifetime variants failed.
 * A 40-minute permuter reached 190, but its read alias enlarged the frame to 0x188. */
#ifdef NON_MATCHING
void overlay13DrawActive(s32 arg0, s32 arg1, s32 arg2) {
    s32 indices[32];
    f32 distances[35];
    O13View *view;
    O13Record *record;
    s32 count;
    s32 recordIndex;
    s32 i;
    s32 done;
    f32 dx;
    f32 dz;
    f32 dy;
    f32 temporaryDistance;
    s32 temporaryIndex;

    if (D_1000 != 0 && D_2C != 0) {
        view = o13GetView();
        record = D_0;
        count = 0;
        recordIndex = 0;
        do {
            if (record->active != 0) {
                dx = record->x - view->x;
                dz = record->z - view->z;
                dy = record->y - view->y;
                indices[count] = recordIndex;
                distances[count] = dx * dx + dz * dz + dy * dy;
                count++;
            }
            record++;
            if (record->active != 0) {
                dx = record->x - view->x;
                dz = record->z - view->z;
                dy = record->y - view->y;
                indices[count] = recordIndex + 1;
                distances[count] = dx * dx + dz * dz + dy * dy;
                count++;
            }
            record++;
            recordIndex += 2;
        } while (recordIndex != 32);

        if (count >= 2) {
            do {
                done = 1;
                for (i = 0; i < count - 1; i++) {
                    if (distances[i + 1] < distances[i]) {
                        temporaryDistance = distances[i];
                        distances[i] = distances[i + 1];
                        distances[i + 1] = temporaryDistance;
                        temporaryIndex = indices[i];
                        indices[i] = indices[i + 1];
                        indices[i + 1] = temporaryIndex;
                        done = 0;
                    }
                }
            } while (done == 0);
        }

        for (i = 0; i < count; i++) {
            func_overlay_013_F0000580_186F098(
                &D_0[indices[i]], arg0, arg1, arg2);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o013/overlay13DrawActive/func_overlay_013_F0000874_186F38C.s")
#endif
