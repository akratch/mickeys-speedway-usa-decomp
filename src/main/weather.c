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

typedef struct Gfx Gfx;
typedef struct Mtx Mtx;
typedef struct Camera Camera;
typedef struct Matrix Matrix;

typedef struct WeatherClipPlanes {
    s16 near;
    s16 far;
    s32 current;
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

typedef struct WeatherPosition {
    s32 x;
    s32 y;
    s32 z;
} WeatherPosition;

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
extern s32 *D_8007C3DC;
extern s8 D_8007C3E0;
extern s32 D_800D4070;
extern s32 D_800D4074;
extern WeatherData D_800D4078;
extern s32 D_800D40C0;
extern s32 D_800D40C4;
extern s8 D_800D40C8;
extern Gfx *D_800D40CC;
extern Mtx *D_800D40D0;
extern WeatherVertex *D_800D40D4;
extern WeatherTriangle *D_800D40D8;
extern Camera *D_800D40DC;
extern Matrix *D_800D40E0;
extern WeatherVertex *D_8007C3C4;
extern s32 D_8007C3C8;
extern s32 D_8007C6E8;
extern s32 D_8007C6EC;
extern s32 D_8007C6F8;
extern s32 D_8007C708;
extern s32 D_8007C70C;

extern s32 func_800299E8(s32 min, s32 max);
extern s32 mathRnd(s32 min, s32 max);
extern void *func_8002B280(s32 size, s32 tag);
extern Camera *func_8002462C(void);
extern Matrix *func_80024698(void);
extern s32 *func_8002E148(s32 assetId);
extern s32 coss_s16(s16 angle);
extern s32 func_8002A1A4(s16 angle);
extern WeatherTexture *func_80034448(s32 textureId);
extern s32 func_80049864(s32 mode);
extern void func_800498FC(s32 mode, f32 arg1, f32 arg2, s32 red, s32 green, s32 blue, s32 alpha);
extern void mmFree(void *ptr);
extern void func_800347A0(WeatherTexture *texture);

void freeWeather(void);
void snow_init(void);
void rain_init(s32 count, s32 intensity, s32 opacity, s32 intensityBase);
void free_rain_memory(void);
void rain_update(s32 updateRate);
void rain_set(s32 intensity, s32 opacity, f32 seconds);
void snow_update(WeatherData *weather, WeatherGfxData *gfx, s32 particleCount, WeatherParticle *particles,
                 s32 updateRate);
s32 snow_vertices(Camera *camera, WeatherGfxData *gfx, s32 particleCount, WeatherParticle *particles,
                  Matrix *cameraMatrix, WeatherVertex *vertices);
void snow_render(void);

/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::initWeather. Mickey's globals and asset ID are authoritative.
 */
void initWeather(void) {
    s32 *temp_v0;

    D_8007C398.positions = NULL;
    D_8007C398.size = 0;
    D_8007C394 = NULL;
    D_800D4070 = 0;
    D_800D40C0 = 6;
    D_800D40C0 <<= 2;
    D_800D40C4 = D_800D40C0 >> 1;
    D_8007C3D4[0] = NULL;
    D_8007C3D4[1] = NULL;
    D_8007C3CC = NULL;
    D_800D40B8.near = -1;
    D_800D40B8.far = -0x200;
    if (D_8007C3DC == NULL) {
        temp_v0 = func_8002E148(0x1B);
        D_8007C3E0 = 0;
        D_8007C3DC = temp_v0;
        while (D_8007C3DC[D_8007C3E0] != -1) {
            D_8007C3E0++;
        }
    }
    D_800D40C8 = 0;
}
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
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::freeWeather. Mickey's globals and release order are
 * authoritative here.
 */
void freeWeather(void) {
    if (D_8007C3CC != NULL) {
        mmFree(D_8007C3CC);
        D_8007C3CC = NULL;
    }
    if (D_8007C3D4[0] != NULL) {
        mmFree(D_8007C3D4[0]);
        D_8007C3D4[0] = NULL;
    }
    if (D_8007C3D4[1] != NULL) {
        mmFree(D_8007C3D4[1]);
        D_8007C3D4[1] = NULL;
    }
    if (D_8007C394 != NULL) {
        mmFree(D_8007C394);
        D_8007C394 = NULL;
    }
    if (D_8007C398.positions != NULL) {
        mmFree(D_8007C398.positions);
        D_8007C398.positions = NULL;
    }
    if (D_8007C398.source.texture != NULL) {
        func_800347A0(D_8007C398.source.texture);
        D_8007C398.source.texture = NULL;
    }
    if (D_8007C3D0 != NULL) {
        mmFree(D_8007C3D0);
        D_8007C3D0 = NULL;
    }
    if (D_8007C6E8 != 0) {
        free_rain_memory();
    }
}
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
    } while (&D_8007C3D4[j] < (WeatherVertex **) &D_8007C3DC);
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
/*
 * PROVENANCE -- body adapted from Diddy Kong Racing's public retail-derived
 * src/weather.c::snow_init. Mickey's scale constants and texture loader are
 * authoritative here.
 */
void snow_init(void) {
    s32 step;
    s32 offset;
    s32 i;

    step = 0x10000 / D_8007C398.size;
    offset = 0;
    for (i = 0; i < D_8007C398.size; i++) {
        ((WeatherPosition *) D_8007C398.positions)[i].x = coss_s16(offset & 0xFFFF) * 4;
        ((WeatherPosition *) D_8007C398.positions)[i].y = 0xFFFE0000;
        ((WeatherPosition *) D_8007C398.positions)[i].z = func_8002A1A4(offset & 0xFFFF);
        offset += step;
    }
    D_8007C398.source.texture = func_80034448(*D_8007C3DC);
}
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::changeWeather. Mickey's condition and assignment ordering
 * are authoritative here.
 */
