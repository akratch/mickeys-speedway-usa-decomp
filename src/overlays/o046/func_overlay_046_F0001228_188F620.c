#include "PR/ultratypes.h"

typedef struct Overlay46DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay46DisplayCommand;

typedef struct Overlay46Emitter {
    f32 x;
    f32 y;
    f32 z;
    f32 speed;
    s16 angle;
    u8 pad12[2];
    s32 vertexCount;
} Overlay46Emitter;

typedef struct Overlay46Spark {
    f32 x;
    f32 y;
    f32 z;
    f32 velocityX;
    f32 velocityY;
    s16 age;
    s16 alpha;
} Overlay46Spark;

typedef struct Overlay46TrailVertex {
    s16 x;
    s16 y;
    s16 z;
} Overlay46TrailVertex;

typedef struct Overlay46DrawPoint {
    u8 pad00[0x1C];
    s16 value0;
    s16 value2;
    s16 value4;
    s16 pad6;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} Overlay46DrawPoint;

extern s16 D_194;
extern s16 D_19C;
extern volatile s16 D_494;
/* These phase-local names resolve to the same overlay addresses as their
 * unsuffixed globals. The separate lifetimes preserve more of the original
 * translation unit's register allocation; promotion would rebind them. */
extern s16 D_494Reload;
extern Overlay46TrailVertex D_498[];
extern Overlay46TrailVertex D_498Spawn[];
extern Overlay46Emitter *D_1450;
extern Overlay46Emitter *D_1450Spawn;
extern Overlay46Spark D_1458[];
extern Overlay46Spark D_1458Spawn[];
extern s16 D_2718;
extern u8 D_80000230[];

extern Overlay46DisplayCommand *D_800D3140;
extern void *D_800D3144;
extern void *D_800D3148;
extern void *D_800D31C8[];

extern void amSndPlay(s32 soundId, void *handle);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void func_800221E8(Overlay46DisplayCommand **commands, void **matrices);
extern void func_80023A08(Overlay46DisplayCommand **commands, void *matrices,
                          void *vertices, void *point,
                          void *object, s32 flags, s32 alpha);
extern void func_800349A4(Overlay46DisplayCommand **commands, void *texture,
                          s32 flags, s32 parameter);

#define O46_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

#define O46_PRIM(packet, red, green, blue, alpha) { \
    Overlay46DisplayCommand *macroCommand = (packet); \
    macroCommand->w0 = O46_SHIFTL(0xFA, 24, 8); \
    macroCommand->w1 = O46_SHIFTL(red, 24, 8) | \
        O46_SHIFTL(green, 16, 8) | O46_SHIFTL(blue, 8, 8) | \
        O46_SHIFTL(alpha, 0, 8); \
}

/* Workbench p5: structure-mismatch; 437/461 candidate/target instructions, 415 words from +0x5C.
 * Lever: constant-audit plus direct-global, alias, and indexed-spark probes; each regressed the emitter shape.
 * Remains: 24 missing instructions, relocation differences, and the inner emitter CFG/register cascade. */
