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
extern f32 func_8002A8BC(s32 angle);
extern void mmFree(void *ptr);
extern void *func_8002B280(s32 size, s32 tag);
extern void lightCreateLightTable(s32 red, s32 green, s32 blue, void *table);
extern void func_8000D728(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern s32 func_8000D62C(f32 x, f32 y, f32 z, f32 radius, f32 radius2, s32 red, s32 green, s32 blue);
extern void func_800188CC(UnkLight *light);
extern void func_80018F08(UnkLight *light, s32 updateRate);
extern u8 *levelGetLevel(void);
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

typedef struct FlareEntry {
    s8 type;
    u8 subtype;
    u8 pad2[6];
    f32 x;
    f32 y;
    f32 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    s16 kind;
    u16 size;
    s16 scaledSize;
    s8 index;
    s8 enabled;
} FlareEntry;

typedef struct GlowEntry {
    u8 pad0[4];
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u16 size;
    u8 scale;
    u8 pad11[2];
    u8 subtype;
} GlowEntry;

typedef struct GlowObject {
    u8 pad0[0x3C];
    GlowEntry *entry;
    u8 pad40[0x24];
    void *flare;
} GlowObject;

typedef struct ObjectLightState {
    f32 directionX;
    f32 directionY;
    f32 directionZ;
    s32 scaleStep;
    s32 colourStep;
    u8 shift;
    u8 endValue;
    u8 valueDelta;
    u8 startValue;
    void *table;
    s16 yaw;
    s16 pitch;
} ObjectLightState;

typedef struct FlareHeader {
    u8 pad0[0x29];
    u8 flareCount;
    u8 pad2A[0x26];
    FlareEntry *flares;
} FlareHeader;

struct FlareObject {
    u8 pad0[0x40];
    FlareHeader *header;
    u8 pad44[0x30];
    void **flares;
};

extern LightingObject **func_8000572C(s32 *start, s32 *end);
extern void func_8001953C(LightingObject *object, s32 objectLight);
extern void func_80019DE8(ObjectLightState *state, s32 arg1, s32 arg2, s16 arg3, s16 arg4, s32 arg5);
extern void mathOneFloatRPY(s16 *rotation, f32 *output);
extern void *camlightAdd(void *object, FlareEntry *entry);
extern void camlightDelete(void);
extern ObjectLightState D_800CB298;

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
#ifdef NON_MATCHING
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c, with Mickey's allocation sizes. */
void setupLights(s32 count, s32 arg1, s32 arg2) {
    s32 i;
    void **buffer;

    freeLights();
    D_80079490 = count;
    buffer = func_8002B280(D_80079490 * 0x78, 0x89);
    D_800CB290 = func_8002B280((D_80079490 << 9) + 0x200, 0x89);
    D_800794A0 = func_8002B280(0x240, 0x89);
    D_8007949C = &buffer[D_80079490];
    D_80079498 = buffer;
    for (i = 0; i < D_80079490; i++) {
        D_80079498[i] = (i * 0x74) + (u8 *) D_8007949C;
        *(void **) ((u8 *) D_8007949C + (i * 0x74) + 0x70) =
            (u8 *) D_800CB290 + 0x200 + (i * 0x200);
    }
    lightCreateLightTable(255, 255, 255, D_800CB290);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/setupLights.s")
#endif
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
            levelData = levelGetLevel();
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
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c. */
void lightUpdateLights(s32 updateRate) {
    s32 i;

    for (i = 0; i < D_80079494; i++) {
        func_80018F08(D_80079498[i], updateRate);
    }
}
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
#ifdef NON_MATCHING
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c, with Mickey's trigonometry helper. */
f32 func_80019934(f32 arg0, f32 arg1, f32 arg2, s32 arg3) {
    f32 temp;

    temp = arg1 * arg2;
    switch (arg3) {
        case 1:
            temp = 1.0f - temp;
            break;
        case 2:
            temp = 1.0f - sqrtf(temp);
            break;
        case 3:
            temp = func_8002A8BC(temp * 16384.0f);
            break;
        case 4:
            temp = func_8002A8BC(temp * 16384.0f);
            temp *= temp;
            break;
        case 5:
            temp = 1.0f - temp;
            temp *= temp;
            break;
    }
    return arg0 * temp;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019934.s")
#endif
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
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
void lightDefaultObjectLight(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s32 arg4) {
    func_80019DE8(&D_800CB298, arg0, arg1, arg2, arg3, arg4);
}
#ifdef NON_MATCHING
/* PROVENANCE: adapted from JFG's public asm/nonmatchings/lights/lightSetObjectLight.s, with Mickey's globals. */
void func_80019DE8(ObjectLightState *state, s32 arg1, s32 arg2, s16 pitch, s16 yaw, s32 shift) {
    s16 rotation[3];
    f32 direction[3];

    arg1 &= 0xFF;
    arg2 &= 0xFF;
    shift &= 7;
    if (arg2 < arg1) {
        arg1 = arg2;
    }
    state->startValue = arg1;
    state->endValue = arg2;
    state->valueDelta = arg2 - arg1;
    state->colourStep = (state->valueDelta & 0xFF) << (shift & 0xFF);
    state->shift = shift;
    state->scaleStep = (1 << (8 - shift)) << 6;
    state->pitch = pitch;
    state->yaw = yaw;
    rotation[1] = pitch;
    rotation[2] = 0;
    rotation[0] = yaw;
    direction[0] = 0.0f;
    direction[1] = 0.0f;
    direction[2] = 1.0f;
    mathOneFloatRPY(rotation, direction);
    state->directionX = direction[0];
    state->directionY = direction[1];
    state->directionZ = direction[2];
    state->table = D_800CB290;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_80019DE8.s")
#endif
/* PROVENANCE: adapted from JFG's public decomp, src/lights.c, with Mickey offsets. */
void lightSetupLightSources(LightSourceObject *object) {
    s32 i;

    for (i = 0; i < object->header->lightCount; i++) {
        object->lights[i] = addObjectLight((s32) object, &object->header->lights[i]);
    }
}
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
void lightSetupFlareSources(FlareObject *object) {
    s32 i;

    for (i = 0; i < object->header->flareCount; i++) {
        object->flares[i] = camlightAdd(object, &object->header->flares[i]);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A008.s")
#ifdef NON_MATCHING
/* PROVENANCE: adapted from JFG's public asm/nonmatchings/lights/lightAdjustGlowingLight.s, with Mickey's constants and offsets. */
void func_8001A154(GlowObject *object) {
    FlareEntry flare;
    GlowEntry *entry;
    s32 scaledSize;

    entry = object->entry;
    flare.type = 0x41;
    flare.subtype = entry->subtype;
    flare.x = entry->x;
    flare.y = entry->y;
    flare.z = entry->z;
    flare.red = entry->red;
    flare.green = entry->green;
    flare.blue = entry->blue & 0xFFFFU;
    flare.alpha = entry->alpha;
    flare.kind = 0x2B;
    flare.size = entry->size;
    scaledSize = (entry->size * entry->scale) / 100;
    flare.index = -1;
    flare.enabled = 0;
    flare.scaledSize = scaledSize;
    object->flare = camlightAdd(NULL, &flare);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/lights/func_8001A154.s")
#endif
/* PROVENANCE: adapted from JFG's public decomp comparison and Mickey's own assembly. */
s32 lightKillGlowingLight(void) {
    camlightDelete();
    return 1;
}
