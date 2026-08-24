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

extern WeatherClipPlanes D_800D40B8;
extern s32 D_8007C6EC;
extern s32 D_8007C6F8;

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
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/setupWeather.s")
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
