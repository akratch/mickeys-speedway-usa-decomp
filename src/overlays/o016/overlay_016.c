#include "overlays/overlay_016.h"

/*
 * JFG overlay 27 has the unique full 0x8C-byte object match for this routine;
 * pinned DKR v77/v80 scans are negative. JFG retains it as assembly, so this
 * C reconstruction follows the shared instruction semantics conservatively.
 */
s32 overlay16BuildGradient(s8 *output, s32 first0, s32 first1, s32 first2,
                           s32 last0, s32 last1, s32 last2) {
    s32 remaining;

    last0 -= first0;
    last1 -= first1;
    last2 -= first2;
    first0 <<= 6;
    first1 <<= 6;
    /* The comma form preserves the retail compiler's v0/v1 loop coloring. */
    first2 <<= (0, 6);
    remaining = 0x3F;
    do {
        output[0] = first0 >> 6;
        output[1] = first1 >> 6;
        output[2] = first2 >> 6;
        first0 += last0;
        first1 += last1;
        first2 += last2;
        output += 3;
    } while (remaining--);
}

/* DKR v77/v80 have no donor; JFG only confirms the gradient helper. */
void overlay16InitializeBuffer(u8 *config) {
    gOverlay16Buffer = overlay16AllocateReloc(0x300, 0x87);
    if (gOverlay16Buffer != NULL) {
        overlay16BuildGradientReloc(gOverlay16Buffer, config[0x102], config[0x103],
                                    config[0x104], config[0xFF], config[0x100],
                                    config[0x101]);
        overlay16BuildGradientReloc(gOverlay16Buffer + 0xC0, config[0xFF], config[0x100],
                                    config[0x101], config[0xFC], config[0xFD],
                                    config[0xFE]);
        overlay16BuildGradientReloc(gOverlay16Buffer + 0x180, config[0xFC], config[0xFD],
                                    config[0xFE], config[0xFF], config[0x100],
                                    config[0x101]);
        overlay16BuildGradientReloc(gOverlay16Buffer + 0x240, config[0xFF], config[0x100],
                                    config[0x101], config[0x102], config[0x103],
                                    config[0x104]);
        gOverlay16Phase = 0;
        gOverlay16Mode = config[0xFB];
    }
}

/* DKR v77/v80 and JFG have no donor for this ownership wrapper. */
void overlay16ReleaseBuffer(void) {
    if (gOverlay16Buffer != NULL) {
        overlay16ReleaseReloc(gOverlay16Buffer);
        gOverlay16Buffer = NULL;
    }
}

/* DKR v77/v80 and JFG contain no exact donor for this gradient pass. */
/* Plateau (2026-08-25): exact-size 0x244, 64 register-only words differ,
 * first +0x3C; sharing the block/vertex counter closed opcode and schedule.
 * The flag lattice was neutral; the 40-minute permuter bottomed out at 345. */
#ifdef NON_MATCHING
void overlay16ApplyGradient(s32 *active, Overlay16Context *context,
                            s32 phaseStep) {
    Overlay16Batch *batch;
    Overlay16ColorSource *source;
    Overlay16Vertex *vertex;
    s32 *activePtr;
    u8 *input;
    u8 *gradient;
    u8 inputBlue;
    u8 gradientRed;
    u8 gradientGreen;
    u8 gradientBlue;
    u8 inputRed;
    u8 inputGreen;
    s32 batchIndex;
    s32 gradientIndex;
    s32 blocks;
    s32 phase;
    s32 one;

    activePtr = active;
    gradient = gOverlay16Buffer;
    if (gradient == NULL) {
        return;
    }
    phase = (gOverlay16Phase + phaseStep) & 0xFF;
    gOverlay16Phase = phase;
    batch = context->batches;
    batchIndex = context->batchCount;
    one = 1;
    while (batchIndex--) {
        if (*activePtr++) {
            source = batch->source;
            if (source) {
                s16 *block;
                block = source->blocks;
                blocks = (batch->vertexCount + 0xF) >> 4;
                while (blocks--) {
                    *block++ = 0;
                }
                input = source->colors;
                vertex = batch->vertices;
                blocks = batch->vertexCount;
                if (gOverlay16Mode == one) {
                    while (blocks--) {
                        u8 *gradientColor;
                        gradientIndex =
                            (vertex->z + vertex->x + vertex->y + phase) & 0xFF;
                        gradientColor = gradient + gradientIndex;
                        gradientColor += gradientIndex;
                        gradientColor += gradientIndex;
                        vertex->red = (input[0] * gradientColor[0]) >> 8;
                        vertex->green = (input[1] * gradientColor[1]) >> 8;
                        vertex->blue = (input[2] * gradientColor[2]) >> 8;
                        input += 3;
                        vertex++;
                    }
                } else {
                    while (blocks--) {
                        u8 *gradientColor;
                        gradientIndex =
                            (vertex->z + vertex->x + vertex->y + phase) & 0xFF;
                        gradientColor = gradient + gradientIndex;
                        gradientColor += gradientIndex;
                        gradientColor += gradientIndex;
                        inputRed = input[0];
                        gradientRed = gradientColor[0];
                        gradientGreen = gradientColor[1];
                        gradientBlue = gradientColor[2];
                        inputGreen = input[1];
                        inputBlue = input[2];
                        if (gradientRed < inputRed) {
                            vertex->red = inputRed;
                        } else {
                            vertex->red = gradientRed;
                        }
                        if (gradientGreen < inputGreen) {
                            vertex->green = inputGreen;
                        } else {
                            vertex->green = gradientGreen;
                        }
                        if (gradientBlue < inputBlue) {
                            vertex->blue = inputBlue;
                        } else {
                            vertex->blue = gradientBlue;
                        }
                        input += 3;
                        vertex++;
                    }
                }
            }
            batch->dirty = 0;
        }
        batch++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o016/overlay_016/func_overlay_016_F00001E0_1873678.s")
#endif
