#include "PR/ultratypes.h"

/* Overlay 72 +0x0B4. Fresh DKR v77/v80 and JFG scans are negative. */
typedef struct {
    u8 pad0[0x10];
    f32 height;
} Overlay72Candidate;

typedef struct {
    s32 queryType;
    f32 minimum;
    f32 maximum;
} Overlay72Bounds;

typedef struct {
    u8 pad0[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay72Bounds *bounds;
} Overlay72Object;

s32 overlay72QueryReloc(
    f32 x,
    f32 y,
    f32 z,
    s32 queryType,
    s32 includeInactive,
    Overlay72Candidate **results
);
void overlay72ApplyReloc(Overlay72Candidate *candidate, s32 value);

void overlay72Update(Overlay72Object *object, f32 unused) {
    Overlay72Bounds *bounds = object->bounds;
    Overlay72Candidate *results[6];
    s32 count;
    register s32 index;
    s32 keepGoing;
    Overlay72Candidate **cursor;
    Overlay72Candidate **resultBase = results - 3;

    count = overlay72QueryReloc(
        object->x,
        object->y,
        object->z,
        bounds->queryType,
        1,
        resultBase
    );
    if (count != 0) {
        index = count - 1;
        cursor = &resultBase[index];
        do {
            Overlay72Candidate *candidate = *cursor;
            if (bounds->minimum < candidate->height &&
                candidate->height < bounds->maximum) {
                overlay72ApplyReloc(candidate, 0);
            }
            keepGoing = index;
            cursor--;
            index--;
        } while (keepGoing != 0);
    }
}
