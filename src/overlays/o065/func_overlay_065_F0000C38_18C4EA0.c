#include "PR/ultratypes.h"

typedef struct Overlay65TrailRecord {
    f32 x[9];
    f32 y[9];
    f32 z[9];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 floorY;
    u8 active;
    u8 red;
    u8 green;
    u8 blue;
} Overlay65TrailRecord;

typedef struct Overlay65TrailVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay65TrailVertex;

typedef struct Overlay65TrailCamera {
    s16 angle;
    u8 pad02[10];
    f32 x;
    f32 y;
    f32 z;
} Overlay65TrailCamera;

typedef struct Overlay65TrailCommand {
    u32 w0;
    u32 w1;
} Overlay65TrailCommand;

extern Overlay65TrailRecord gOverlay65TrailRecords[50];
extern s16 gOverlay65TrailSpawnTimer;
extern f32 gOverlay65TrailGravity;
extern f32 D_2978;
extern f32 D_297C;
extern Overlay65TrailVertex *D_2988;
extern u8 D_800000C0[];

extern Overlay65TrailCamera *func_80021970(s32 index);
extern void camSetNo(s32 index);
extern void func_800221E8(Overlay65TrailCommand **commands, s32 **cursor);
extern s32 mathRnd(s32 lower, s32 upper);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void func_800349A4(Overlay65TrailCommand **commands, void *texture,
                          s32 flags, s32 parameter);
extern void func_overlay_065_F0001A14_18C5C7C(f32 x, f32 y, f32 z);

/* Workbench: structure-mismatch (mixed), 870/887 positional words differ; first mismatch +0x0.
 * Levers tried: explicit vertex-pair expansion and live-global pointer access reduced the size gap to 34 instructions.
 * Remaining: a 24-byte frame deficit, four missing saved registers, and structural/FP allocation drift. */
#ifdef NON_MATCHING
void func_overlay_065_F0000C38_18C4EA0(Overlay65TrailCommand **commandPtr,
                                       s32 **cursorPtr, s32 updateRate) {
    Overlay65TrailCommand *commands;
    Overlay65TrailCamera *camera;
    Overlay65TrailRecord *record;
    s32 *cursor;
    s32 recordIndex;
    s32 updateIndex;
    s32 pointIndex;
    s32 randomX;
    s32 randomZ;
    f32 sinAngle;
    f32 cosAngle;
    f32 spawnX;
    f32 spawnZ;

    commands = *commandPtr;
    cursor = *cursorPtr;
    camera = func_80021970(0);
    camSetNo(0);
    func_800221E8(&commands, &cursor);

    if (gOverlay65TrailSpawnTimer > 0) {
        gOverlay65TrailSpawnTimer -= updateRate;
    } else {
        randomX = mathRnd(-500, 500);
        randomZ = mathRnd(-500, 500);
        sinAngle = func_8002A8BC(camera->angle);
        cosAngle = func_8002A8C0(camera->angle);
        spawnX = (camera->x + (randomX * sinAngle)) -
                 (randomZ * cosAngle);
        spawnZ = camera->z + (randomZ * sinAngle) +
                 (randomX * cosAngle);
        func_overlay_065_F0001A14_18C5C7C(
            spawnX, camera->y + mathRnd(-100, -50), spawnZ);
        gOverlay65TrailSpawnTimer = 1;
    }

    func_800349A4(&commands, NULL, 1, 0);
    record = gOverlay65TrailRecords;
    for (recordIndex = 0; recordIndex < 50; recordIndex++, record++) {
        if (record->active == 0) {
            continue;
        }

        for (updateIndex = 0; updateIndex < updateRate; updateIndex++) {
            record->x[0] += record->velocityX;
            record->y[0] += record->velocityY;
            record->z[0] += record->velocityZ;
            for (pointIndex = 1; pointIndex < 9; pointIndex++) {
                record->x[pointIndex] +=
                    (record->x[pointIndex - 1] - record->x[pointIndex]) *
                    0.25f;
                record->y[pointIndex] +=
                    (record->y[pointIndex - 1] - record->y[pointIndex]) *
                    0.25f;
                record->z[pointIndex] +=
                    (record->z[pointIndex - 1] - record->z[pointIndex]) *
                    0.25f;
            }
            for (pointIndex = 0; pointIndex < 9; pointIndex++) {
                record->x[pointIndex] += D_2978;
                record->z[pointIndex] += D_297C;
            }
            record->velocityY -= gOverlay65TrailGravity;
            if (record->y[0] < record->floorY) {
                record->active = 0;
            }
        }

        if (record->active == 0) {
            continue;
        }
        commands->w0 = 0x040000BCU |
            (((((u32)D_2988 + 0x80000000U) & 6U) | 0x90U) << 16);
        commands->w1 = (u32)D_2988 + 0x80000000U;
        commands++;
        commands->w0 = 0x05F10100U;
        commands->w1 = (u32)D_800000C0;
        commands++;

        for (pointIndex = 0; pointIndex < 9; pointIndex++) {
            D_2988->x = record->x[pointIndex] - 3.0f;
            D_2988->y = record->y[pointIndex];
            D_2988->z = record->z[pointIndex] - 3.0f;
            D_2988->red = record->red;
            D_2988->green = record->green;
            D_2988->blue = record->blue;
            D_2988->alpha = 0xFF;
            D_2988++;
            D_2988->x = record->x[pointIndex] + 3.0f;
            D_2988->y = record->y[pointIndex];
            D_2988->z = record->z[pointIndex] + 3.0f;
            D_2988->red = record->red;
            D_2988->green = record->green;
            D_2988->blue = record->blue;
            D_2988->alpha = 0xFF;
            D_2988++;
        }
    }
    *commandPtr = commands;
    *cursorPtr = cursor;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o065/func_overlay_065_F0000C38_18C4EA0/func_overlay_065_F0000C38_18C4EA0.s")
#endif
