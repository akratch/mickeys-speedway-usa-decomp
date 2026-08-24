/*
 * Snow and rain weather system -- ROM 0x3B480-0x3D030.
 *
 * PROVENANCE -- the TU and descriptive function names are borrowed from Jet
 * Force Gemini's and Diddy Kong Racing's public retail-derived src/weather.c
 * files and JFG's nonmatching assembly names.  Evidence is tier B/D except
 * for the existing tier-A weather_clip_planes identity.  No reference body
 * is adapted by this scaffold; Mickey's ROM remains authoritative.
 *
 * snow_update and snow_vertices are extractor-marked hand-written routines;
 * snow_vertices also uses odd single-precision FP registers.  Both stay asm.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/initWeather.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/weather_clip_planes.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rainDensity.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_update.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_render_splashes.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_lightning.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/weather/rain_sound.s")
