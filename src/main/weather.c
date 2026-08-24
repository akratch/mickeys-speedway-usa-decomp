/*
 * Snow and rain weather system -- ROM 0x3B480-0x3D030.
 *
 * PROVENANCE -- the TU and descriptive function names are borrowed from Jet
 * Force Gemini's and Diddy Kong Racing's public retail-derived src/weather.c
 * files and JFG's nonmatching assembly names.  Evidence is tier B/D except
 * for the existing tier-A weather_clip_planes identity.  Adapted bodies carry
 * point-of-use provenance notes; Mickey's ROM remains authoritative.
 *
 * snow_update and snow_vertices are extractor-marked hand-written routines;
 * snow_vertices also uses odd single-precision FP registers.  Both stay asm.
 */

#include "PR/ultratypes.h"

typedef struct WeatherClipPlanes {
    s16 near;
    s16 far;
} WeatherClipPlanes;

typedef struct WeatherData {
    s32 intensity;
    s32 intensityStep;
    s32 intensityTarget;
    s32 velX;
    s32 velXStep;
    s32 velXTarget;
    s32 velY;
    s32 velYStep;
    s32 velYTarget;
    s32 velZ;
    s32 velZStep;
    s32 velZTarget;
    s32 opacity;
    s32 opacityStep;
    s32 opacityTarget;
    s32 shiftTime;
} WeatherData;

typedef struct WeatherTexture {
    u8 pad0[6];
    s16 width;
    s16 height;
} WeatherTexture;

typedef struct WeatherGfxData {
    void *positions;
    s32 size;
    union {
        s32 type;
        WeatherTexture *texture;
    } source;
    s32 offsetX;
    s32 offsetY;
    s32 offsetZ;
    s32 radiusX;
    s32 radiusY;
    s32 radiusZ;
    s16 vertOffsetW;
    s16 vertOffsetH;
    s16 vertWidth;
    s16 vertHeight;
} WeatherGfxData;

typedef struct WeatherParticle {
    s32 x;
    s32 y;
    s32 z;
    u8 xScale;
    u8 yScale;
    u8 zScale;
    u8 index;
} WeatherParticle;

typedef struct WeatherVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} WeatherVertex;

typedef struct WeatherTexCoord {
    s16 u;
    s16 v;
} WeatherTexCoord;

typedef struct WeatherTriangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    WeatherTexCoord uv0;
    WeatherTexCoord uv1;
    WeatherTexCoord uv2;
} WeatherTriangle;

extern WeatherClipPlanes D_800D40B8;
extern s32 D_8007C6EC;
extern s32 D_8007C6F8;
extern WeatherGfxData D_8007C310[];
extern WeatherParticle *D_8007C394;
extern WeatherGfxData D_8007C398;
extern WeatherTriangle *D_8007C3CC;
extern s16 *D_8007C3D0;
extern WeatherVertex *D_8007C3D4[2];
extern WeatherVertex *D_8007C3DC;
extern s32 D_800D4070;
extern WeatherData D_800D4078;
extern s32 D_800D40C4;
extern s8 D_800D40C8;

extern s32 func_800299E8(s32 min, s32 max);
extern s32 mathRnd(s32 min, s32 max);
extern void *func_8002B280(s32 size, s32 tag);

void freeWeather(void);
void snow_init(void);
void rain_init(s32 count, s32 intensity, s32 opacity, s32 intensityBase);

#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/initWeather.s")
/*
 * PROVENANCE -- body adapted from Diddy Kong Racing's public retail-derived
 * src/weather.c::weather_clip_planes.  Mickey's bytes and global layout are
 * authoritative here.
 */
