/*
 * Resident front-end menu -- ROM 0x39350-0x3B1A0 (VRAM 0x80038750).
 *
 * The translation-unit identity and function crosswalk come from Jet Force
 * Gemini's public decompilation of the same Rare engine. The boundary evidence
 * and provenance are recorded in docs/modules.md section 3.4. Functions stay
 * under GLOBAL_ASM until their C compiles to Mickey's bytes exactly.
 *
 * Flags: -O2 -mips2 -32, via the shared src/main rule.
 */

#include "PR/ultratypes.h"
#include "game/font.h"
#include "game/gameVi.h"
#include "game/menu.h"
#include "game/pi.h"

/* PROVENANCE: base layout adapted from JFG's public decomp,
 * src/menu.h::Resbitfield; twoPlayerSplit and stereoMode are Mickey-derived
 * from their paired getters and byte-preserving setters. */
typedef struct MenuScreenModeBits {
    u32 unused : 1;
    u32 modeBit0 : 1;
    u32 modeBit1 : 1;
    u32 twoPlayerSplit : 1;
    u32 unusedStereoGap : 5;
    u32 stereoMode : 2;
    u32 unusedLanguageGap : 5;
    u32 language : 6;
    u32 rest : 10;
} MenuScreenModeBits;

extern s8 D_800D312B;
extern s8 D_800D3050;
extern MenuScreenModeBits D_800D3128;
extern u8 D_8007C08C;
extern u8 D_8007C090;
extern u8 D_8007C088;
extern s32 D_8007C098;
extern s32 *D_8007C094;
extern s16 D_8007BF70;
extern s16 D_8007BF7C[];
extern u8 D_8007BEF4;
extern u8 D_8007BEF8;
extern u8 D_8007BF30;
extern u8 D_8007BF34;
extern u8 D_8007C0A0;
extern s32 D_8007C09C;
extern s32 D_8007C1A4;
extern s32 D_8007C1AC;
extern u8 D_8007C308[];
extern s32 D_800C947C;
extern s32 D_800D314C;
extern s32 D_800D3150[];
extern s32 D_800D3168[];
extern u8 D_800826C0[];
extern u16 D_800D312A;
extern u16 D_800D312C;
extern u16 D_800D312E;
extern void amTuneStop(void);
extern void amTuneSetGlobalVolume(s32 volume);
extern void alSurround_OutputType(u8 mode);
extern void func_80038750();
extern void func_800389CC(void);
extern void func_80038BC4(void);
extern void func_8003968C(void);
extern s32 levelGetRegionNo(void);
extern s8 viGetWideAdjust(void);
extern void gsSndpSetGlobalVolume(s32 volume);
extern void viSetWideAdjust(s32 offset);
extern s32 TrapDanglingJump();
extern void amSndPlay(s32 soundId, s32 *handle);
extern void amSndSetVol(s32 soundId, s32 handle, s32 volume, s32 *handleOut);
extern void amSndStop(s32 handle);
extern void func_80000510(u8 value, s16 arg1);
extern void func_80006EA0(void *object);
extern void func_80006FA0(void);
extern s32 func_80005820(s32 arg0);
extern u8 *func_80028F54(void);
extern void func_800347A0(void *texture);
extern void func_800359D4(void *sprite);
extern void func_80039A9C(s32 assetId);
extern void func_80039BE4(s32 assetId);
extern void func_80039720(s32 updateRate);
extern void func_8004BF64(s32 windowId);
extern void func_80044BC8(s32 arg0, u8 *source, s32 line);
extern u32 joyGetButtons(s32 controller);
extern u32 joyGetPressed(s32 controller);
extern s8 joyGetStickX(s32 controller);
extern s8 joyGetStickY(s32 controller);
extern void mainTitlePageInit(s32 mode);
extern void modFreeModel(void *model);
extern void *func_8002B280(s32 size, s32 tag);
extern u32 *piRomLoad(u32 assetIndex);

extern u32 D_800D3170[4];
extern u32 D_800D3180[4];
extern s8 D_800D3190[4];
extern s8 D_800D3194[4];
extern s8 D_800D3198[4];
extern s8 D_800D319C[4];
extern u32 D_800D31A0[4];
extern u8 D_800D31B0;
extern s32 D_800D31B4;
extern s32 D_800D31B8;
extern s16 D_800D31BC;
extern s16 D_800D31BE;
extern s16 D_800D31C0;
extern s16 D_800D31C2;
extern u8 D_800D3044;
extern u8 D_800D3045;
extern u8 D_800D3046;
extern u8 D_800D3047;
extern u8 D_800D3498[];

struct MenuCommand {
    u32 w0;
    u32 w1;
};

typedef struct MenuRectangle {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    u32 colour;
} MenuRectangle;

extern void func_80039380(MenuCommand **displayList, s32 count, MenuRectangle *rectangles, s32 arg3);

typedef struct MenuCurrentObject {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 index;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    s8 pad1C[4];
} MenuCurrentObject;

