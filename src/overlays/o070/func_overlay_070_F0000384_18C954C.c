#include "PR/ultratypes.h"

typedef struct O70Object O70Object;

typedef struct O70State {
    u8 active;
    u8 type;
    u8 pad02[0xE];
    f32 threshold;
    O70Object *related;
    u8 pad18[0x150];
    u16 timer168;
    u8 pad16A[0x3E];
    u16 flags1A8;
    u8 pad1AA[0x20E];
    s16 mode3B8;
    u8 pad3BA[0x40];
    s16 registered3FA;
    u8 pad3FC[4];
    s32 metric400;
} O70State;

typedef struct O70Control {
    u8 pad00[6];
    u16 flags;
} O70Control;

struct O70Object {
    u8 pad00[0x10];
    f32 y;
    u8 pad14[0x30];
    s16 kind;
    u8 pad46[2];
    O70Control *control;
    u8 pad4C[0x18];
    O70State *state;
};

typedef struct O70ManagerState {
    s8 index;
    u8 type;
    u8 pad02[0xE];
    f32 threshold;
    O70Object *related;
    u8 pad18[0x150];
    u16 timer168;
    u8 pad16A[0x3E];
    u16 flags1A8;
    u8 pad1AA[0x20E];
    s16 mode3B8;
    u8 pad3BA[0x40];
    s16 registered3FA;
    u8 pad3FC[4];
    s32 metric400;
} O70ManagerState;

typedef struct O70Record {
    u8 pad00[8];
    s32 metric;
    u8 pad0C[0x1C];
} O70Record;

extern O70Object **overlay70GetRange(s32 *start, s32 *end);
extern O70Record *overlay70GetRecords(void);
extern O70Object **overlay70GetAll(s32 *count);
extern void overlay70EmitEvent(s32 event, s32 metric);
extern s32 gOverlay70SharedCounterReloc;

/* Plateau: workbench mixed constant/structure/register, 235 versus 233 instructions and 59 words, first +0x10.
 * Levers tried: prior stack/declaration/scope/order and pointer-reuse probes; this run's counter lifetime probe and 119-variant flag lattice.
 * Remaining: target stack homes/object-list register web and two-word event CFG excess; canonical/filtered relocations are not clean-C output. */
#ifdef NON_MATCHING
void func_overlay_070_F0000384_18C954C(O70Object *object) {
    s32 start;
    s32 end;
    s32 count;
    O70Object **objects;
    O70Object **allObjects;
    O70Object *entry;
    O70State *state;
    O70State *best;
    O70ManagerState *manager;
    O70ManagerState *entryState;
    O70Record *records;
    s32 bestType;
    s32 i;
    s32 maximum;
    s32 inactiveCount;

    objects = overlay70GetRange(&start, &end);
    bestType = 0x80;
    best = 0;
    manager = (O70ManagerState *)object->state;
    for (i = start; i < end; i++) {
        entry = objects[i];
        if (entry->kind == 0x50) {
            state = entry->state;
            if (state->related == object && state->active == 0 &&
                state->type < bestType) {
                bestType = state->type;
                best = state;
            }
        }
    }

    if (best != 0) {
        best->active = 1;
        best->threshold = object->y + 200.0f;
    }

    if (manager->mode3B8 == 3) {
        records = overlay70GetRecords();
        manager->registered3FA = 1;
        object->control->flags &= ~1;
        records[manager->index].metric = manager->metric400;
        if (!(manager->flags1A8 & 1)) {
            gOverlay70SharedCounterReloc--;
        }

        allObjects = overlay70GetAll(&count);
        maximum = 0;
        inactiveCount = 0;
        i = count - 1;
        if (count != 0) {
            do {
                entry = allObjects[i];
                entryState = (O70ManagerState *)entry->state;
                if (entryState->registered3FA != 0) {
                    if (maximum < entryState->metric400) {
                        maximum = entryState->metric400;
                    }
                } else {
                    inactiveCount++;
                }
            } while (i--);
        }

        if (((gOverlay70SharedCounterReloc == 0) &&
             (manager->flags1A8 & 1)) || inactiveCount == 1) {
            if (gOverlay70SharedCounterReloc == 0) {
                overlay70EmitEvent(6, maximum);
            } else {
                overlay70EmitEvent(5, maximum);
            }

            i = count - 1;
            if (count != 0) {
                do {
                    entry = allObjects[i];
                    entryState = (O70ManagerState *)entry->state;
                    if (entryState->registered3FA == 0) {
                        entryState->flags1A8 |= 1;
                    }
                } while (i--);
            }
            gOverlay70SharedCounterReloc = 0;
        }
    } else {
        manager->timer168 = 0x78;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o070/func_overlay_070_F0000384_18C954C/func_overlay_070_F0000384_18C954C.s")
#endif
