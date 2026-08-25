#ifndef GAME_LIGHTS_H
#define GAME_LIGHTS_H

#include "PR/ultratypes.h"

/* PROVENANCE: adapted from JFG's public decomp, include/structs.h. */
typedef struct UnkLight {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    s16 home;
    s16 index;
    f32 directionX;
    f32 directionY;
    f32 directionZ;
    void *owner;
    f32 x;
    f32 y;
    f32 z;
    f32 radius;
    f32 radius2;
    f32 radius3;
    f32 radiusSquare;
    f32 radiusInverse;
    u8 pad38[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 unk43;
    f32 unk44;
    u8 colourCycle[0xC];
    s32 unk54;
    s16 value58;
    s16 value5A;
    s16 value5C;
    s16 value5E;
    u8 pad60[0xC];
    s32 unk6C;
} UnkLight;

/* PROVENANCE: adapted from JFG's public decomp, src/lights.c comparison. */
typedef struct RomdefLight {
    u8 pad0[4];
    s16 x;
    s16 y;
    s16 z;
    u8 modeAndType;
    u8 flags;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    s16 radius;
    s16 radius2;
    u8 pad14[2];
    s16 value58;
    s16 value5A;
    s16 value5C;
    s16 value5E;
    u8 colourCycleIndex;
    u8 enabledFlags;
} RomdefLight;

/* PROVENANCE: adapted from JFG's public decomp, src/lights.c comparison. */
typedef struct ObjectLightEntry {
    u8 mode;
    u8 type;
    u8 flags;
    s8 index;
    u16 home;
    s16 x;
    s16 y;
    s16 z;
    u16 radius;
    u16 radius2;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    s16 colourCycleIndex;
    u8 value58;
    u8 value5A;
} ObjectLightEntry;

typedef struct LightSourceObject LightSourceObject;
typedef struct FlareObject FlareObject;

void freeLights(void);
void setupLights(s32 count, s32 arg1, s32 arg2);
void turnLightOff(UnkLight *light);
void turnLightOn(UnkLight *light);
void toggleLight(UnkLight *light);
void changeLightColour(UnkLight *light, u8 red, u8 green, u8 blue);
void changeLightColourCycle(s32 arg0, s32 arg1);
void changeLightIntensity(UnkLight *light, u8 intensity);
void lightUpdateLights(s32 updateRate);
void killLight(UnkLight *light);
void **lightGetLights(s32 *count);
UnkLight *lightGetStrongestEffect(f32 x, f32 y, f32 z);
void lightUpdateObjects(void);
void lightSetupLightSources(LightSourceObject *object);
void lightSetupFlareSources(FlareObject *object);
void lightDefaultObjectLight(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s32 arg4);
s32 lightKillGlowingLight(void);
UnkLight *addRomdefLight(s32 arg0, RomdefLight *entry);
UnkLight *addObjectLight(s32 owner, ObjectLightEntry *entry);
f32 lightDirectionCalc(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6);

#endif