typedef struct MenuObjectResource {
    u8 pad0[0x4E];
    s8 unk4E;
    u8 pad4F[0x19];
    void *unk68;
} MenuObjectResource;

typedef struct MenuFrontObject {
    MenuObjectResource *resource;
    s32 unk4;
    u8 pad8[2];
    s16 indexA;
    s32 unkC[1];
} MenuFrontObject;

typedef struct MenuSpawnPacket {
    /* 0x00 */ s16 kind;
    /* 0x02 */ u8 mode;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 flags;
    /* 0x0C */ u8 unkC;
} MenuSpawnPacket;

typedef struct MenuSpawnHeader {
    /* 0x00 */ u8 pad0[0x22];
    /* 0x22 */ s8 count;
} MenuSpawnHeader;

typedef struct MenuSpawnInner {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ s16 mode;
} MenuSpawnInner;

typedef struct MenuSpawnedObject {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ MenuSpawnHeader *header;
    /* 0x44 */ u8 pad44[0x24];
    /* 0x68 */ MenuSpawnInner **inner;
} MenuSpawnedObject;

extern void *func_80034448(s32 assetId);
extern void *func_800355A0(s32 assetId, s32 arg1);
extern MenuSpawnedObject *func_8000590C(MenuSpawnPacket *packet, s32 mode);
extern void *func_8001F520(s32 assetId, s32 arg1);

typedef struct MenuDrawStack {
    u8 pad30[0x24];
    s16 sp7C;
    s16 sp7E;
    s16 sp80;
    u8 pad82[2];
    f32 sp84;
    f32 sp88;
    f32 sp8C;
    f32 sp90;
    u8 pad94[0x10];
    f32 spA4;
    u8 padA8[4];
    MenuFrontObject *spAC;
    u8 padB0[8];
} MenuDrawStack;

extern u8 D_8007C0A4;
extern u8 D_8007C0A8;
extern u8 D_8007C0AC;
extern s32 D_8007C0B4;
extern s32 D_8007C0BC;
extern u8 D_8007C07C;
extern s16 *D_8007C1B8;
extern s16 D_8007C1BC;
extern s16 D_8007C1C0;
extern MenuCommand *D_800D3140;
extern void *D_800D3144;
extern void *D_800D3148;
extern MenuFrontObject *D_800D31C8[];
extern MenuCurrentObject D_8007C1C4[];
extern MenuCurrentObject D_800D3550[];
typedef struct MenuLanguageText {
    u8 pad0[0x30];
    char *demoMessage;
} MenuLanguageText;
extern MenuLanguageText *D_8007C0B8;
extern void func_8004B0A4(s32 font);
extern void func_8004B0DC(s32 red, s32 green, s32 blue, s32 alpha);
extern void func_8004B0F8(MenuCommand **displayList, s32 x, s32 y,
                          char *text, s32 alignmentFlags);
extern void func_80009E78(MenuCommand **commands, void **matrices,
                          void **vertices, void *object);
extern void func_80023F84(MenuCommand **commands, void **matrices,
                          void **vertices, void *transform, void *object,
                          s32 arg5, s32 arg6);
extern void func_800244EC(MenuCommand **commands, void **matrices,
                          void *transform, f32 scale, f32 extra);
extern void func_8002460C(MenuCommand **commands);

#ifdef NON_MATCHING
/* Exact-size and exact-frame plateau: 14/74 words differ, first at +0x2C.
 * IDO schedules the cached-header store after its local copy, then orders the
 * destination and offset loads differently. The generated switch also binds
 * its jump table to anonymous .rodata instead of the target's named symbol. */
