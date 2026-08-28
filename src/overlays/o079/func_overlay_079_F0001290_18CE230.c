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

/* Workbench plateau (2026-08-28, 10 directed variants after baseline):
 * the best source-faithful candidate remains an exact 123-instruction, 492-byte
 * frame (-72) with 12 differing words and first mismatch at +0xC8. The
 * residual is allocation-only in the linked-state and counter/flag webs, with
 * shared-overlay relocation identity still requiring linked proof. Scoped
 * linked-state lifetimes, explicit next-node declarations, branch-local
 * spawned declarations, direct chained stores, flag-base pointers, explicit
 * byte stores, and counter-result locals either remained at 12 or worsened to
 * 18-59 words; every lifetime-local form that changed code also grew the frame
 * to -80. The counter assignment spelling was byte-identical to baseline.
 * The remaining blocker is the target's v0/v1 linked-state pool coloring and
 * t2/t3 counter/flag temp FIFO; no exact, relocation-complete, linked result
 * was found within the bounded campaign. */
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
