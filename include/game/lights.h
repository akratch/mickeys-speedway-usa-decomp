#ifndef GAME_LIGHTS_H
#define GAME_LIGHTS_H

#include "PR/ultratypes.h"

/* PROVENANCE: adapted from JFG's public decomp, include/structs.h. */
typedef struct UnkLight {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} UnkLight;

void turnLightOff(UnkLight *light);
void turnLightOn(UnkLight *light);
void toggleLight(UnkLight *light);
void changeLightColourCycle(s32 arg0, s32 arg1);

#endif
