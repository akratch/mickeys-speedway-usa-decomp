#include "PR/ultratypes.h"

/*
 * Plane-side priority selection reconstructed from Mickey's call and field
 * evidence; exact DKR v77/v80 and JFG scans are negative. IDO needs the
 * measured -Wab,-r4300_mul override to reproduce the shipped FP schedule.
 */
typedef struct Overlay21Plane {
    s16 id;
    s8 lowerPriority;
    s8 upperPriority;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 distance;
} Overlay21Plane;

typedef struct Overlay21Object {
    u8 pad0[0x64];
    Overlay21Plane *plane;
} Overlay21Object;

typedef struct Overlay21Position {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay21Position;

typedef struct Overlay21PriorityEntry {
    u8 pad0[2];
    s16 priority;
    Overlay21Position *position;
} Overlay21PriorityEntry;

extern s32 gOverlay21ObjectCount;
extern Overlay21Object *gOverlay21Objects[];

Overlay21Position *overlay21GetReferenceReloc(void);

/*
 * Plateau (2026-08-25, current run): -O2 -mips2 with -Wab,-r4300_mul is
 * exact-size and differs in ten of 114 masked words from +0x24.  Every
 * non-count word agrees; the source selects v1 for each reload of
 * gOverlay21ObjectCount while retail selects a0.  Volatile, cached-value,
 * and pointer spellings regress to 58 words.  A two-worker, ten-minute
 * permuter batch reached score 45 only with redundant aliases and
 * multiply-by-one expressions, and remained non-exact.
 */
#ifdef NON_MATCHING
void overlay21ApplyPriorities(s32 planeId, s32 entryCount,
                              Overlay21PriorityEntry **entries) {
    Overlay21Position *reference;
    Overlay21Plane *plane;
    Overlay21PriorityEntry *entry;
    Overlay21PriorityEntry **cursor;
    Overlay21Position *position;
    f32 distance;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 planeDistance;
    s32 planeIndex;
    s32 entryIndex;
    s32 continueLoop;

    reference = overlay21GetReferenceReloc();
    for (planeIndex = 0; planeIndex < gOverlay21ObjectCount; planeIndex++) {
        plane = gOverlay21Objects[planeIndex]->plane;
        if (plane->id == planeId) {
            normalX = plane->normalX;
            normalY = plane->normalY;
            normalZ = plane->normalZ;
            planeDistance = plane->distance;
            cursor = entries;
            distance = (reference->x * normalX) +
                       (reference->y * normalY) +
                       (reference->z * normalZ) + planeDistance;
            if (distance > 0.0f) {
                if (entryCount != 0) {
                    entryIndex = entryCount - 1;
                    do {
                        entry = *cursor++;
                        position = entry->position;
                        distance = (position->x * normalX) +
                                   (position->y * normalY) +
                                   (position->z * normalZ) + planeDistance;
                        if ((distance < 0.0f) &&
                            (plane->lowerPriority < entry->priority)) {
                            entry->priority = plane->lowerPriority;
                        }
                        continueLoop = entryIndex;
                        entryIndex--;
                    } while (continueLoop != 0);
                }
            } else if ((plane->upperPriority >= 0) && (entryCount != 0)) {
                entryIndex = entryCount - 1;
                do {
                    entry = *cursor++;
                    position = entry->position;
                    distance = (position->x * normalX) +
                               (position->y * normalY) +
                               (position->z * normalZ) + planeDistance;
                    if ((distance > 0.0f) &&
                        (plane->upperPriority < entry->priority)) {
                        entry->priority = plane->upperPriority;
                    }
                    continueLoop = entryIndex;
                    entryIndex--;
                } while (continueLoop != 0);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o021/overlay21ApplyPriorities/func_overlay_021_F000010C_1877D94.s")
#endif
