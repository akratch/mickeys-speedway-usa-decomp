#ifndef GAME_LIGHTS_H
#define GAME_LIGHTS_H

#include "PR/ultratypes.h"

/* PROVENANCE: adapted from JFG's public decomp, include/structs.h. */
typedef struct UnkLight {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 pad4[0x3C];
    u8 red;
    u8 green;
    u8 blue;
    u8 unk43;
    f32 unk44;
    u8 pad48[0xC];
    s32 unk54;
} UnkLight;

void turnLightOff(UnkLight *light);
void turnLightOn(UnkLight *light);
void toggleLight(UnkLight *light);
void changeLightColour(UnkLight *light, u8 red, u8 green, u8 blue);
void changeLightColourCycle(s32 arg0, s32 arg1);

#endif
