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

#define O65_RECORD(base) ((Overlay65TrailRecord *)(base))

extern u8 D_0[];
extern s16 D_1900;
extern f32 D_214;
extern f32 D_2978;
extern f32 D_297C;
extern Overlay65TrailVertex *D_2988;
extern u8 D_800000C0[];

extern Overlay65TrailCamera *func_80021970(s32 index);
extern void camSetNo(s32 index);
extern void func_800221E8(void **commands, s32 *cursor);
extern s32 mathRnd(s32 lower, s32 upper);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern void func_800349A4(void **commands, void *texture,
                          s32 flags, s32 parameter);
extern void func_overlay_065_F0001A14_18C5C7C(f32 x, f32 y, f32 z);

/*
 * Workbench: structure-mismatch (mixed), 311/887 positional words differ; target/candidate are 887 instructions, first +0x0, frame deficit 8 bytes.
 * Levers: MIPS-II flags, target do-while/D_0 record base, float conversion, and typed-pointer probes; target record pool versus candidate temp remains.
 * Remaining: 157 register / 92 structural residuals and target unrolled-writer/stack-frame allocation drift; no exact C codegen.
 */
#ifdef NON_MATCHING
void func_overlay_065_F0000C38_18C4EA0(void **commandPtr,
                                       s32 *cursorPtr, s32 updateRate) {
    f32 randomX;
    f32 randomZ;
    f32 sinAngle;
    f32 cosAngle;
    f32 spawnX;
    void *commands;
    f32 spawnZ;
    s32 cursor;
    s32 recordIndex;
    s32 updateIndex;
    s32 pointIndex;
    Overlay65TrailCamera *camera;
    Overlay65TrailRecord *record;

    commands = *commandPtr;
    cursor = *cursorPtr;
    camera = func_80021970(0);
    camSetNo(0);
    func_800221E8(&commands, &cursor);

    if (D_1900 > 0) {
        D_1900 -= updateRate;
    } else {
        randomX = (f32)mathRnd(-500, 500);
        randomZ = (f32)mathRnd(-500, 500);
        sinAngle = func_8002A8BC(camera->angle);
        cosAngle = func_8002A8C0(camera->angle);
        spawnX = (camera->x + (randomX * sinAngle)) -
                 (randomZ * cosAngle);
        spawnZ = camera->z + (randomZ * sinAngle) +
                 (randomX * cosAngle);
        func_overlay_065_F0001A14_18C5C7C(
            spawnX, camera->y + mathRnd(-100, -50), spawnZ);
        D_1900 = 1;
    }

    func_800349A4(&commands, NULL, 1, 0);
    record = O65_RECORD(D_0);
    recordIndex = 0;
    do {
        if (O65_RECORD(record)->active != 0) {
            for (updateIndex = 0; updateIndex < updateRate; updateIndex++) {
                O65_RECORD(record)->x[0] += O65_RECORD(record)->velocityX;
                O65_RECORD(record)->y[0] += O65_RECORD(record)->velocityY;
                O65_RECORD(record)->z[0] += O65_RECORD(record)->velocityZ;
                for (pointIndex = 1; pointIndex < 9; pointIndex++) {
                    O65_RECORD(record)->x[pointIndex] +=
                        (O65_RECORD(record)->x[pointIndex - 1] -
                         O65_RECORD(record)->x[pointIndex]) *
                        0.25f;
                    O65_RECORD(record)->y[pointIndex] +=
                        (O65_RECORD(record)->y[pointIndex - 1] -
                         O65_RECORD(record)->y[pointIndex]) *
                        0.25f;
                    O65_RECORD(record)->z[pointIndex] +=
                        (O65_RECORD(record)->z[pointIndex - 1] -
                         O65_RECORD(record)->z[pointIndex]) *
                        0.25f;
                }
                for (pointIndex = 0; pointIndex < 9; pointIndex++) {
                    O65_RECORD(record)->x[pointIndex] += D_2978;
                    O65_RECORD(record)->z[pointIndex] += D_297C;
                }
                O65_RECORD(record)->velocityY -= D_214;
                if (O65_RECORD(record)->y[0] < O65_RECORD(record)->floorY) {
                    O65_RECORD(record)->active = 0;
                }
            }

            if (O65_RECORD(record)->active != 0) {
                ((u32 *)commands)[0] = 0x040000BCU |
                    (((((u32)D_2988 + 0x80000000U) & 6U) | 0x90U) << 16);
                ((u32 *)commands)[1] = (u32)D_2988 + 0x80000000U;
                commands = (u8 *)commands + 8;
                ((u32 *)commands)[0] = 0x05F10100U;
                ((u32 *)commands)[1] = (u32)D_800000C0;
                commands = (u8 *)commands + 8;

                for (pointIndex = 0; pointIndex < 9; pointIndex++) {
                    D_2988->x = O65_RECORD(record)->x[pointIndex] - 3.0f;
                    D_2988->y = O65_RECORD(record)->y[pointIndex];
                    D_2988->z = O65_RECORD(record)->z[pointIndex] - 3.0f;
                    D_2988->red = O65_RECORD(record)->red;
                    D_2988->green = O65_RECORD(record)->green;
                    D_2988->blue = O65_RECORD(record)->blue;
                    D_2988->alpha = 0xFF;
                    D_2988++;
                    D_2988->x = O65_RECORD(record)->x[pointIndex] + 3.0f;
                    D_2988->y = O65_RECORD(record)->y[pointIndex];
                    D_2988->z = O65_RECORD(record)->z[pointIndex] + 3.0f;
                    D_2988->red = O65_RECORD(record)->red;
                    D_2988->green = O65_RECORD(record)->green;
                    D_2988->blue = O65_RECORD(record)->blue;
                    D_2988->alpha = 0xFF;
                    D_2988++;
                }
            }
        }
        recordIndex++;
        record++;
    } while (recordIndex != 50);
    *commandPtr = commands;
    *cursorPtr = cursor;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o065/func_overlay_065_F0000C38_18C4EA0/func_overlay_065_F0000C38_18C4EA0.s")
#endif
