/*
 * Resident animation/collision block -- ROM 0x50C00-0x58570
 * (VRAM 0x80050000-0x80057970).
 *
 * PROVENANCE -- names and structural comparisons in this file use Jet Force
 * Gemini's public decompilation, principally src/anim.c, src/hit.c, src/fmv.c,
 * and their declarations. JFG is a permitted published retail-derived decomp
 * under docs/CLEANROOM.md. Mickey's own ROM remains authoritative; the block
 * is kept under its existing 16-byte-aligned boundaries because no internal
 * whole-object boundary has yet been proved.
 *
 * Flags: -O2 -mips2 -32, inherited from the src/main/ build rule.
 */

#include "PR/ultratypes.h"
#include "game/anim.h"

extern s32 D_8007D690;
extern void *D_8007D694;
extern s32 D_8007D6B0;
extern s32 D_8007D684;
extern u32 *D_800D6B04;
extern u8 D_800D6BF8[];
extern u8 D_800D6C38[];
extern s16 D_800D6C3E;
extern s16 D_800D6C44;
extern s32 D_800D6C48;
extern s16 D_800D6C4C;
extern s16 D_800D6C52;
extern s16 D_800D6C54;
extern f32 D_8007D6B4;

typedef struct AnimCameraSource {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u8 pad6[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x16];
    s16 unk2E;
} AnimCameraSource;

typedef struct AnimCameraTarget {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u8 pad6[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x26];
    s16 unk3E;
} AnimCameraTarget;

extern AnimCameraSource *D_800D6B08[];

typedef struct AnimLightReset {
    s32 unk0;
    u8 pad4[0xC];
    s32 unk10;
    u8 pad14[0xC];
    s32 unk20;
    u8 pad24[0xC];
    s32 unk30;
    u8 pad34[0xC];
} AnimLightReset;

extern AnimLightReset D_800D6C58[];

void *func_8002B280();
void piRomLoadSection();
void func_80021504(f32 value, s32 arg1);
f32 sqrtf(f32 value);

/*
 * PROVENANCE: adapted from JFG's func_80076020_76C20. Mickey's globals and
 * final compiler output are independently established from Mickey's ROM.
 */
void func_80050000(s32 *stream) {
    D_800D6D54 = stream;
    D_800D6D58 = (u8 *) *stream;
    D_800D6D5C = 0x80;
}

/*
 * PROVENANCE: adapted from JFG's func_80076044_76C44. The bitstream globals
 * are Mickey's, and the compiled result is checked against Mickey's ROM.
 */
s32 func_80050024(u32 bitCount) {
    s32 value;

    value = 0;
    if (bitCount != 0) {
        bitCount = 1 << (bitCount + 0x1F);
        do {
            if (D_800D6D5C == 0) {
                D_800D6D58++;
                D_800D6D5C = 0x80;
            }
            if (*D_800D6D58 & D_800D6D5C) {
                value |= bitCount;
            }
            bitCount >>= 1;
            D_800D6D5C >>= 1;
        } while (bitCount != 0);
    }
    return value;
}

/*
 * PROVENANCE: adapted from JFG's func_800760C0_76CC0. Mickey's ROM fixes the
 * signed-extension expression and all generated instruction choices.
 */
s32 func_800500A4(u32 bitCount) {
    u32 signMask;
    s32 value;

    value = 0;
    if (bitCount != 0) {
        signMask = 0xFFFFFFFF << (bitCount - 1);
        bitCount = 1 << (bitCount - 1);
        do {
            if (D_800D6D5C == 0) {
                D_800D6D58++;
                D_800D6D5C = 0x80;
            }
            if (*D_800D6D58 & D_800D6D5C) {
                value |= bitCount;
            }
            bitCount >>= 1;
            D_800D6D5C >>= 1;
        } while (bitCount != 0);
        if (value & signMask) {
            value |= signMask;
        }
    }
    return value;
}

void func_8005013C(void) {
    if (D_800D6D5C != 0x80) {
        D_800D6D58++;
    }
    *D_800D6D54 = (s32) D_800D6D58;
}