#ifdef NON_MATCHING
void func_overlay_046_F0001228_188F620(s32 updateRate) {
    s32 i;
    s32 batchCount;
    s32 batchSize;
    s32 remaining;
    s32 vertexCount;
    s16 timer;
    f32 halfX;
    f32 halfY;
    f32 step;
    volatile f32 zero;
    Overlay46DisplayCommand *command;
    Overlay46Emitter *emitter;
    Overlay46Spark *spark;
    Overlay46TrailVertex *vertex;
    Overlay46DrawPoint point;

    if (D_19C < (timer = D_494)) {
        D_494 = timer - updateRate;
        timer = D_494;
        if (((timer + updateRate) > 0) && (timer <= 0)) {
            amSndPlay(0x11, NULL);
            timer = D_494Reload;
        }
    }

    if (timer <= 0) {
        if (D_19C < timer) {
            D_2718 = 0xFF;
            if (updateRate > 0) {
                i = 0;
                do {
                    emitter = D_1450Spawn;
                    vertexCount = emitter->vertexCount;
                    if (vertexCount < 0x190) {
                        vertex = (Overlay46TrailVertex *)
                            ((u8 *)D_498Spawn + (vertexCount * 10));
                        emitter->x += emitter->speed *
                            func_8002A8C0(emitter->angle);
                        emitter = D_1450Spawn;
                        emitter->y += emitter->speed *
                            func_8002A8BC(emitter->angle);
                        emitter = D_1450Spawn;
                        halfX = func_8002A8C0(emitter->angle + 0x4000) * 5.0f;
                        emitter = D_1450Spawn;
                        halfY = func_8002A8BC(emitter->angle + 0x4000) * 5.0f;
                        emitter = D_1450Spawn;
                        vertex[0].x = (s32)(emitter->x - halfX);
                        vertex[0].y = (s32)(emitter->y - halfY);
                        vertex[0].z = (s32)emitter->z;
                        vertex = (Overlay46TrailVertex *)((u8 *)vertex + 10);
                        emitter = D_1450Spawn;
                        vertex[0].x = (s32)(emitter->x + halfX);
                        vertex[0].y = (s32)(emitter->y + halfY);
                        vertex[0].z = (s32)emitter->z;
                        emitter = D_1450Spawn;
                        emitter->angle -= D_194;
                        emitter = D_1450Spawn;
                        emitter->vertexCount += 2;
                    }

                    spark = D_1458Spawn;
                    for (batchCount = 0; batchCount < 0xC8;
                         batchCount++, spark++) {
                        if (spark->age < 0) {
                            emitter = D_1450Spawn;
                            spark->age = 0;
                            spark->x = emitter->x;
                            spark->y = emitter->y;
                            spark->alpha = 0xFF;
                            spark->z = emitter->z;
                            spark->velocityX =
                                (f32)mathRnd(-0x1F4, 0x1F4) / 500.0f;
                            spark->velocityY =
                                (f32)mathRnd(-0x1F4, 0x1F4) / 500.0f;
                            break;
                        }
                    }
                    i++;
                } while (i != updateRate);
            }
        } else {
            D_2718 -= updateRate * 8;
            if (D_2718 < 0) {
                D_2718 = 0;
            }
        }

        func_800221E8(&D_800D3140, &D_800D3144);
        point.value4 = 0;
        point.value0 = 0;
        point.value2 = 0;
        point.scale = 6.0f;
        zero = 0.0f;

        spark = D_1458;
        do {
            if (spark->age != -1) {
                point.x = spark->x;
                point.y = spark->y;
                point.z = spark->z;
                command = D_800D3140++;
                O46_PRIM(command, 0xA0, 0xA0, 0xA0, spark->alpha);
                func_80023A08(&D_800D3140, D_800D3144, D_800D3148,
                              &point.value0, D_800D31C8[9], 7, spark->alpha);
            }
            spark++;
        } while (spark < (Overlay46Spark *)&D_2718);

        func_800349A4(&D_800D3140, NULL, 3, 0);
        command = D_800D3140++;
        O46_PRIM(command, 0xC0, 0xC0, 0xC0, 0xFF);

        emitter = D_1450;
        vertex = D_498;
        vertexCount = emitter->vertexCount - 2;
        batchCount = vertexCount / 16;
        remaining = batchCount * 16;
        i = 0;
        if (batchCount > 0) {
            do {
                batchSize = remaining >= 16 ? 16 : remaining;
                command = D_800D3140++;
                command->w0 = 0x04000000 |
                    (O46_SHIFTL((batchSize + 2) * 8 |
                                (((u32)vertex + 0x80000000) & 6), 16, 8)) |
                    (((batchSize + 2) * 10) + 8);
                command->w1 = (u32)vertex + 0x80000000;
                command = D_800D3140++;
                command->w0 = 0x05000000 |
                    (O46_SHIFTL(((batchSize - 1) * 16) | 1, 16, 8)) |
                    (batchSize * 16);
                command->w1 = (u32)D_80000230;
                i++;
                remaining -= batchSize;
                vertex = (Overlay46TrailVertex *)
                    ((u8 *)vertex + (batchSize * 10));
            } while (i < batchCount);
        }

        batchSize = vertexCount % 16;
        if (batchSize != 0) {
            command = D_800D3140++;
            command->w0 = 0x04000000 |
                (O46_SHIFTL((batchSize + 2) * 8 |
                            (((u32)vertex + 0x80000000) & 6), 16, 8)) |
                (((batchSize + 2) * 10) + 8);
            command->w1 = (u32)vertex + 0x80000000;
            command = D_800D3140++;
            command->w0 = 0x05000000 |
                (O46_SHIFTL(((batchSize - 1) * 16) | 1, 16, 8)) |
                (batchSize * 16);
            command->w1 = (u32)D_80000230;
        }

        if (D_2718 > 0) {
            emitter = D_1450;
            point.x = emitter->x;
            point.y = emitter->y;
            point.scale = 6.0f;
            point.z = emitter->z;
            command = D_800D3140++;
            O46_PRIM(command, 0xA0, 0xA0, 0xA0, D_2718);
            func_80023A08(&D_800D3140, D_800D3144, D_800D3148,
                          &point.value0, D_800D31C8[10], 7, D_2718);
        }

        spark = D_1458;
        do {
            if (spark->age != -1) {
                step = (f32)updateRate;
                spark->age += updateRate;
                spark->x += spark->velocityX * step;
                spark->y += spark->velocityY * step;
                if (spark->age >= 0x3D) {
                    spark->alpha -= updateRate * 8;
                    if (spark->alpha < 0) {
                        spark->age = -1;
                    }
                }
            }
            spark++;
        } while (spark != (Overlay46Spark *)&D_2718);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o046/func_overlay_046_F0001228_188F620/func_overlay_046_F0001228_188F620.s")
#endif
