#include "PR/ultratypes.h"

typedef struct O92CourseEntry {
    u8 pad00[8];
    u16 start;
    u16 end;
    s8 value;
} O92CourseEntry;

typedef struct O92Object {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x2C];
    s16 type;
    u8 pad46[0x1E];
    O92CourseEntry *course;
} O92Object;

typedef struct O92VehicleState {
    u8 pad00[0x3B2];
    u8 coursePosition;
} O92VehicleState;

typedef struct O92Racer {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    O92VehicleState *vehicle;
} O92Racer;

extern f32 gOverlay92DistanceParameters[2];
extern O92Object **overlay92GetObjectRange(s32 *start, s32 *end);
extern f32 sqrtf(f32 value);

/* Workbench: mixed structural/schedule/allocation; best 16 words, first +0x48.
 * Integer pool-position reads cut 26 to 16, but require non-source empty guards.
 * Remaining: index/cursor swap, one schedule pair, and first-distance FP coloring. */
#ifdef NON_MATCHING
s32 func_overlay_092_F0000068_18D5F88(O92Racer *racer, f32 *outX,
                                      f32 *outY, f32 *outZ, s32 *outValue) {
    O92Object **objects;
    s32 start;
    s32 end;
    O92Object **cursor;
    O92Object *object;
    O92Object *nearest;
    O92VehicleState *vehicle;
    s32 index;
    s32 limit;
    s32 valid;
    O92CourseEntry *course;
    f32 nearestDistance;
    f32 distance;
    f32 scale;
    f32 dx;
    f32 dy;
    f32 dz;

    vehicle = racer->vehicle;
    objects = overlay92GetObjectRange(&start, &end);
    nearestDistance = gOverlay92DistanceParameters[0];
    nearest = 0;

    if (start < end) {
        index = start * 4;
        cursor = (O92Object **)((u8 *)objects + index);
        do {
            object = *cursor;
            if (object->type != 12) {
                limit = end * 4;
            } else {
                course = object->course;
                valid = 0;
                if (course->start < course->end) {
                    if ((vehicle->coursePosition >= course->start) &&
                        (vehicle->coursePosition < course->end)) {
                        valid = 1;
                    }
                } else if ((vehicle->coursePosition >= course->start) ||
                           (vehicle->coursePosition < course->end)) {
                    valid = 1;
                }
                if (valid != 0) {
                    dx = object->x - racer->x;
                    dy = object->y - racer->y;
                    dz = object->z - racer->z;
                    distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
                    if (distance < nearestDistance) {
                        nearestDistance = distance;
                        nearest = object;
                    }
                }
                limit = end * 4;
            }
            index += 4;
            cursor++;
        } while (index < limit);
    }

    if (nearest != 0) {
        dx = nearest->x - racer->x;
        dy = nearest->y - racer->y;
        dz = nearest->z - racer->z;
        course = nearest->course;
        distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        nearestDistance = gOverlay92DistanceParameters[1];
        scale = 60.0f / distance;
        dx *= scale;
        dy *= scale;
        dz *= scale;
        *outX = racer->x + dx;
        *outY = racer->y + dy;
        *outZ = racer->z + dz;
        dx = *outX - nearest->x;
        dy = *outY - nearest->y;
        dz = *outZ - nearest->z;
        *outX = nearest->x + (dx * nearestDistance);
        *outY = nearest->y + (dy * nearestDistance);
        *outZ = nearest->z + (dz * nearestDistance);
        *outValue = course->value;
        return 1;
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o092/overlay92FindNearestCourse/func_overlay_092_F0000068_18D5F88.s")
#endif
