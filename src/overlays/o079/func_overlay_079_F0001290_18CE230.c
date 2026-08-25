#include "PR/ultratypes.h"

typedef struct Overlay79Object Overlay79Object;
typedef struct Overlay79Node Overlay79Node;
typedef struct Overlay79Spawned Overlay79Spawned;

typedef struct Overlay79Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay79Vector;

struct Overlay79Node {
    s32 active;
    Overlay79Node *next;
    u8 pad8[0x5C];
    Overlay79Spawned *state;
};

struct Overlay79Object {
    u8 pad0[0xC];
    Overlay79Vector position;
    u8 pad18[0x4C];
    Overlay79Node *node;
};

struct Overlay79Spawned {
    u8 pad0[0x3C];
    s32 field3C;
    s32 field40;
};

typedef struct Overlay79SpawnDesc {
    s16 objectId;
    u8 kind;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
    s16 angle;
    u8 padC[0xC];
} Overlay79SpawnDesc;

extern s32 overlay79RandomReloc(s32 lower, s32 upper);
extern Overlay79Spawned *overlay79SpawnReloc(Overlay79SpawnDesc *desc, s32 count);
extern void overlay79EmitAtReloc(s32 id, f32 x, f32 y, f32 z, s32 arg4, s32 arg5);
extern void overlay79FinishReloc(Overlay79Object *object);
extern s32 overlay79FindNearby(Overlay79Vector *position, f32 distance);
extern void overlay79EmitReloc(s32 id, s32 arg1);
extern void overlay79TriggerReloc(void);

extern s32 gOverlay79CounterReloc;
extern u8 gOverlay79FlagsReloc[];

/*
 * Plateau (2026-08-25, 6 attempts): the canonical -O2 candidate has the
 * exact 123-word size, differs in 12 words, and first diverges at +0xC8.
 * The remaining differences are register choices in the two linked-state
 * stores and the counter/flag tail; direct, scoped, assignment-expression,
 * and declaration-order spellings all retained or worsened that register web.
 * Revalidated on 2026-08-25: the full 119-combination lattice retained the
 * 12-word result. Direct chained access worsened it to 18 words, a volatile
 * pointer changed size and 83 words, and a 10-minute two-worker permuter run
 * improved score 150 to 130 only by inserting an empty goto/label artifact;
 * no zero-score candidate was found, so the idiomatic source remains.
 */
#ifdef NON_MATCHING
void func_overlay_079_F0001290_18CE230(Overlay79Object *object, s32 arg1) {
    Overlay79Node *node;
    Overlay79SpawnDesc desc;
    Overlay79Spawned *spawned;

    node = object->node;
    if (node->active != 0) {
        desc.objectId = 0x14B;
        desc.kind = 0xC;
        desc.flags = 0;
        desc.x = object->position.x;
        desc.y = object->position.y;
        desc.z = object->position.z;
        desc.angle = overlay79RandomReloc(-0x7FFF, 0x8000);
        spawned = overlay79SpawnReloc(&desc, 1);
        if (spawned != 0) {
            spawned->field3C = 0;
            overlay79EmitAtReloc(0x277, object->position.x, object->position.y,
                                 object->position.z, 4, 0);
        }
        spawned = node->next->state;
        spawned->field40 = 0;
        overlay79FinishReloc(object);
    } else if (overlay79FindNearby(&object->position, 900.0f) != 0) {
        desc.objectId = 0x14B;
        desc.kind = 0xC;
        desc.flags = 0;
        desc.x = object->position.x;
        desc.y = object->position.y;
        desc.z = object->position.z;
        desc.angle = overlay79RandomReloc(-0x7FFF, 0x8000);
        spawned = overlay79SpawnReloc(&desc, 1);
        if (spawned != 0) {
            spawned->field3C = 0;
            if (++gOverlay79CounterReloc == 0x14) {
                overlay79EmitReloc(0x27C, 0);
                gOverlay79FlagsReloc[1] |= 4;
                overlay79TriggerReloc();
            } else {
                overlay79EmitAtReloc(0x277, object->position.x,
                                     object->position.y, object->position.z,
                                     4, 0);
            }
        }
        spawned = node->next->state;
        spawned->field40 = 0;
        overlay79FinishReloc(object);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o079/func_overlay_079_F0001290_18CE230/func_overlay_079_F0001290_18CE230.s")
#endif
