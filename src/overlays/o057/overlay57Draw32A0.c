#include "PR/ultratypes.h"

typedef struct Overlay57Draw32A0Record {
    s32 word0;
    s32 word4;
    u32 packed8;
} Overlay57Draw32A0Record;

extern s32 gO57Draw32A0Available4CC;
extern s32 gO57Draw32A0Selection1A0;
extern s32 gO57Draw32A0State118;
extern s32 gO57Draw32A0Envelope124;
extern s32 gO57Draw32A0TableMode18C[];
extern u32 gO57Draw32A0WordTable4B8[];
extern Overlay57Draw32A0Record gO57Draw32A0Records3F8[];
extern Overlay57Draw32A0Record gO57Draw32A0Records458[];
extern Overlay57Draw32A0Record gO57Draw32A0Work34C[];
extern u32 gO57Draw32A0Masks3C8[];
extern u32 gO57Draw32A0Shifts3D8[];
extern u8 gO57Draw32A0Local08[];
extern u8 gO57Draw32A0Local18[];
extern u8 gO57Draw32A0Local28[];

extern s32 gO57Draw32A0GateReloc;
extern u8 gO57Draw32A0AnchorReloc[];
extern u8 gO57Draw32A0StripeReloc[];
extern u16 gO57Draw32A0HalfwordsReloc[];

extern s32 o57Draw32A0Call0Reloc(void);
extern s32 o57Draw32A0Call1Reloc(void);
extern s32 o57Draw32A0Call2Reloc(void);
extern void o57Draw32A0SetupReloc(void *anchor, s32 count,
                                  Overlay57Draw32A0Record *records, s32 mode);
extern void o57Draw32A0ModeReloc(s32 mode);
extern void o57Draw32A0ColorReloc(s32 red, s32 green, s32 blue, s32 alpha,
                                  s32 envelope);
extern void o57Draw32A0Stripe0Reloc(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void o57Draw32A0Stripe1Reloc(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void o57Draw32A0Stripe2Reloc(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void o57Draw32A0Stripe3Reloc(void *anchor, s32 width, s32 position,
                                    void *data, s32 count);
extern void o57Draw32A0RenderReloc(void *anchor,
                                   Overlay57Draw32A0Record *record,
                                   f32 scale, f32 position, f32 xScale,
                                   f32 yScale, s32 color, s32 command);

/* Overlay 57 text +0x32A0..+0x35E0. */
/* Plateau (batch 20): exact 0x340; 60 words remain, first +0x0 (frame -0x78/-0x70).
 * Reused rising for packed and count-first arms improved 86; 119 flags and 10 attempts failed.
 * The 40-minute -mips2 permuter's best 835 required unsupported pointer/width detours. */
#ifdef NON_MATCHING
void overlay57Draw32A0(s32 updateRate) {
    Overlay57Draw32A0Record *records;
    Overlay57Draw32A0Record *cursor;
    u32 *maskCursor;
    u32 *shiftCursor;
    s32 envelope;
    s32 rowCount[1];
    s32 position;
    s32 i;
    u32 bits;
    s32 rising;

    if (gO57Draw32A0Available4CC == 0) {
        return;
    }

    rising = 0;
    if ((gO57Draw32A0GateReloc == 1) &&
        (o57Draw32A0Call0Reloc() < 5) &&
        (o57Draw32A0Call1Reloc() != 0)) {
        gO57Draw32A0Selection1A0 = o57Draw32A0Call2Reloc();
        if (gO57Draw32A0State118 == 1) {
            rising = 1;
        }
    }

    if (rising != 0) {
        envelope = (gO57Draw32A0Envelope124 += updateRate * 8);
        if (envelope >= 0xFF) {
            envelope = rising + 0xFD;
            gO57Draw32A0Envelope124 = envelope;
        }
    } else {
        envelope = gO57Draw32A0Envelope124 - updateRate * 32;
        gO57Draw32A0Envelope124 = envelope;
        if (envelope < 0) {
            gO57Draw32A0Envelope124 = 0;
            return;
        }
    }

    if (gO57Draw32A0TableMode18C[0] != 0) {
        rowCount[0] = 4;
        records = gO57Draw32A0Records3F8;
        position = 0x3E;
    } else {
        rowCount[0] = 3;
        records = gO57Draw32A0Records458;
        position = 0x4D;
    }

    rising = gO57Draw32A0WordTable4B8[gO57Draw32A0Selection1A0] |
             ((envelope * 5) >> 3);
    i = 1;
    cursor = &records[1];
    do {
        i++;
        cursor->packed8 = rising;
        cursor++;
    } while (i < 8);

    o57Draw32A0SetupReloc(gO57Draw32A0AnchorReloc, 8, records, 0);
    o57Draw32A0ModeReloc(2);
    o57Draw32A0ColorReloc(0xFF, 0xFF, 0xFF, 0xFF,
                          gO57Draw32A0Envelope124);
    o57Draw32A0Stripe0Reloc(gO57Draw32A0AnchorReloc, 0xF8, position,
                            gO57Draw32A0StripeReloc, 4);
    position += 0x1E;
    o57Draw32A0Stripe1Reloc(gO57Draw32A0AnchorReloc, 0xF8, position,
                            gO57Draw32A0Local08, 4);
    position += 0x1E;
    o57Draw32A0Stripe2Reloc(gO57Draw32A0AnchorReloc, 0xF8, position,
                            gO57Draw32A0Local18, 4);
    position += 0x1E;
    if (gO57Draw32A0TableMode18C[0] != 0) {
        o57Draw32A0Stripe3Reloc(gO57Draw32A0AnchorReloc, 0xF8, position,
                                gO57Draw32A0Local28, 4);
    }

    i = 0;
    position -= 0x48;
    if (rowCount[0] > 0) {
        maskCursor = gO57Draw32A0Masks3C8;
        shiftCursor = gO57Draw32A0Shifts3D8;
        do {
            bits = gO57Draw32A0HalfwordsReloc[4 +
                    gO57Draw32A0Selection1A0];
            bits = (bits & *maskCursor) >> *shiftCursor;
            gO57Draw32A0Work34C[0].packed8 = bits << 16;
            o57Draw32A0RenderReloc(gO57Draw32A0AnchorReloc,
                                   gO57Draw32A0Work34C, 248.0f,
                                   (f32)position, 1.0f, 1.0f,
                                   gO57Draw32A0Envelope124 | -256, 0x2003);
            i++;
            maskCursor++;
            shiftCursor++;
            position += 0x1E;
        } while (i != rowCount[0]);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/overlay57Draw32A0/func_overlay_057_F00032A0_18A6E98.s")
#endif