void func_80038750(s32 language) {
    s32 *header;
    s32 *offsets;
    s32 assetIndex;
    s32 destination;
    s32 end;
    s32 index;

    header = D_8007C094;
    if (header == NULL) {
        header = (s32 *) piRomLoad(7);
        D_8007C094 = header;
    }
    assetIndex = 1;
    switch (language) {
        case 1:
            assetIndex = 6;
            break;
        case 2:
            assetIndex = 5;
            break;
        case 3:
            assetIndex = 4;
            break;
        case 4:
            assetIndex = 3;
            break;
        case 5:
            assetIndex = 2;
            break;
        default:
            assetIndex = 1;
            break;
    }
    offsets = (s32 *) ((u8 *) header + (assetIndex * 4));
    destination = (s32) D_8007C0B8;
    end = offsets[1];
    assetIndex = offsets[0];
    if (destination != 0) {
        end -= assetIndex;
        piRomLoadSection(6, destination, assetIndex, end);
        index = 0;
        while (index < D_8007C094[0]) {
            if (((s32 *) D_8007C0B8)[index] == -1) {
                ((s32 *) D_8007C0B8)[index] = 0;
            } else {
                ((s32 *) D_8007C0B8)[index] += (s32) D_8007C0B8;
            }
            index++;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038750.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: role and structure compared with JFG's public
 * src/menu.c::initFront; JFG retains assembly, so this body is Mickey-derived.
 * With -Wo,-loopunroll,0 it has the exact 0x154 size and 0x18 frame, but
 * 66/85 words differ, first at +0x14. IDO reuses the D_800D3150 address where
 * the target rematerializes it, then diverges in later address/store schedules. */
void func_80038878(void) {
    s32 *buffer;
    s32 *bufferEnd;
    s32 value;
    s32 nextValue;
    u8 *loaded;
    u8 *loadedEnd;

    D_800D3150[0] = (s32) func_8002B280(0x5B8, 0x8F);
    buffer = D_800D3150;
    bufferEnd = D_800D3168;
    value = buffer[-1];
    do {
        buffer++;
        nextValue = value + 0xF4;
        value = nextValue;
        buffer[-1] = nextValue;
    } while ((u32) buffer < (u32) bufferEnd);
    D_8007C0B8 = func_8002B280(0x1000, 0x8F);
    func_80038750(0);
    buffer = (s32 *) D_800D31C8;
    bufferEnd = (s32 *) D_800D3498;
    do {
        buffer += 4;
        buffer[-4] = 0;
        buffer[-3] = 0;
        buffer[-2] = 0;
        buffer[-1] = 0;
    } while (buffer != bufferEnd);
    D_8007C088 = 0;
    D_8007C1B8 = (s16 *) piRomLoad(0x1A);
    D_8007C1BC = 0;
    if (D_8007C1B8[D_8007C1BC] != -1) {
        do {
            D_8007C1BC++;
        } while (D_8007C1B8[D_8007C1BC] != -1);
    }
    D_8007C1C0 = 0;
    if (D_8007C1BC > 0) {
        loaded = D_800D3498;
        loadedEnd = loaded + D_8007C1BC;
        do {
            *loaded++ = 0;
        } while ((u32) loaded < (u32) loadedEnd);
    }
    D_800D3044 = 2;
    D_800D3045 = 2;
    D_800D3046 = 2;
    D_800D3047 = 2;
    D_800D31B0 = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038878.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontFreeMode;
 * Mickey supplies the smaller 19-mode switch and exact resident state. All
 * 126 executable words are exact. Promotion is blocked by the shared 0x81590
 * rodata slice: IDO emits an anonymous 0x4C-byte table while the target's
 * named copy remains canonical; relocation metadata first differs at +0x2C. */
void func_800389CC(void) {
    u8 *selection;
    u8 value;

    if (D_8007C088 != 0) {
        selection = func_80028F54();
        switch (D_8007C0A0) {
        case 0:
            func_8004BF64(1);
            break;
        case 1:
            break;
        case 2:
            TrapDanglingJump();
            break;
        case 6:
            TrapDanglingJump();
            break;
        case 3:
            TrapDanglingJump();
            break;
        case 4:
        case 10:
        case 12:
        case 17:
        case 18:
            TrapDanglingJump();
            break;
        case 9:
        case 11:
            TrapDanglingJump();
            break;
        case 7:
            TrapDanglingJump();
            break;
        case 8:
            TrapDanglingJump();
            break;
        case 5:
            value = *selection;
            if ((value == 5) || (value == 6)) {
                if (D_8007BEF4 == 1) {
                    TrapDanglingJump();
                } else if (D_8007BEF4 < 3) {
                    TrapDanglingJump();
                } else {
                    TrapDanglingJump();
                }
            } else {
                if (D_8007BEF4 == 1) {
                    TrapDanglingJump();
                } else if (D_8007BEF4 < 3) {
                    TrapDanglingJump();
                } else {
                    TrapDanglingJump();
                }
            }
            TrapDanglingJump();
            break;
        case 13:
            TrapDanglingJump();
            break;
        case 14:
            TrapDanglingJump();
            break;
        case 15:
            TrapDanglingJump();
            break;
        case 16:
            TrapDanglingJump();
            break;
        }
        if (D_8007C1A4 != 0) {
            amSndStop(D_8007C1A4);
        }
        D_8007C1AC = 0;
        D_8007C088 = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_800389CC.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: role and switch ordering compared with JFG's public
 * src/menu.c::frontInitMode; JFG retains assembly, and this body is derived
 * from Mickey's state and call surface. All 122 executable words are exact.
 * Promotion is blocked by the shared 0x81590 rodata slice: IDO emits an
 * anonymous 0x4C-byte table while the target's named copy remains canonical;
 * relocation metadata first differs at function +0x2C. */
void func_80038BC4(void) {
    u8 *selection;
    u8 value;

    if (D_8007C088 == 0) {
        selection = func_80028F54();
        switch (D_8007C0A0) {
        case 0:
        case 1:
            break;
        case 2:
            TrapDanglingJump();
            break;
        case 6:
            TrapDanglingJump();
            break;
        case 3:
            D_800D31B0 = 1;
            TrapDanglingJump();
            break;
        case 4:
        case 10:
        case 12:
        case 17:
        case 18:
            TrapDanglingJump();
            break;
        case 9:
        case 11:
            TrapDanglingJump();
            break;
        case 7:
            TrapDanglingJump();
            break;
        case 8:
            TrapDanglingJump();
            break;
        case 5:
            value = *selection;
            if ((value == 5) || (value == 6)) {
                switch (D_8007BEF4) {
                case 1:
                    TrapDanglingJump();
                    break;
                case 2:
                    TrapDanglingJump();
                    break;
                default:
                    TrapDanglingJump();
                    break;
                }
            } else {
                switch (D_8007BEF4) {
                case 1:
                    TrapDanglingJump();
                    break;
                case 2:
                    TrapDanglingJump();
                    break;
                default:
                    TrapDanglingJump();
                    break;
                }
            }
            TrapDanglingJump();
            break;
        case 13:
            TrapDanglingJump();
            break;
        case 14:
            TrapDanglingJump();
            break;
        case 15:
            TrapDanglingJump();
            break;
        case 16:
            TrapDanglingJump();
            break;
        }
        D_8007C088 = 1;
        D_8007BF70 = -1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038BC4.s")
#endif
/* PROVENANCE: name, role, call order, and state resets compared with JFG's
 * public src/menu.c::frontSetMode; Mickey supplies the exact state surface. */
void frontSetMode(s32 mode) {
    func_800389CC();
    D_8007C0A0 = mode;
    func_80038BC4();
    func_8003968C();
    D_8007BF30 = 0;
    D_8007BF34 = 1;
    if (mode == 0) {
        D_8007BEF8 = 1;
        D_8007BEF4 = 1;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetMode. */
u8 frontGetMode(void) {
    return D_8007C0A0;
}
#ifdef NON_MATCHING
/* Size- and frame-exact plateau: 248/279 words differ, first at +0x24.
 * IDO assigns the persistent fade-state address to a0 instead of v1, then
 * cascades into a different register and switch schedule. */
s32 func_80038E1C(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3, s32 updateRate) {
    s32 sp24;
    u8 *sp20;
    u16 sp1E;
    u16 sp1C;
    u16 sp1A;
    s32 temp_t1;
    s32 temp_t4;
    s32 temp_v0;
    u8 temp_v0_2;

    sp20 = func_80028F54();
    if (D_800C947C != 0) {
        if (D_8007C1A4 == 0) {
            amSndPlay(0xA, &D_8007C1A4);
        }
        D_8007C1AC = 0x7F;
    } else if (D_8007C1AC > 0) {
        temp_t1 = D_8007C1AC - (updateRate * 3);
        D_8007C1AC = temp_t1;
        if (temp_t1 <= 0) {
            if (D_8007C1A4 != 0) {
                amSndStop(D_8007C1A4);
            }
        } else {
            amSndSetVol(0xA, D_8007C1A4, temp_t1 & 0xFF, &D_8007C1A4);
        }
    }
    func_80039720(updateRate);
    if (TrapDanglingJump() != 0) {
    } else {
        if (func_8003A550() != 0) {
            func_8003A544(0);
            temp_t4 = D_8007C09C - updateRate;
            D_8007C09C = temp_t4;
            if ((temp_t4 < 0) ||
                (sp1A = joyGetPressed(2), sp1C = joyGetPressed(1),
                 sp1E = joyGetPressed(0),
                 ((joyGetPressed(3) | sp1E | sp1C | sp1A) & 0x9000) != 0)) {
                mainTitlePageInit(1);
                D_8007C09C = 0x4B0;
            } else {
                func_8003A544(1);
            }
        }
        temp_v0 = func_80005820(0);
        D_800D3140 = *arg0;
        D_800D3144 = *arg1;
        D_800D3148 = *arg2;
        sp24 = temp_v0;
        D_800D314C = *arg3;
        func_80044BC8(D_800D3140, D_800826C0, 0x297);
        switch (D_8007C0A0) {
        case 2:
            TrapDanglingJump(updateRate);
            break;
        case 6:
            TrapDanglingJump(updateRate);
            break;
        case 3:
            TrapDanglingJump(updateRate);
            break;
        case 4:
        case 10:
        case 12:
        case 17:
        case 18:
            TrapDanglingJump(updateRate);
            break;
        case 9:
        case 11:
            TrapDanglingJump(updateRate);
            break;
        case 7:
            TrapDanglingJump(updateRate);
            break;
        case 8:
            TrapDanglingJump(updateRate);
            break;
        case 5:
            temp_v0_2 = *sp20;
            if ((temp_v0_2 == 5) || (temp_v0_2 == 6)) {
                if (D_8007BEF4 == 1) {
                    TrapDanglingJump(sp24, updateRate);
                    TrapDanglingJump(updateRate);
                } else if (D_8007BEF4 < 3) {
                    TrapDanglingJump(updateRate);
                } else {
                    TrapDanglingJump(updateRate);
                }
            } else if (D_8007BEF4 == 1) {
                TrapDanglingJump(sp24, updateRate);
                TrapDanglingJump(updateRate);
            } else if (D_8007BEF4 < 3) {
                TrapDanglingJump(updateRate);
            } else {
                TrapDanglingJump(updateRate);
            }
            break;
        case 13:
            TrapDanglingJump(updateRate);
            break;
        case 14:
            TrapDanglingJump(updateRate);
            break;
        case 15:
            TrapDanglingJump((s32)&D_800D3140, (s32)&D_800D3144, &D_800D3148, updateRate);
            break;
        case 16:
            TrapDanglingJump(updateRate);
            break;
        }
        func_80044BC8(D_800D3140, D_800826C0, 0x2C5);
        *arg0 = D_800D3140;
        *arg1 = D_800D3144;
        *arg2 = D_800D3148;
        *arg3 = D_800D314C;
        D_8007BF34 = 0;
        if (D_8007BF70 != -1) {
            D_8007BF70 -= updateRate;
            if (D_8007BF70 <= 0) {
                D_8007BF70 = -1;
                func_80000510(D_800D3050, -1);
            }
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038E1C.s")
#endif
/* PROVENANCE: name and ordered role compared with JFG's public decomp,
 * src/menu.c::frontDemoMessage; the body is derived from Mickey. */
void frontDemoMessage(MenuCommand **displayList, s32 updateRate) {
    s32 x;
    s32 y;

    D_8007C07C += updateRate;
    if (D_8007C07C & 0x10) {
        x = 0xA0;
        y = 0xD0;
        viConvertXY(&x, &y);
        func_80038750(frontGetLanguage());
        func_8004B0A4(2);
        func_8004B0DC(0, 0, 0, 0);
        fontColour(0, 0, 0, 0xFF, 0xFF);
        func_8004B0F8(displayList, x + 1, y + 1,
                      D_8007C0B8->demoMessage, 0xC);
        fontColour(0xFF, 0xFF, 0xFF, 0, 0xFF);
        func_8004B0F8(displayList, x, y, D_8007C0B8->demoMessage, 0xC);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039380.s")
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontDrawRectangle; body and rectangle layout derived from Mickey. */
void frontDrawRectangle(MenuCommand **displayList, s32 left, s32 top, s32 right, s32 bottom, u32 colour) {
    MenuRectangle rectangle;

    rectangle.left = left;
    rectangle.top = top;
    rectangle.right = right;
    rectangle.bottom = bottom;
    rectangle.colour = colour;
    func_80039380(displayList, 1, &rectangle, 1);
}
/* PROVENANCE: name and screen-limit role compared with JFG's public decomp,
 * src/menu.c::frontPlayerScreenLimits; body and table indexing derived from Mickey. */
void frontPlayerScreenLimits(s32 player, s32 *left, s32 *top, s32 *right, s32 *bottom) {
    s32 offset;
    s16 *limits;

    offset = (((D_8007BEF4 - 1) << 2) + player) << 2;
    if (D_8007BEF4 == 2 && frontGet2PlayerSplit()) {
        offset += 0x40;
    }
    limits = &D_8007BF7C[offset];
    *left = limits[0];
    *top = limits[1];
    viConvertXY(left, top);
    *right = limits[2];
    *bottom = limits[3];
    viConvertXY(right, bottom);
}
#ifdef NON_MATCHING
/* Instruction words and linked bytes are exact, but 18 relocation sites
 * differ from +0x24: IDO binds unrolled elements to three array bases while
 * the target binds each element's BSS symbol separately. */
void func_8003968C(void) {
    s32 controller;

    for (controller = 0; controller < 4; controller++) {
        D_800D31A0[controller] = -1;
        D_800D3198[controller] = 0x14;
        D_800D319C[controller] = 0xF;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003968C.s")
#endif
void func_80039720(s32 updateRate) {
    s32 controller;
    s8 repeatXNegative;
    s8 repeatXPositive;
    s8 repeatYNegative;
    s8 repeatYPositive;
    s8 axisX;
    s8 axisY;

    D_800D31B8 = 0;
    D_800D31B4 = 0;
    D_800D31C0 = 0;
    D_800D31C2 = 0;
    D_800D31BC = 0;
    D_800D31BE = 0;
    for (controller = 0; controller != 4; controller++) {
        D_800D3170[controller] = joyGetButtons(controller);
        D_800D3180[controller] =
            D_800D3170[controller] & ~D_800D31A0[controller];
        D_800D31A0[controller] = D_800D3170[controller];
        D_800D3190[controller] = joyGetStickX(controller);
        axisX = D_800D3190[controller];
        if (axisX < -0x23) {
            repeatXNegative = D_800D3198[controller];
            if (repeatXNegative < 0) {
                D_800D3198[controller] = repeatXNegative + updateRate;
                if (D_800D3198[controller] >= 0) {
                    D_800D3198[controller] = -0xF;
                } else {
                    D_800D3190[controller] = 0;
                }
            } else {
                D_800D3198[controller] = -0x14;
            }
        } else if (axisX >= 0x24) {
            repeatXPositive = D_800D3198[controller];
            if (repeatXPositive > 0) {
                D_800D3198[controller] = repeatXPositive - updateRate;
                if (D_800D3198[controller] <= 0) {
                    D_800D3198[controller] = 0xF;
                } else {
                    D_800D3190[controller] = 0;
                }
            } else {
                D_800D3198[controller] = 0x14;
            }
        } else {
            D_800D3190[controller] = 0;
            D_800D3198[controller] = 0;
        }
        D_800D3194[controller] = joyGetStickY(controller);
        axisY = D_800D3194[controller];
        if (axisY < -0x23) {
            repeatYNegative = D_800D319C[controller];
            if (repeatYNegative < 0) {
                D_800D319C[controller] = repeatYNegative + updateRate;
                if (D_800D319C[controller] >= 0) {
                    D_800D319C[controller] = -0xF;
                } else {
                    D_800D3194[controller] = 0;
                }
            } else {
                D_800D319C[controller] = -0x14;
            }
        } else if (axisY >= 0x24) {
            repeatYPositive = D_800D319C[controller];
            if (repeatYPositive > 0) {
                D_800D319C[controller] = repeatYPositive - updateRate;
                if (D_800D319C[controller] <= 0) {
                    D_800D319C[controller] = 0xF;
                } else {
                    D_800D3194[controller] = 0;
                }
            } else {
                D_800D319C[controller] = 0x14;
            }
        } else {
            D_800D3194[controller] = 0;
            D_800D319C[controller] = 0;
        }
        if (D_800D31B0 & (1 << controller)) {
            D_800D31B8 |= joyGetPressed(controller);
            D_800D31B4 |= joyGetButtons(controller);
            D_800D31C0 += joyGetStickX(controller);
            D_800D31C2 += joyGetStickY(controller);
            D_800D31BC += D_800D3190[controller];
            D_800D31BE += D_800D3194[controller];
        }
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_assetgroup_free;
 * JFG's public src/menu.c supplies the freeFrontEndList role and order. */
#pragma weak func_80039A40 = freeFrontEndList
void freeFrontEndList(s16 *assetGroup) {
    s32 index = 0;

    while (assetGroup[index] != -1) {
        func_80039A9C(assetGroup[index++]);
    }
}
/* PROVENANCE: name and ordered role compared with JFG's public decomp,
 * src/menu.c::freeFrontEndItem; the body is derived from Mickey. */
#pragma weak func_80039A9C = freeFrontEndItem
void freeFrontEndItem(s32 assetId) {
    if (D_800D3498[assetId] != 0) {
        if (((s32 *) D_800D31C8)[assetId] != 0) {
            if (((D_8007C1B8[assetId] & 0xC000) == 0xC000) &&
                (((s32 *) D_800D31C8)[assetId] != 0)) {
                func_800347A0((void *) ((s32 *) D_800D31C8)[assetId]);
            } else if (D_8007C1B8[assetId] & 0x8000) {
                func_800359D4((void *) ((s32 *) D_800D31C8)[assetId]);
            } else if (D_8007C1B8[assetId] & 0x4000) {
                func_80006EA0((void *) ((s32 *) D_800D31C8)[assetId]);
            } else {
                modFreeModel((void *) ((s32 *) D_800D31C8)[assetId]);
            }
        }
        ((s32 *) D_800D31C8)[assetId] = 0;
        D_800D3498[assetId] = 0;
        D_8007C1C0--;
        func_80006FA0();
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_assetgroup_load;
 * JFG's public src/menu.c supplies the loadFrontEndList role and order. */
void loadFrontEndList(s16 *assetGroup) {
    s32 index = 0;

    while (assetGroup[index] != -1) {
        func_80039BE4(assetGroup[index++]);
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_asset_load;
 * JFG's public src/menu.c::loadFrontEndItem and exact-size assembly skeleton
 * supply the role and order. Mickey supplies the packet and object offsets. */
#pragma weak func_80039BE4 = loadFrontEndItem
void loadFrontEndItem(s32 assetId) {
    s16 resourceId;
    MenuSpawnPacket packet;
    MenuSpawnedObject *object;

    if (D_800D3498[assetId] == 0) {
        resourceId = D_8007C1B8[assetId];
        if ((resourceId & 0xC000) == 0xC000) {
            D_800D31C8[assetId] = func_80034448(resourceId & 0x3FFF);
        } else if (resourceId & 0x8000) {
            D_800D31C8[assetId] = func_800355A0(resourceId & 0x3FFF, 0);
        } else if (resourceId & 0x4000) {
            packet.kind = resourceId & 0x3FFF;
            packet.mode = 0xA;
            packet.x = 0;
            packet.y = 0;
            packet.z = 0;
            packet.unkC = 0;
            packet.flags = 0x40;
            packet.unkA = 0;
            object = func_8000590C(&packet, 0);
            if (object->header->count > 0) {
                MenuSpawnInner *inner;

                inner = *object->inner;
                inner->mode = 2;
            }
            D_800D31C8[assetId] = (MenuFrontObject *) object;
        } else {
            D_800D31C8[assetId] = func_8001F520(resourceId & 0x3FFF, 0);
        }
        D_800D3498[assetId] = 1;
        D_8007C1C0++;
    }
}
/* PROVENANCE: body adapted from DKR's public src/menu.c::menu_imagegroup_load;
 * JFG's public src/menu.c supplies the setupFrontEndList role and order. */
void setupFrontEndList(s16 *objectGroup) {
    s32 index = 0;

    while (objectGroup[index] != -1) {
        setupFrontEndObject(objectGroup[index++]);
    }
}
/* PROVENANCE: name and ordered role compared with JFG's public decomp,
 * src/menu.c::setupFrontEndObject; record layout and body derived from Mickey. */
void setupFrontEndObject(s32 objectId) {
    MenuCurrentObject *destination;
    MenuCurrentObject *source;

    destination = &D_800D3550[objectId];
    source = &D_8007C1C4[objectId];
    destination->unk0 = source->unk0;
    destination->unk2 = source->unk2;
    destination->unk4 = source->unk4;
    destination->index = source->index;
    destination->unkC = source->unkC;
    destination->unk10 = source->unk10;
    destination->unk14 = source->unk14;
    destination->unk8 = source->unk8;
    destination->unk18 = source->unk18;
    destination->pad1C[0] = source->pad1C[0];
    destination->pad1C[1] = source->pad1C[1];
    destination->pad1C[2] = source->pad1C[2];
    destination->pad1C[3] = source->pad1C[3];
}
#ifdef NON_MATCHING
/* Exact 0xB8 frame and local stack homes, but one word too long; 242/262
 * words differ, first at +0x14. IDO assigns the D_800D31C8 base/object pair
 * to t2/a3 instead of t5/t0, cascading through the function. */
void func_80039E34(s32 index) {
    MenuDrawStack stack;
    s16 flags;
    MenuFrontObject *entryObject;
    MenuFrontObject *renderObject;
    MenuCurrentObject *current;
    MenuCommand *command;

    current = &D_800D3550[index];
    entryObject = D_800D31C8[current->index];
    if ((entryObject != NULL) &&
        ((D_8007C1B8[current->index] & 0xC000) != 0xC000)) {
        stack.sp7C = current->unk0;
        stack.sp7E = current->unk2;
        stack.sp80 = current->unk4;
        stack.sp88 = current->unkC;
        stack.sp8C = current->unk10;
        stack.sp90 = current->unk14;
        stack.sp84 = current->unk8;
        flags = D_8007C1B8[current->index];
        if (flags & 0x4000) {
            MenuCurrentObject *drawObject = (MenuCurrentObject *)entryObject;

            drawObject->unk0 = current->unk0;
            drawObject->unk2 = current->unk2;
            drawObject->unk4 = current->unk4;
            drawObject->unkC = current->unkC;
            drawObject->unk10 = current->unk10;
            drawObject->unk14 = current->unk14;
            drawObject->unk8 = current->unk8;
            *((s8 *)entryObject + 0x39) = D_8007C0BC;
            func_80009E78(&D_800D3140, &D_800D3144, &D_800D3148,
                          entryObject);
            return;
        }
        if (flags & 0x8000) {
            stack.spA4 = current->unk18;
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w0 = 0xE7000000;
            command->w1 = 0;
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w0 = 0xFA000000;
            command->w1 = (D_8007C0A4 << 24) | (D_8007C0A8 << 16) |
                          (D_8007C0AC << 8) | (D_8007C0BC & 0xFF);
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w1 = -0x100;
            command->w0 = 0xFB000000;
            func_80023F84(&D_800D3140, &D_800D3144, &D_800D3148,
                          &stack.sp7C,
                          D_800D31C8[current->index], D_8007C0B4,
                          D_8007C0BC);
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w1 = 0;
            command->w0 = 0xE7000000;
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w1 = -1;
            command->w0 = 0xFA000000;
            return;
        }
        command = D_800D3140;
        D_800D3140 = command + 1;
        command->w1 = 0;
        command->w0 = 0xE7000000;
        if (D_8007C0BC < 0xFF) {
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w0 = 0xFA000000;
            command->w1 = (D_8007C0BC & 0xFF) | ~0xFF;
        } else {
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w1 = -1;
            command->w0 = 0xFA000000;
        }
        command = D_800D3140;
        D_800D3140 = command + 1;
        command->w1 = -0x100;
        command->w0 = 0xFB000000;
        stack.spA4 = current->unk18 * 0.0625f;
        renderObject = D_800D31C8[current->index];
        if (renderObject->resource->unk4E == 0) {
            stack.spAC = renderObject;
            func_800244EC(&D_800D3140, &D_800D3144, &stack.sp7C, 1.0f,
                          0.0f);
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w0 = (((stack.spAC->unkC[stack.spAC->indexA] +
                            0x80000000) &
                            0xFFFFFF) | 0xBF000000);
            command->w1 = stack.spAC->unk4 + 0x80000000;
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w0 = 0x06000000;
            command->w1 = (s32)stack.spAC->resource->unk68 + 0x80000000;
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w1 = 0;
            command->w0 = 0xBF000000;
            func_8002460C(&D_800D3140);
        }
        if (D_8007C0BC < 0xFF) {
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w1 = 0;
            command->w0 = 0xE7000000;
            command = D_800D3140;
            D_800D3140 = command + 1;
            command->w1 = -1;
            command->w0 = 0xFA000000;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039E34.s")
#endif
#pragma weak func_8003A24C = frontGetLanguage
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontGetLanguage; body derived from Mickey. */
s32 frontGetLanguage(void) {
    return (u32)D_800D312A >> 10;
}
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontSetLanguage; body and bitfield derived from Mickey. */
void frontSetLanguage(s32 language) {
    D_800D3128.language = language;
    func_80038750(language);
}
s32 frontGetScreenMode(void) {
    s32 mode;

    mode = 0;
    if (D_800D3128.modeBit0) {
        mode = 1;
    }
    if (D_800D3128.modeBit1) {
        mode |= 2;
    }
    return mode;
}
#ifdef NON_MATCHING
/* Size-exact plateau: 19/32 words differ from +0xC, all in register operands;
 * IDO does not retain the mode-state address and normalized mode in a1/v0/v1.
 * PROVENANCE: mask, state-change guard, and order compared with JFG's public
 * src/menu.c::frontSetScreenMode; packed fields derived from Mickey. */
void func_8003A2C8(s32 screenMode) {
    u8 *modeState;
    s32 mode;
    u8 modeBits;

    modeState = &D_8007C090;
    mode = (modeBits = screenMode & 3);
    if (*modeState != mode) {
        D_8007C090 = screenMode & 3;
        if (modeBits & (1 ^ 0)) {
            D_800D3128.modeBit0 = 1;
        } else {
            D_800D3128.modeBit0 = 0 & 0xFFFFFFFFFFFFFFFFu;
        }
        if (modeBits & 2) {
            D_800D3128.modeBit1 = 1;
        } else {
            D_800D3128.modeBit1 = 0;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A2C8.s")
#endif
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontStoreScreenMode. */
void frontStoreScreenMode(void) {
    D_8007C08C = D_8007C090;
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontRecallScreenMode. */
u8 frontRecallScreenMode(void) {
    return D_8007C08C;
}
s32 frontGetLevelScreenMode(void) {
    /* Mickey-derived control flow; JFG's body remains GLOBAL_ASM. */
    if (D_8007C090 == 1) {
        goto mode_one;
    }
    if (D_8007C090 == 2) {
        goto mode_two;
    }
    if (D_8007C090 != 3) {
        goto current_mode;
    }

    return 3;
mode_two:
    return levelGetRegionNo() | 2;
mode_one:
    return 1;
current_mode:
    return levelGetRegionNo();
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetWideAdjust. */
s8 frontGetWideAdjust(void) {
    return D_800D312B;
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetWideAdjust. */
void frontSetWideAdjust(s32 offset) {
    viSetWideAdjust(offset);
    D_800D312B = viGetWideAdjust();
}
/* PROVENANCE: name and order compared with JFG's public decomp,
 * src/menu.c::frontGetStereoMode; body and bitfield derived from Mickey. */
u32 frontGetStereoMode(void) {
    return D_800D3128.stereoMode;
}
/* PROVENANCE: name, clamp, table lookup, and order compared with JFG's public
 * src/menu.c::frontSetStereoMode; packed storage derived from Mickey. */
void frontSetStereoMode(s32 mode) {
    if (mode < 0) {
        mode = 0;
    }
    if (mode >= 4) {
        mode = 3;
    }
    D_800D3128.stereoMode = mode;
    alSurround_OutputType(D_8007C308[mode]);
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A47C = frontGetSfxVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetSfxVolume. */
u16 frontGetSfxVolume(void) {
    return D_800D312C;
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A488 = frontSetSfxVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetSfxVolume. */
void frontSetSfxVolume(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_800D312C = volume;
    gsSndpSetGlobalVolume(volume);
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A4C4 = frontGetBgmVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetBgmVolume. */
u16 frontGetBgmVolume(void) {
    return D_800D312E;
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A4D0 = frontSetBgmVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetBgmVolume. */
void frontSetBgmVolume(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_800D312E = volume;
    amTuneSetGlobalVolume(volume);
}
s32 frontGet2PlayerSplit(void) {
    s32 split;

    split = D_800D3128.twoPlayerSplit;
    return split;
}
/* Size-exact plateau: three register operands differ from +0x8; IDO assigns
 * the old-flag value chain two temporary registers earlier than the target. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A520.s")
void func_8003A544(s32 value) {
    D_8007C098 = value;
}
s32 func_8003A550(void) {
    return D_8007C098;
}
void func_8003A55C(s32 value) {
    amTuneStop();
    D_800D3050 = value;
    D_8007BF70 = 0x78;
}
void func_8003A590(void) {
    D_8007BF70 = -1;
}
