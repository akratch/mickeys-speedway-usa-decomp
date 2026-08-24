/*
 * Resident lights -- ROM 0x19310-0x1AE60 (VRAM 0x80018710).
 *
 * The boundary and function comparison map are documented in
 * docs/modules.md section 3.4. The preceding four-function shadows TU uses
 * odd single-precision FP registers and remains assembly by section 6.2;
 * none of the functions in this TU uses an odd FP register.
 *
 * PROVENANCE -- comparison names, declarations and future starting bodies
 * for this TU come from Jet Force Gemini's public decomp, src/lights.c and
 * src/lights.h. They are permitted published-decomp material under
 * docs/CLEANROOM.md. Mickey's own bytes decide every body and name adoption;
 * the unresolved functions below therefore retain their Mickey func_ names.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul (measured on the FP helpers).
 */

#include "PR/ultratypes.h"
#include "game/lights.h"

extern void initColourCycle();
extern f32 sqrtf(f32 value);
extern void mmFree(void *ptr);
extern void func_8000D728(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern s32 func_8000D62C(f32 x, f32 y, f32 z, f32 radius, f32 radius2, s32 red, s32 green, s32 blue);
extern void func_800188CC(UnkLight *light);
extern void func_80018F08(UnkLight *light, s32 updateRate);
extern u8 *func_8002679C(void);
extern s32 D_80079490;
extern s32 D_80079494;
extern void **D_80079498;
extern void *D_8007949C;
extern void *D_800794A0;
extern f32 D_800817B0;
extern void *D_800CB290;

/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
typedef struct LightingObject {
    u8 pad0[0x40];
    u8 *segmentData;
    u8 pad44[0xC];
    s32 objectLight;
    u8 pad54[0x3B];
    u8 unk8F;
    u8 pad90[3];
    u8 segmentIndex;
} LightingObject;

/* PROVENANCE: adapted from JFG's public decomp, include/structs.h, with Mickey offsets. */
typedef struct LightSourceHeader {
    u8 pad0[0x28];
    s8 lightCount;
    u8 pad29[0x23];
    ObjectLightEntry *lights;
} LightSourceHeader;

struct LightSourceObject {
    u8 pad0[0x40];
    LightSourceHeader *header;
    u8 pad44[0x2C];
    UnkLight **lights;
};

extern LightingObject **func_8000572C(s32 *start, s32 *end);
extern void func_8001953C(LightingObject *object, s32 objectLight);

/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void freeLights(void) {
    if (D_80079498 != 0) {
        mmFree(D_80079498);
        D_80079498 = 0;
        D_8007949C = 0;
    }
    if (D_800CB290 != 0) {
        mmFree(D_800CB290);
        D_800CB290 = 0;
    }
    if (D_800794A0 != 0) {
        mmFree(D_800794A0);
        D_800794A0 = 0;
    }
    D_80079494 = 0;
    D_80079490 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001879C.s")
/* PROVENANCE: adapted from DKR's public decomp, src/lights.c, and Mickey's own assembly. */
void func_800188CC(UnkLight *light) {
    f32 radius;

    if (!(light->unk3 & 0x40)) {
        radius = light->radius;
        light->unk6C = func_8000D62C(
            light->x, light->y, light->z,
            radius * 1.25f, radius * D_800817B0,
            (light->red * light->unk43) >> 8,
            (light->green * light->unk43) >> 8,
            (light->blue * light->unk43) >> 8);
    } else {
        light->unk6C = 0;
    }
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
UnkLight *addRomdefLight(s32 arg0, RomdefLight *entry) {
    UnkLight *light;
    u8 *levelData;

    light = NULL;
    if (D_80079494 < D_80079490) {
        light = D_80079498[D_80079494];
        D_80079494++;
        light->unk0 = (entry->modeAndType >> 4) & 0xF;
        light->unk1 = entry->modeAndType & 0xF;
        if ((entry->flags & 0xE0) == 0x40) {
            light->unk0 = 3;
        }
        light->unk2 = 7;
        light->unk3 = ~entry->enabledFlags & 0x61;
        light->home = 0;
        light->index = -1;
        light->directionX = 0.0f;
        light->directionY = 0.0f;
        light->directionZ = 0.0f;
        light->owner = NULL;
        light->x = entry->x;
        light->y = entry->y;
        light->z = entry->z;
        light->radius = entry->radius;
        light->radius2 = light->radius;
        light->radius3 = light->radius;
        if (light->unk0 == 2) {
            light->radius2 = entry->radius2;
        }
        light->radiusSquare = light->radius * light->radius;
        light->radiusInverse = 1.0f / light->radius;
        light->red = entry->red;
        light->green = entry->green;
        light->blue = entry->blue;
        light->unk43 = entry->intensity;
        light->unk44 = (u32) entry->intensity;
        light->unk54 = 0;
        if (entry->colourCycleIndex < 7) {
            levelData = func_8002679C();
            if (*(s16 *)(levelData + 0x94 + (entry->colourCycleIndex * 2)) != -1) {
                initColourCycle(light->colourCycle, *(s16 *)(levelData + 0x94 + (entry->colourCycleIndex * 2)), entry);
            }
        }
        light->value58 = entry->value58;
        light->value5C = entry->value5C;
        light->value5A = entry->value5A;
        light->value5E = entry->value5E;
        func_800188CC(light);
        func_80018F08(light, 0);
    }
    return light;
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
UnkLight *addObjectLight(s32 owner, ObjectLightEntry *entry) {
    UnkLight *light;

    light = NULL;
    if (D_80079494 < D_80079490) {
        light = D_80079498[D_80079494];
        D_80079494++;
        light->unk0 = entry->mode;
        light->unk1 = entry->type;
        light->unk2 = 7;
        light->unk3 = entry->flags;
        light->home = entry->home;
        light->index = entry->index;
        light->directionX = entry->x;
        light->directionY = entry->y;
        light->directionZ = entry->z;
        light->owner = (void *) owner;
        light->x = light->directionX;
        light->y = light->directionY;
        light->z = light->directionZ;
        light->radius = (u32) entry->radius;
        light->radius2 = light->radius;
        light->radius3 = light->radius;
        if (light->unk0 == 2) {
            light->radius2 = (u32) entry->radius2;
        }
        light->radiusSquare = light->radius * light->radius;
        light->radiusInverse = 1.0f / light->radius;
        light->red = entry->red;
        light->green = entry->green;
        light->blue = entry->blue;
        light->unk43 = entry->intensity;
        light->unk44 = (u32) entry->intensity;
        if (entry->colourCycleIndex != -1) {
            initColourCycle(light->colourCycle, entry->colourCycleIndex, entry);
        } else {
            light->unk54 = 0;
        }
        light->value58 = entry->value58 << 8;
        light->value5C = 0;
        light->value5A = entry->value5A << 8;
        light->value5E = 0;
        func_800188CC(light);
        func_80018F08(light, 0);
    }
    return light;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void turnLightOff(UnkLight *light) {
    light->unk3 &= ~1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void turnLightOn(UnkLight *light) {
    light->unk3 |= 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void toggleLight(UnkLight *light) {
    light->unk3 ^= 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightColour(UnkLight *light, u8 red, u8 green, u8 blue) {
    light->red = red;
    light->green = green;
    light->blue = blue;
    light->unk2 |= 2;
    light->unk54 = 0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightColourCycle(s32 arg0, s32 arg1) {
    initColourCycle(arg0 + 0x48, arg1);
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void changeLightIntensity(UnkLight *light, u8 intensity) {
    light->unk43 = intensity;
    light->unk44 = intensity;
    if (light->unk6C != 0) {
        light->unk2 |= 2;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018E7C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80018F08.s")
/* PROVENANCE: adapted from DKR's public decomp, src/lights.c, and Mickey's own assembly. */
void killLight(UnkLight *light) {
    s32 i;
    UnkLight *entry;

    entry = NULL;
    for (i = 0; (i < D_80079494) && (entry == NULL); i++) {
        if (light == D_80079498[i]) {
            entry = D_80079498[i];
        }
    }
    if (entry != NULL) {
        if (light->unk6C != 0) {
            func_8000D728(light->unk6C, i, D_80079494, (s32) entry);
        }
        D_80079494--;
        for (i--; i < D_80079494; i++) {
            D_80079498[i] = D_80079498[i + 1];
        }
        D_80079498[D_80079494] = entry;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void **lightGetLights(s32 *count) {
    *count = D_80079494;
    return D_80079498;
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
UnkLight *lightGetStrongestEffect(f32 x, f32 y, f32 z) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distanceSquare;
    f32 effect;
    f32 strongestEffect;
    s32 index;
    s32 offset;
    UnkLight *light;
    UnkLight *strongestLight;

    strongestEffect = 0.0f;
    strongestLight = NULL;
    index = D_80079494;
    if (index--) {
        offset = index * 4; do {
            light = *(UnkLight **)((u8 *) D_80079498 + offset);
            if (light != NULL) {
                dx = x - light->x;
                dy = y - light->y;
                dz = z - light->z;
                distanceSquare = (dx * dx) + (dy * dy) + (dz * dz);
                if (distanceSquare < light->radiusSquare) {
                    effect = (1.0f - (sqrtf(distanceSquare) * light->radiusInverse)) * light->unk44;
                    if (strongestEffect < effect) {
                        strongestEffect = effect;
                        strongestLight = light;
                    }
                }
            }
            offset -= 4;
        } while (index--);
    }
    return strongestLight;
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
void lightUpdateObjects(void) {
    s32 objectLight;
    LightingObject **objects;
    LightingObject *object;
    s32 start;
    s32 end;

    objects = func_8000572C(&start, &end);
    if (start < end) {
        do {
            object = objects[start];
            start++;
            objectLight = object->objectLight;
            if ((objectLight != 0) &&
                (*(s8 *) (object->segmentData + object->segmentIndex + 0x1E) == 0) &&
                (object->unk8F == 0)) {
                func_8001953C(object, objectLight);
            }
        } while (start < end);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001953C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019934.s")
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
f32 lightDirectionCalc(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6) {
    f32 temp_f0;
    f32 var_f2;

    if (arg6 > 0.0f) {
        temp_f0 = 1.0f / arg6;
        var_f2 = (arg3 * temp_f0 * arg0) + (arg4 * temp_f0 * arg1) + (arg5 * temp_f0 * arg2);
        if (var_f2 < 0.0f) {
            var_f2 = 0.0f;
        }
    } else {
        var_f2 = 1.0f;
    }
    return var_f2;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019AB8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019D98.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019DE8.s")
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c, with Mickey offsets. */
void lightSetupLightSources(LightSourceObject *object) {
    s32 i;

    for (i = 0; i < object->header->lightCount; i++) {
        object->lights[i] = addObjectLight((s32) object, &object->header->lights[i]);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019F7C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A008.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A154.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A23C.s")