void weather_clip_planes(s16 near, s16 far) {
    if (D_800D40B8.far < D_800D40B8.near) {
        D_800D40B8.near = near;
        D_800D40B8.far = far;
    } else {
        D_800D40B8.near = far;
        D_800D40B8.far = near;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/freeWeather.s")
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::setupWeather.  Mickey's bytes, extra rain-init argument,
 * random bounds, and globals are authoritative here.
 */
void setupWeather(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    s16 temp_s1_2;
    s16 temp_s2;
    s32 temp_s1;
    u8 var_a1;
    s32 i;
    WeatherParticle *var_s1_2;
    WeatherTriangle *var_v1_2;
    WeatherVertex *var_a3;
    s32 j;
    s8 *var_a0;
    u8 *var_v1;
    s32 pad;
    s32 numOfElements;

    freeWeather();
    D_800D4078.velX = arg2;
    D_800D4078.velXStep = 0;
    D_800D4078.velXTarget = arg2;
    D_800D4078.velY = arg3;
    D_800D4078.velYStep = 0;
    D_800D4078.velYTarget = arg3;
    D_800D4078.velZStep = 0;
    D_800D4078.intensityStep = 0;
    D_800D4078.opacityStep = 0;
    D_800D4078.shiftTime = 0;
    D_800D4078.velZ = arg4;
    D_800D4078.velZTarget = arg4;
    D_800D4078.intensity = arg5;
    D_800D4078.intensityTarget = arg5;
    D_800D4078.opacity = arg6;
    D_800D4078.opacityTarget = arg6;
    if (arg0 > 1) {
        arg0 = 1;
    }
    if (D_8007C310[arg0].source.type == 1) {
        rain_init(arg1, arg5 + 1, arg6 + 1, arg5);
        return;
    }
    var_v1 = (u8 *) &D_8007C310[arg0];
    var_a0 = (s8 *) &D_8007C398;
    var_a1 = 0x2C;
    while (var_a1--) {
        *var_a0++ = *var_v1++;
    }
    if (!var_s1_2) {
        ;
    }
    D_8007C398.positions = func_8002B280(D_8007C310[arg0].size * 0xC, 0x93);
    if (D_8007C310[arg0].source.type == 0) {
        snow_init();
    }
    numOfElements = arg1;
    D_800D4070 = arg1;
    D_8007C3D0 = func_8002B280(arg1 * sizeof(s16), 0x93);
    D_8007C394 = func_8002B280(arg1 * sizeof(WeatherParticle), 0x93);
    var_s1_2 = D_8007C394;
    for (i = 0; i < D_800D4070; i++) {
        var_s1_2->x = func_800299E8(0, D_8007C398.radiusX);
        var_s1_2->y = func_800299E8(0, D_8007C398.radiusY);
        var_s1_2->z = func_800299E8(0, D_8007C398.radiusZ);
        var_s1_2->xScale = 1 << (func_800299E8(0, 0x1F) + 5);
        var_s1_2->yScale = 1 << (func_800299E8(0, 0x1F) + 5);
        var_s1_2->zScale = 1 << (func_800299E8(0, 0x1F) + 5);
        var_s1_2->index = mathRnd(0, D_8007C398.size - 1);
        var_s1_2++;
    }
    numOfElements *= 4;
    temp_s1 = sizeof(WeatherVertex);
    temp_s1 *= numOfElements;
    D_8007C3D4[0] = func_8002B280(temp_s1, 0x93);
    D_8007C3D4[1] = func_8002B280(temp_s1, 0x93);
    j = 0;
    do {
        var_a3 = D_8007C3D4[j];
        for (i = 0; i < numOfElements; i++, var_a3++) {
            var_a3->r = 0xFF;
            var_a3->g = 0xFF;
            var_a3->b = 0xFF;
            var_a3->a = 0xFF;
        }
        j++;
    } while (&D_8007C3D4[j] < &D_8007C3DC);
    temp_s1_2 = (D_8007C398.source.texture->width << 5) - 1;
    temp_s2 = (D_8007C398.source.texture->height << 5) - 1;
    D_8007C3CC = func_8002B280(D_800D40C4 * sizeof(WeatherTriangle), 0x93);
    var_v1_2 = D_8007C3CC;
    for (i = 0; i < D_800D40C4; i += 2) {
        var_v1_2[0].flags = 0;
        var_v1_2[0].vi0 = (i << 1) + 3;
        var_v1_2[0].uv0.u = 0;
        var_v1_2[0].uv0.v = temp_s2;
        var_v1_2[0].vi1 = (i << 1) + 1;
        var_v1_2[0].uv1.u = temp_s1_2;
        var_v1_2[0].uv1.v = 0;
        var_v1_2[0].vi2 = i << 1;
        var_v1_2[0].uv2.u = 0;
        var_v1_2[0].uv2.v = 0;
        var_v1_2[1].flags = 0;
        var_v1_2[1].vi0 = (i << 1) + 3;
        var_v1_2[1].uv0.u = 0;
        var_v1_2[1].uv0.v = temp_s2;
        var_v1_2[1].vi1 = (i << 1) + 2;
        var_v1_2[1].uv1.u = temp_s1_2;
        var_v1_2[1].uv1.v = temp_s2;
        var_v1_2[1].vi2 = (i << 1) + 1;
        var_v1_2[1].uv2.u = temp_s1_2;
        var_v1_2[1].uv2.v = 0;
        var_v1_2 += 2;
    }
    D_800D40C8 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/snow_init.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/changeWeather.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/doWeather.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/snow_render.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_init.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/free_rain_memory.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_set.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rainSetFog.s")
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::rainDensity.  Mickey's bytes and globals are authoritative.
 */
f32 rainDensity(void) {
    f32 density;

    density = (f32)(((D_8007C6F8 >> 2) * D_8007C6EC) >> 14) / 0x10000;
    if (density < 0.0f) {
        density = 0.0f;
    }
    if (density > 1.0f) {
        density = 1.0f;
    }
    return density;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_update.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_render_splashes.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_lightning.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_sound.s")
