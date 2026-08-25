#include "overlays/overlay_045.h"
#include "game/font.h"
#include "n_audio/mbi.h"

extern DialogueBoxBackground D_800D64E8[];

extern void func_80034920(Gfx **displayList);
extern void func_8004B0A4(s32 font);
extern void func_800221E8(Gfx **displayList, void *matrix);
extern void func_80022A50(Gfx **displayList, void *matrix);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 opacity);
extern s32 func_8002997C(s32 minimum, s32 maximum);
extern f32 func_8002A8C0(s32 angle);

extern void func_overlay_045_F0001158_188D5B0(
    Gfx **displayList, s32 arg1, s32 arg2,
    Overlay45ResourceDescriptor *descriptor);

/* Mickey-local reconstruction. The closest permitted donor skeleton scores
 * 0.047 and does not contain this descriptor/element update shape. */
/*
 * Plateau (2026-08-25, 10 attempts): -O2 -mips2 with
 * -Wab,-r4300_mul produces the target's 637 instructions with identical
 * opcode order and register allocation, but reserves a 0x90-byte frame
 * instead of retail's 0x88-byte frame. The resulting 20 positional word
 * differences begin at +0x0 and are frame/incoming-argument offsets; the
 * separate-TU constants and resident/local references also still need their
 * retail relocation bindings. Correcting element signedness, using the SDK
 * display-list macros, splitting the sequencing boolean from its delay, and
 * preserving the natural in-place updates closed all opcode and allocator
 * residuals. A two-worker ten-minute permuter run improved only the fade
 * expression; the equivalent in-place spelling is retained here. Register
 * storage hints did not change the remaining frame, so assembly stays active.
 */
#ifdef NON_MATCHING
void func_overlay_045_F0000764_188CBBC(
    Gfx **displayList, void *matrix, s32 arg2, s32 arg3,
    s32 mode, s32 updateRate) {
    Overlay45ResourceDescriptor *descriptor;
    Overlay45Element *element;
    s32 anyVisible;
    s32 remaining;
    s32 i;

    anyVisible = 0;
    func_80034920(displayList);
    func_8004B0A4(3);
    descriptor = gOverlay45ResourceHead;
    func_800221E8(displayList, NULL);
    func_80022A50(displayList, matrix);

    gDPSetScissor((*displayList)++, G_SC_NON_INTERLACE,
                  D_800D64E8[0].x1, D_800D64E8[0].y1,
                  D_800D64E8[0].x2, D_800D64E8[0].y2);

    while (descriptor != NULL) {
        func_8004B0B8(0xFF, 0xFF, 0xFF, descriptor->unk22, 0xFF);
        if ((descriptor->mode > 0) && (mode == descriptor->unk20)) {
            remaining = descriptor->count;
            element = descriptor->elements;
            while (remaining != 0) {
                if (element->unk13 != 0) {
                    element->unk1E += element->unk1F * updateRate;
                    if (element->unk20 != 0xFF00) {
                        element->unk20 += updateRate * 8;
                        if (element->unk20 >= 0x100) {
                            element->unk20 = 0xFF00;
                        }
                    }
                    if (element->unk1E < 0) {
                        element->unk1E = 0;
                        element->unk13 = 0;
                        descriptor->unk1E--;
                        element->unk20 = 0;
                    }
                    element->unk1F -= updateRate;
                }

                if (descriptor->flags & 0x40) {
                    for (i = 0; i < updateRate; i++) {
                        element->x +=
                            ((f32)element->drawX - element->x) * 0.075f;
                        element->y +=
                            ((f32)element->drawY - element->y) * 0.075f;
                    }
                } else if (descriptor->flags & 0x80) {
                    element->x += (f32)(element->offsetX * updateRate);
                    element->y += (f32)(element->offsetY * updateRate);
                    if (element->x < -32.0f) {
                        element->x = -32.0f;
                    }
                    if (element->x > 352.0f) {
                        element->x = 352.0f;
                    }
                    if (element->y < -32.0f) {
                        element->y = -32.0f;
                    }
                    if (element->y > 272.0f) {
                        element->y = 272.0f;
                    }
                }

                element->unk0C += element->unk0E * updateRate;
                if (element->unk0C >= 0x8001) {
                    element->unk0C -= 0x8000;
                    element->unk0E = func_8002997C(0x600, 0xA00);
                }

                element->unk0A += element->unk10 * updateRate;
                if (element->unk0A < -0x1000) {
                    element->unk0A = -0x1000;
                    element->unk10 = func_8002997C(0x100, 0x200);
                } else if (element->unk0A >= 0x1001) {
                    element->unk0A = 0x1000;
                    element->unk10 = -func_8002997C(0x100, 0x200);
                }

                if ((element->x > -16.0f) && (element->x < 336.0f) &&
                    (element->y > -16.0f) && (element->y < 256.0f)) {
                    anyVisible = 1;
                    if (descriptor->flags & 0x200) {
                        s32 expired;

                        expired = 0;
                        if (element->unk22 > 0) {
                            element->unk22 -= updateRate;
                            if (element->unk22 <= 0) {
                                expired = 1;
                            }
                        }
                        if ((element->unk13 == 0) &&
                            ((func_8002997C(0, 0x3E8) >= 0x3E7) || expired)) {
                            element->unk13 = 1;
                            descriptor->unk1E++;
                            element->unk1F = 0xA;
                        }
                    }
                }

                element->unk12 = (s32)(func_8002A8C0(element->unk0C) * 5.0f);
                remaining--;
                element++;
            }

            ((s16 *)descriptor->unk28)[0] = descriptor->width;
            ((s16 *)descriptor->unk28)[1] = descriptor->height;

            if (anyVisible != 0) {
                if (descriptor->flags & 0x200) {
                    if (descriptor->optionalValue > 0) {
                        descriptor->optionalValue -= updateRate;
                    } else if (descriptor->unk1E == 0) {
                        s32 direction;
                        s32 delay;
                        s32 forward;

                        element = descriptor->elements;
                        if (descriptor->unk1F > 0) {
                            forward = 1;
                        } else if (descriptor->unk1F < 0) {
                            forward = 0;
                        } else {
                            forward = func_8002997C(0, 1);
                        }
                        descriptor->unk1F = 0;
                        if (forward != 0) {
                            delay = 4;
                            direction = 4;
                        } else {
                            direction = -4;
                            delay = descriptor->count * 4;
                        }
                        for (i = 0; i < descriptor->count; i++) {
                            element->unk22 = delay;
                            delay += direction;
                            element++;
                        }
                        if (func_8002997C(0, 3) == 0) {
                            descriptor->optionalValue = 8;
                            descriptor->unk1F = -direction;
                        } else {
                            descriptor->optionalValue =
                                func_8002997C(0x78, 0x258);
                        }
                    }
                }

                if (descriptor->flags & 0x400) {
                    for (i = 0; i < updateRate; i++) {
                        descriptor->unk10 += descriptor->unk14;
                        descriptor->unk14 -= 0.003f;
                        if (descriptor->unk10 < 0.0f) {
                            descriptor->unk10 = 0.0f;
                            descriptor->unk14 = 0.05f;
                        }
                    }
                } else {
                    descriptor->unk10 *= 0.5f;
                }

                func_overlay_045_F0001158_188D5B0(
                    displayList, arg2, arg3, descriptor);
            }
        }
        descriptor = descriptor->next;
    }

    if (mode == 2) {
        gDPPipeSync((*displayList)++);
        gSPEndDisplayList((*displayList)++);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o045/func_overlay_045_F0000764_188CBBC/func_overlay_045_F0000764_188CBBC.s")
#endif
