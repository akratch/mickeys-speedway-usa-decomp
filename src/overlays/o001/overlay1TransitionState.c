#include "PR/ultratypes.h"

typedef struct Transform {
    s16 rotY;
    s16 rotX;
    s16 rotZ;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 alpha;
    u8 pad3A[0xE];
    struct ObjectHeader *header;
} Transform;

typedef struct ObjectHeader {
    u8 pad00[6];
    u16 flags;
} ObjectHeader;

typedef struct Spawned {
    s16 angle;
    u8 pad02[2];
    f32 y;
    u8 pad08[4];
    f32 x;
    union {
        f32 y2;
        u16 flags;
    } at10;
    f32 z;
    u8 pad18[0x2C];
    f32 x2;
    f32 z2;
} Spawned;

typedef struct State {
    u8 pad000[0x16C];
    u8 done;
    u8 pad16D[3];
    u8 phase;
    u8 pad171[0xB];
    s16 pathId;
    u8 selectorA;
    u8 selectorB;
    u8 selectorC;
    u8 pad181[0xF];
    u8 fade;
    u8 active;
    u8 pad192[0x16];
    u16 flags;
    u8 pad1AA[0x236];
    Spawned *spawned;
} State;

extern s32 G_o1_83e4;
extern void ext_o7_ccc(Transform *, s32);
extern Spawned *local_378(State *);
extern void ext_o0_1bed0(Transform *, f32, f32, f32, s16, s16, s16);
extern void ext_o0_1c6bc(Transform *, State *);
extern void ext_o0_5a914(Transform *, s32, s32, s32);
extern Spawned *local_414(s16, Spawned **);
extern s16 local_c0(Spawned *);

void func_overlay_001_F0003FD8_18503B8(Transform *obj, State *state, s32 updateRate) {
    Spawned *sp3C;
    s32 value;
    s32 phaseValue;
    u8 phase;
    u8 index;
    u8 *point;
    Spawned *spawned;

    if (G_o1_83e4 == 3) {
        phase = state->phase;
        if (phase == 0) {
            return;
        }
        phaseValue = phase;
        if (phase == 1) {
                ext_o7_ccc(obj, 0x13);
                state->spawned = local_378(state);
                state->phase = 2;
                return;
            }
            if (phaseValue == 2) {
                value = state->fade - (updateRate * 4);
                if (value <= 0) {
                    state->phase = 3;
                    return;
                }
                state->fade = value;
                return;
            }
            if (phaseValue == 3) {
                spawned = state->spawned;
                obj->x = spawned->x;
                obj->y = spawned->at10.y2 + 100.0f;
                obj->z = spawned->z;
                obj->rotX = 0;
                obj->rotZ = 0;
                obj->rotY = spawned->angle;
                ext_o0_1bed0(obj, obj->x, obj->y, obj->z, obj->rotY, obj->rotX, obj->rotZ);
                ext_o0_1c6bc(obj, state);
                state->flags &= ~8;
                obj->header->flags &= ~1;
                state->fade = 0;
                state->active = 1;
                state->phase = 4;
                return;
            }
            if (phaseValue == 4) {
                value = state->fade + (updateRate * 4);
                if (value >= 255) {
                    state->fade = 255;
                    state->phase = 5;
                    return;
                }
                state->fade = value;
                return;
            }
            if (phaseValue == 5) {
                obj->header->flags |= 1;
                state->phase = 0;
                state->active = 0;
                state->done = 1;
                ext_o0_5a914(obj, 12, -1, 0);
                state->spawned = 0;
        }
    } else {
        phase = state->phase;
        if (phase == 0) {
            return;
        }
        phaseValue = phase;
        if (phase == 1) {
                ext_o7_ccc(obj, 0x13);
                state->spawned = local_414(state->pathId, &sp3C);
                state->pathId = local_c0(sp3C);
                state->spawned->at10.flags |= 8;
                state->phase = 2;
                return;
            }
            if (phaseValue == 2) {
                value = obj->alpha - (updateRate * 4);
                if (value <= 0) {
                    state->phase = 3;
                    return;
                }
                obj->alpha = value;
                return;
            }
            if (phaseValue == 3) {
                spawned = state->spawned;
                index = 3;
                state->selectorA = index;
                state->selectorB = index;
                state->selectorC = 0;
                point = (u8 *)spawned + (index << 4);
                obj->x = *(f32 *)(point + 0x14);
                obj->y = spawned->y + 100.0f;
                point += 0x14;
                obj->z = *(f32 *)(point + 4);
                obj->rotX = 0;
                obj->rotZ = 0;
                obj->rotY = *(s16 *)((u8 *)spawned + 0xC) + 0x4000;
                ext_o0_1bed0(obj, obj->x, obj->y, obj->z, obj->rotY, obj->rotX, obj->rotZ);
                ext_o0_1c6bc(obj, state);
                state->flags &= ~8;
                obj->header->flags &= ~1;
                obj->alpha = 0;
                state->active = 1;
                state->phase = 4;
                return;
            }
            if (phaseValue == 4) {
                value = obj->alpha + (updateRate * 4);
                if (value >= 255) {
                    obj->alpha = 255;
                    state->phase = 5;
                    return;
                }
                obj->alpha = value;
                return;
            }
            if (phaseValue == 5) {
                obj->header->flags |= 1;
                state->phase = 0;
                state->active = 0;
                state->done = 1;
                ext_o0_5a914(obj, 12, -1, 0);
                state->spawned->at10.flags &= 0xFFF7;
                state->spawned = 0;
        }
    }
}