void changeWeather(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    if ((arg5 > 0) &&
        ((arg0 != D_800D4078.velXTarget) || (arg1 != D_800D4078.velYTarget) ||
         (arg2 != D_800D4078.velZTarget) || (arg3 != D_800D4078.intensity) ||
         (arg4 != D_800D4078.opacity))) {
        D_800D4078.velXStep = (s32) ((arg0 - D_800D4078.velX) / arg5);
        D_800D4078.velXTarget = arg0;
        D_800D4078.velYStep = (s32) ((arg1 - D_800D4078.velY) / arg5);
        D_800D4078.velYTarget = arg1;
        D_800D4078.velZStep = (s32) ((arg2 - D_800D4078.velZ) / arg5);
        D_800D4078.velZTarget = arg2;
        if (D_8007C6E8 == 0) {
            D_800D4078.intensityTarget = arg3;
            D_800D4078.intensityStep = (s32) ((arg3 - D_800D4078.intensity) / arg5);
            D_800D4078.opacityStep = (s32) ((arg4 - D_800D4078.opacity) / arg5);
            D_800D4078.opacityTarget = arg4;
            D_800D4078.shiftTime = arg5;
            return;
        }
        D_800D4078.intensity = arg3;
        D_800D4078.opacity = arg4;
        D_800D4078.shiftTime = 0;
        rain_set(arg3 + 1, arg4 + 1, (f32) arg5 / 60.0f);
    }
}
#ifdef NON_MATCHING
/*
 * PROVENANCE -- body adapted from Jet Force Gemini's public retail-derived
 * src/weather.c::doWeather. Mickey's split vertex/render calls and globals
 * are authoritative here.
 */
void doWeather(Gfx **arg0, Mtx **arg1, WeatherVertex **arg2, WeatherTriangle **arg3, s32 updateRate) {
    WeatherVertex *temp_t3;

    D_800D40CC = *arg0;
    D_800D40D0 = *arg1;
    D_800D40D4 = *arg2;
    D_800D40D8 = *arg3;
    D_800D40DC = func_8002462C();
    D_800D40E0 = func_80024698();
    if (D_8007C6E8 != 0) {
        rain_update(updateRate);
    } else {
        if (D_800D4078.shiftTime > 0) {
            if (updateRate < D_800D4078.shiftTime) {
                D_800D4078.intensity =
                    (s32) (D_800D4078.intensity + (D_800D4078.intensityStep * updateRate));
                D_800D4078.velX = (s32) (D_800D4078.velX + (D_800D4078.velXStep * updateRate));
                D_800D4078.velY = (s32) (D_800D4078.velY + (D_800D4078.velYStep * updateRate));
                D_800D4078.shiftTime = (s32) (D_800D4078.shiftTime - updateRate);
                D_800D4078.velZ = (s32) (D_800D4078.velZ + (D_800D4078.velZStep * updateRate));
                D_800D4078.opacity =
                    (s32) (D_800D4078.opacity + (D_800D4078.opacityStep * updateRate));
            } else {
                D_800D4078.shiftTime = 0;
                D_800D4078.intensity = D_800D4078.intensityTarget;
                D_800D4078.velX = D_800D4078.velXTarget;
                D_800D4078.velY = D_800D4078.velYTarget;
                D_800D4078.velZ = D_800D4078.velZTarget;
                D_800D4078.opacity = D_800D4078.opacityTarget;
            }
        }
        D_800D4074 = (D_800D4070 * D_800D4078.intensity) >> 16;
        D_800D40B8.current =
            (D_800D40B8.near + ((D_800D40B8.far - D_800D40B8.near) * D_800D4078.opacity)) >> 16;
        snow_update(&D_800D4078, &D_8007C398, D_800D4070, D_8007C394, updateRate);
        if (D_800D4074 > 0 && D_800D40B8.current < D_800D40B8.near) {
            temp_t3 = D_8007C3D4[D_800D40C8];
            D_8007C3C4 = temp_t3;
            D_8007C3C8 = snow_vertices(D_800D40DC, &D_8007C398, D_800D4074, D_8007C394,
                                       D_800D40E0, temp_t3);
            snow_render();
            D_800D40C8 = 1 - D_800D40C8;
        }
    }
    *arg0 = D_800D40CC;
    *arg1 = D_800D40D0;
    *arg2 = D_800D40D4;
    *arg3 = D_800D40D8;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/doWeather.s")
#endif
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
/*
 * PROVENANCE -- body adapted from Diddy Kong Racing's and Jet Force Gemini's
 * public retail-derived src/weather.c::rain_lightning. Mickey's thresholds,
 * transition call, and timer arithmetic are authoritative here.
 */
void rain_lightning(s32 updateRate) {
    s32 delay;

    if (D_8007C70C > 0) {
        D_8007C70C -= updateRate;
        if (D_8007C70C <= 0) {
            if (D_8007C6F8 > 0x8000) {
                if (func_80049864(4) == 0) {
                    func_800498FC(4, 0.0834f, 0.0334f, 0xFF, 0xFF, 0xFF, 0x40);
                }
            }
            D_8007C70C = 0;
        }
    } else if (D_8007C6EC > 0xC000) {
        if (D_8007C708 > 0) {
            D_8007C708 -= updateRate;
        } else {
            delay = (s32) ((D_8007C6EC * 0x258) + 0xFE3E0000) >> 14;
            D_8007C70C = delay + 0x3C;
            D_8007C708 = mathRnd(0x4B0, 0x5DC) - delay;
        }
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_sound.s")