/*
 * Clear the current animation-sequence cursors. The exact JFG donor assembly
 * corroborates the three-global shape; this C is reconstructed from Mickey.
 */
void func_8005017C(void) {
    if (D_8007D698 != NULL) {
        D_8007D698 = NULL;
        D_8007D69C = NULL;
        D_8007D6A0 = 0;
    }
}

s32 func_800501AC(u16 *entry) {
    return D_8007D6C0[(entry[1] >> 8) & 0xFF];
}

/* JFG corroborates this scan's CFG; the C expression choices are Mickey-led. */
#ifdef NON_MATCHING
s32 func_800501C8() {
    s32 step;
    s32 done;
    s32 entryCount;
    s32 total;
    u8 *cursor;

    cursor = D_8007D698;
    total = 0;
    done = 0;
    entryCount = 0;
    if (cursor != NULL) {
        do {
            if (((((u16 *) cursor)[1] >> 8) & 0xFF) == 0x7F) {
                done = 1;
            }
            step = func_800501AC((u16 *) cursor);
            if (step != 0) {
                total += step;
                cursor += (step >> 1) * 2;
                entryCount++;
                if (entryCount >= 0x2001) {
                    done = 1;
                }
            } else {
                total = 0;
                done = 1;
            }
        } while (done == 0);
    }
    return total;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800501C8.s")
#endif

/* Exact JFG donor assembly corroborates this setup shape; C is Mickey-led. */
void func_8005027C(void) {
    s32 *base;
    s32 header;

    base = D_8007D68C;
    header = *base;
    D_8007D698 = (u8 *) base + (header & 0xFFFFFF);
    D_8007D69C = D_8007D698;
    D_8007D6A0 = func_800501C8(&D_8007D698);
}

void func_800502CC(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (path->unk8 != NULL) {
            func_80006EA0(path->unk8);
        }
        mmFree(path);
    }
    D_800D6B00[pathIndex] = NULL;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050348.s")
#ifdef NON_MATCHING
/*
 * JFG's animseqResetPath assembly corroborates this Mickey-led reset.
 * Plateau: exact 75-instruction size; first mismatch +0x40. The typed trap
 * alias leaves one relocation identity and the remaining allocator cycle.
 */
#pragma weak animResetTrap = TrapDanglingJump
extern s32 animResetTrap(AnimPath *, f32, s32, s32);
extern void func_800031E8(s32 handle);
void func_8005055C(u8 pathIndex) {
    AnimPath *path;
    AnimPathObject *object;
    s32 soundHandle;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (!(path->flags & 8)) {
            object = path->unk8;
            path->flags &= 0x80;
            path->unk10 = 1.0f;
            path->unk1 = path->unk0;
            path->unk1C = 0.0f;
            path->unk14 = path->unk6;
            path->unk15 = path->unk7;
            path->currentNode = path->nodes;
            path->unkC = path->unk4 / 16384.0f;
            if (object != NULL) {
                object->unk6 |= 0x400;
                path->unk24 = 0xFF;
                path->unk25 = 0xFF;
                path->unk26 = 0;
                path->unk27 = 0;
                path->unk2A = 0;
                path->unk2C = 1.0f;
                path->unk30 = 0.0f;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 0;
                }
                animResetTrap(path, 0.0f, 0, 0);
                soundHandle = object->soundHandle;
                if (soundHandle != 0) {
                    func_800031E8(soundHandle);
                    object->soundHandle = 0;
                }
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005055C.s")
#endif
void animseqStartPath(u8 pathIndex) {
    AnimPath *path;
    AnimPathObject *object;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (!(path->flags & 8) && !(path->flags & 1)) {
            object = path->unk8;
            path->flags |= 1;
            path->flags &= ~4;
            if (object != NULL) {
                object->unk6 &= ~0x400;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 1;
                }
            }
        }
    }
}
void animseqStopPath(u8 pathIndex) {
    AnimPath *path;
    AnimPathObject *object;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (!(path->flags & 8) && (path->flags & 5)) {
            object = path->unk8;
            path->flags &= ~5;
            if (object != NULL) {
                object->unk6 |= 0x400;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 0;
                }
            }
        }
    }
}
u32 func_8005077C(u8 pathIndex) {
    AnimPath *path;
    u32 result;

    path = D_800D6B00[pathIndex];
    result = 1;
    if (path != NULL) {
        return (path->flags & 1) == 0;
    }
    return result;
}

void animseqHoldPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (!(path->flags & 8)) {
            if (path->flags & 1) {
                path->flags &= ~1;
            } else if (path->unk8 != NULL) {
                path->unk8->unk6 &= ~0x400;
                if (path->unk8->unk58 != NULL) {
                    path->unk8->unk58->unk132 = 1;
                }
            }
            path->flags |= 4;
        }
    }
}
void animseqLockPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        path->flags |= 8;
    }
}

void animseqUnLockPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        path->flags &= ~8;
    }
}

AnimPath *func_800508B4(u8 pathIndex) {
    return D_800D6B00[pathIndex];
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800508D4.s")
/* JFG's animseqLinkNodes assembly corroborates this Mickey-led body. */
void func_80050AD4(u8 pathIndex) {
    AnimPath *path;
    s32 nodeIndex;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        if (path->nodeCount >= 2) {
            nodeIndex = 0;
            if (path->nodeCount > 0) {
                do {
                    if (nodeIndex > 0) {
                        path->nodes[nodeIndex].previous =
                            &path->nodes[nodeIndex - 1];
                    }
                    if (nodeIndex < path->nodeCount - 1) {
                        path->nodes[nodeIndex].next =
                            &path->nodes[nodeIndex + 1];
                    }
                    nodeIndex++;
                } while (nodeIndex < path->nodeCount);
            }
        }
        if (path->flags & 0x80) {
            path->nodes[0].previous = &path->nodes[path->nodeCount - 1];
            path->nodes[path->nodeCount - 1].next = &path->nodes[0];
        } else {
            path->nodes[0].previous = &path->nodes[0];
            path->nodes[path->nodeCount - 1].next =
                &path->nodes[path->nodeCount - 1];
        }
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050BF4.s")
void func_80050D50(void) {
    void **entry = D_800D6B18, **end = D_800D6B58;
    do {
        if (*entry != NULL) {
            amSndStop(*entry);
            *entry = NULL;
        }
        entry++;
    } while (entry != end);
}

void animseqFreeLevelData(void) {
    if (D_8007D680 != NULL) {
        mmFree(D_8007D680);
        D_8007D680 = NULL;
        D_8007D688 = -1;
        func_80050E9C();
    }
}

#ifdef NON_MATCHING
void func_80050DF0(s32 levelId) {
    u32 *entry;
    s32 source;

    if (levelId != -1 && levelId != D_8007D688) {
        animseqFreeLevelData();
        entry = &D_800D6B04[levelId];
        source = entry[0];
        D_8007D684 = entry[1] - source;
        if (D_8007D684 > 0) {
            D_8007D680 = func_8002B280(D_8007D684, 0x81, source);
            if (D_8007D680 != NULL) {
                piRomLoadSection(0x3E, D_8007D680, source, D_8007D684);
                D_8007D688 = levelId;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050DF0.s")
#endif
/*
 * PROVENANCE: adapted from JFG's animseqFreeGroup assembly. Mickey's data
 * boundaries, calls, scheduling, and final compiler output remain authoritative.
 */
#ifdef NON_MATCHING
void func_80050E9C(void) {
    s32 emptyIndex;
    u8 *entry;
    AnimLightReset *light;
    s32 pathIndex;

    if (D_8007D68C != NULL) {
        if (D_8007D694 != NULL) {
            mmFree(D_8007D68C);
            D_8007D694 = NULL;
        }
        emptyIndex = -1;
        D_8007D68C = NULL;
        D_8007D690 = emptyIndex;
        func_8005017C();

        pathIndex = 0;
        do {
            func_800502CC((u8) pathIndex);
            pathIndex++;
        } while ((pathIndex < 0x100) != 0);

        entry = (u8 *) D_800D6B08;
        do {
            *(s32 *) entry = 0;
            entry += 4;
        } while (entry < (u8 *) D_800D6B18);

        entry = (u8 *) D_800D6B58;
        do {
            entry[0] = 0xFF;
            *(s32 *) (entry + 4) = 0;
            *(s32 *) (entry + 0xC) = 0;
            entry += 0x14;
        } while (entry < D_800D6BF8);

        entry = D_800D6BF8;
        do {
            entry[0] = emptyIndex;
            entry += 8;
        } while (entry < D_800D6C38);

        D_8007D6B0 = 0;
        light = D_800D6C58;
        do {
            light->unk0 = 0;
            light->unk10 = 0;
            light->unk20 = 0;
            light->unk30 = 0;
            light++;
        } while (light != (AnimLightReset *) D_800D6D18);

        D_800D6C3E = 0;
        D_800D6C44 = 0;
        D_800D6C48 = 0;
        D_800D6C52 = 0xFF;
        D_800D6C54 = D_800D6C52;
        D_800D6C4C = 0;
        func_80050D50();
        func_800534C0();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050E9C.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051004.s")
/* Exact JFG donor assembly corroborates the loop; C is Mickey-led. */
void animseqInitGroup(void) {
    s32 pathIndex;

    pathIndex = 0;
    do {
        func_80050348(pathIndex & 0xFF);
        pathIndex++;
    } while (pathIndex != 0x100);
}

/* JFG corroborates the reset-path loop; the remaining control flow is Mickey-led. */
void animseqResetGroup(void) {
    s32 pathIndex;

    if (D_8007D68C != NULL) {
        pathIndex = 0;
        do {
            func_8005055C(pathIndex & 0xFF);
            pathIndex++;
        } while (pathIndex != 0x100);
        D_8007D69C = D_8007D698;
        D_8007D6A8 = 0;
        D_8007D6AC = 0.0f;
        if (TrapDanglingJump(osRomBase) == 0) {
            D_8007D69C = NULL;
        }
        func_80050D50();
        func_800534C0();
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800511C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051364.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800517E0.s")
/*
 * PROVENANCE: JFG's public src/anim.c supplies the ordered animseqCamera
 * comparison; the body and local layouts are reconstructed from Mickey.
 */
AnimCameraSource *func_80053420(s32 index, AnimCameraTarget *target) {
    AnimCameraSource *source;

    source = D_800D6B08[index];
    if (source != NULL) {
        target->unkC = source->unkC;
        target->unk10 = source->unk10;
        target->unk14 = source->unk14;
        target->unk3E = source->unk2E;
        target->unk0 = 0x8000 - source->unk0;
        target->unk2 = -source->unk2;
        target->unk4 = source->unk4;
        func_80021504(D_8007D6B4, 0);
    }
    return source;
}
/* JFG's ordered anim.c tail and this store establish the tier-D Play name. */
void animseqPlay(void) {
    D_8007D6A4 = 1;
}

void func_800534C0(s32 i) {
    AnimPauseSlot *slot;

    /* The incoming scratch value is replaced before its first use. */
    slot = D_800D6D18;
    i = 4;
    do {
        slot->unkB = 0;
        slot->unk0 = 0;
        slot++;
    } while (i--);
}

void func_800534EC(s32 arg0) {
    AnimPauseSlot *slot;
    s32 i;

    slot = D_800D6D18;
    i = 4;
    do {
        if (slot->unkB > 0) {
            TrapDanglingJump(arg0, slot);
        }
        slot++;
    } while (i--);
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80053550.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80053868.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80054B3C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055104.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800557F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055970.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055B24.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055D08.s")
void func_80055E50(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    HitCopyTarget *target;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    firstSource = first->source;
    first->position.x = firstSource->current.x;
    first->position.y = firstSource->current.y;
    first->position.z = firstSource->current.z;
    firstSource->previous.x = firstSource->current.x;
    firstSource->previous.y = firstSource->current.y;
    firstSource->previous.z = firstSource->current.z;

    secondSource = second->source;
    second->position.x = secondSource->current.x;
    second->position.y = secondSource->current.y;
    second->position.z = secondSource->current.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;

    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    target = second->target;
    target->unk1C = deltaX / distance;
    target->unk20 = deltaY / distance;
    target->unk24 = deltaZ / distance;
    TrapDanglingJump(first, 1, second);
    TrapDanglingJump(second, 0xA);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055F64.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800560D0.s")
void func_80056274(HitCopyState *first, HitCopyState *second, f32 unused) {
    HitCopyTarget *firstTarget;
    HitCopyTarget *secondTarget;
    HitCopySource *firstSource;
    HitCopySource *secondSource;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    firstSource = first->source;
    secondTarget = second->target;
    secondSource = second->source;
    firstTarget = first->target;
    first->position.x = firstSource->current.x;
    first->position.y = firstSource->current.y;
    first->position.z = firstSource->current.z;
    firstSource->previous.x = firstSource->current.x;
    firstSource->previous.y = firstSource->current.y;
    firstSource->previous.z = firstSource->current.z;

    second->position.x = secondSource->current.x;
    second->position.y = secondSource->current.y;
    second->position.z = secondSource->current.z;
    secondSource->previous.x = secondSource->current.x;
    secondSource->previous.y = secondSource->current.y;
    secondSource->previous.z = secondSource->current.z;

    deltaX = secondSource->current.x - firstSource->current.x;
    deltaY = secondSource->current.y - firstSource->current.y;
    deltaZ = secondSource->current.z - firstSource->current.z;
    distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                     (deltaZ * deltaZ));

    firstTarget->unk1C = deltaX / distance;
    firstTarget->unk20 = deltaY / distance;
    firstTarget->unk24 = deltaZ / distance;
    secondTarget->unk1C = -firstTarget->unk1C;
    secondTarget->unk20 = -firstTarget->unk20;
    secondTarget->unk24 = -firstTarget->unk24;
    TrapDanglingJump(first, 6, firstTarget);
    TrapDanglingJump(second, 0xA);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800563B4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80056DD8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005716C.s")
void func_800572AC(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat) {
    f32 currentX;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    HitCopySource *source;
    HitCopyTarget *target;

    source = state->source;
    currentX = source->current.x;
    target = state->target;
    deltaX = state->position.x - source->previous.x;
    deltaY = state->position.y - source->previous.y;
    deltaZ = state->position.z - source->previous.z;
    source->previous.x = currentX;
    source->previous.y = source->current.y;
    source->previous.z = source->current.z;
    state->position.x = currentX + deltaX;
    state->position.y = source->previous.y + deltaY;
    state->position.z = source->previous.z + deltaZ;
    target->unk14 = position->x;
    target->unk18 = position->y;
    target->unk1C = position->z;
    TrapDanglingJump(state, 0x16);
}
void func_80057350(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat) {
    HitCopySource *source;
    HitCopyTarget *target;

    source = state->source;
    target = state->target;
    state->position.x = source->current.x;
    state->position.y = source->current.y;
    state->position.z = source->current.z;
    source->previous.x = source->current.x;
    source->previous.y = source->current.y;
    source->previous.z = source->current.z;
    target->unk1C = position->x;
    target->unk20 = position->y;
    target->unk24 = position->z;
    TrapDanglingJump(state, 0xE);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800573C8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005776C.s")
/*
 * PROVENANCE: adapted from JFG's src/fmvInit.c. Mickey's ROM establishes the
 * resource ID, globals, structure layout, and final compiler output here.
 */
void fmvInit(void) {
    FmvPlayer *player;
    s32 i;

    D_800D76D0[0] = piRomLoad(0x41);
    player = D_800D76D8;

    i = 2;
    while (i--) {
        player->unk0 = -1;
        player->unk14 = 0;
        player->unk18 = 0;
        player->unk1C = 0;
        player->unk20 = 0;
        player++;
    }
}
