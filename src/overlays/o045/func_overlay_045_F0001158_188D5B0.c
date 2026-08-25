#include "overlays/overlay_045.h"
#include "game/font.h"
#include "n_audio/mbi.h"

typedef struct Overlay45FontData {
    u8 width;
    u8 verticalExtent;
    u8 characterWidth;
    u8 height;
    u32 romOffset;
    s16 textureSize;
    u16 format;
    u32 unused0C;
    Gfx *displayList;
} Overlay45FontData;

typedef struct Overlay45GlyphData {
    u8 font;
    u8 character;
    u16 allocationOffset;
    u8 state;
    u8 chainLength;
    u16 textureOffset;
    u16 textureOffset2;
    u8 left;
    u8 top;
    u8 right;
    u8 bottom;
    u8 advance;
} Overlay45GlyphData;

typedef struct Overlay45Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay45Vertex;

typedef struct Overlay45Triangle {
    u8 flags;
    u8 vertex0;
    u8 vertex1;
    u8 vertex2;
    s16 s0;
    s16 t0;
    s16 s1;
    s16 t1;
    s16 s2;
    s16 t2;
} Overlay45Triangle;

extern DialogueBoxBackground D_800D64E8[];
extern Gfx D_8007D490[];
extern u8 D_800D60E0;
extern Overlay45FontData *D_800D60E4;
extern u32 D_800D6638;
extern u8 *D_800D6648;
extern u8 D_800D664D;

extern void func_80034920(Gfx **displayList);
extern void func_8004D39C(char *input, char *output);
extern Overlay45GlyphData *func_8004C690(s32 character);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);

/*
 * PROVENANCE -- the control-stream and font-command organization was
 * cross-checked through Mickey's resident func_8004B1DC, whose corresponding
 * source shape was adapted from Diddy Kong Racing, src/font.c
 * (render_text_string). Overlay 45's rotation, private vertex/triangle
 * records, descriptor fields, and final control flow come from Mickey's own
 * instructions and callers.
 */
/* Workbench: structure-mismatch; 593 words differ, 681 versus 674 instructions, first mismatch +0x0.
 * Lever: constant-audit found only frame/displacement fallout; post-call pointer acquisition regressed to 639 words.
 * Remains: 24-byte frame/save-web delta and broad command/vertex structure; no source literal correction is available. */
#ifdef NON_MATCHING
void func_overlay_045_F0001158_188D5B0(
    Gfx **displayList, Overlay45Vertex **vertexPtr, void *unused,
    Overlay45ResourceDescriptor *descriptor) {
    DialogueBoxBackground *window;
    Overlay45Element *element;
    Overlay45FontData *font;
    Overlay45GlyphData *glyph;
    Overlay45Vertex *vertices;
    Overlay45Triangle *triangles;
    Gfx *dList;
    Gfx *fontCommands;
    u8 *current;
    u8 first;
    u8 second;
    s32 savedFont;
    volatile s32 activeColour;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 glyphWidth;
    s32 glyphHeight;
    s32 textureLeft;
    s32 textureTop;
    s32 textureRight;
    s32 textureBottom;
    s32 loopY;
    u32 vertexAddress;
    f32 sine;
    f32 cosine;
    f32 halfWidth;
    f32 halfHeight;
    f32 negativeHalfWidth;
    f32 negativeHalfHeight;
    f32 leftFloat;
    f32 rightFloat;
    f32 upperFloat;
    f32 lowerFloat;

    if (descriptor == NULL) {
        return;
    }

    dList = *displayList;
    window = descriptor->unk28;
    vertices = *vertexPtr;
    triangles = descriptor->allocation;
    func_8004D39C(descriptor->elementEnd, D_800D6648);
    savedFont = D_800D60E0;
    D_800D60E0 = descriptor->unk21;
    font = &D_800D60E4[D_800D60E0];
    current = D_800D6648;

    gSPDisplayList(dList++, D_8007D490);
    if (window != D_800D64E8) {
        s32 x1;
        s32 y1;
        s32 x2;
        s32 y2;

        x1 = window->x1;
        y1 = window->y1;
        x2 = window->x2;
        y2 = window->y2;
        if ((D_800D64E8[0].x2 < x1) || (D_800D64E8[0].y2 < y1) ||
            (x2 < 0) || (y2 < 0)) {
            return;
        }
        if (x1 < 0) {
            x1 = 0;
        }
        if (y1 < 0) {
            y1 = 0;
        }
        if (D_800D64E8[0].x2 < x2) {
            x2 = D_800D64E8[0].x2;
        }
        if (D_800D64E8[0].y2 < y2) {
            y2 = D_800D64E8[0].y2;
        }
        gDPSetScissor(dList++, G_SC_NON_INTERLACE, x1, y1, x2, y2);
    }

    gDPSetPrimColor(dList++, 0, 0, 0xFF, 0xFF, 0xFF, descriptor->mode);
    gDPSetEnvColor(dList++, window->textColourR, window->textColourG,
                   window->textColourB, descriptor->unk22);

    activeColour = 0;
    first = *current;
    element = descriptor->elements;
    while ((first != 0) && (window->y2 >= loopY)) {
        current++;
        if (first & 0x80) {
            second = *current++;
            if ((second != 0) && (second != 0xF)) {
                if (D_800D664D != 0) {
                    if (second == 2) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0, 0, 0xFF, 0xFF);
                        activeColour = 1;
                    } else if (second == 0xE) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0, 0xFF, 0, 0xFF);
                        activeColour = 1;
                    } else if ((second >= 0x41) && (second < 0x45)) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, 0xFF, 0xFF, 0, 0xFF);
                        activeColour = 1;
                    } else if (activeColour != 0) {
                        gDPPipeSync(dList++);
                        gDPSetEnvColor(dList++, window->textColourR,
                                       window->textColourG,
                                       window->textColourB,
                                       window->textColourA);
                        activeColour = 0;
                    }
                }

                glyph = func_8004C690(second);
                if (glyph != NULL) {
                    left = (s32)element->x;
                    top = (s32)element->y;
                    right = (glyph->right + left) - glyph->left;
                    bottom = (glyph->bottom + top) - glyph->top;
                    glyphWidth = right - left;
                    glyphHeight = bottom - top;
                    textureLeft = glyph->left << 5;
                    textureTop = glyph->top << 5;
                    textureRight = ((glyphWidth << 5) + textureLeft) - 0x20;
                    textureBottom = ((glyphHeight << 5) + textureTop) - 0x20;

                    fontCommands = font->displayList;
                    dList->words.w0 = fontCommands->words.w0;
                    dList->words.w1 = D_800D6638 + glyph->textureOffset;
                    dList++;
                    fontCommands++;
                    vertexAddress = (u32)vertices + 0x80000000;
                    if (font->format == 4) {
                        dList->words.w0 = 0x07060030;
                        dList->words.w1 = (u32)fontCommands + 0x80000000;
                        dList++;
                        dList->words.w0 = fontCommands[6].words.w0;
                        dList->words.w1 = D_800D6638 + glyph->textureOffset2;
                        dList++;
                        fontCommands += 7;
                    }
                    dList->words.w0 = 0x07080040;
                    dList->words.w1 = (u32)fontCommands + 0x80000000;
                    dList++;
                    dList->words.w0 = 0xEF082C0F;
                    dList->words.w1 = 0x00504240;
                    dList++;
                    dList->words.w0 = 0x04000030 |
                        (((((vertexAddress & 6) | 0x20) & 0xFF) << 16));
                    dList->words.w1 = vertexAddress;
                    dList++;
                    dList->words.w0 = 0x05110020;
                    dList->words.w1 = (u32)triangles + 0x80000000;
                    dList++;

                    if (descriptor->unk10 != 0.0f) {
                        f32 expandX;
                        f32 expandY;

                        expandX = descriptor->unk10 * (f32)glyphWidth * 0.5f;
                        expandY = descriptor->unk10 * (f32)glyphHeight * 0.5f;
                        left = (s32)((f32)left - expandX);
                        right = (s32)((f32)right + expandX);
                        top = (s32)((f32)top - expandY);
                        bottom = (s32)((f32)bottom + expandY);
                    }

                    upperFloat = (f32)((0x78 - top) + element->unk12 +
                                       element->unk1E);
                    lowerFloat = (f32)((0x78 - bottom) + element->unk12 +
                                       element->unk1E);
                    left -= 0xA0;
                    right -= 0xA0;
                    sine = func_8002A8BC(
                        element->unk0A + ((s16)element->unk20 << 8));
                    cosine = func_8002A8C0(
                        element->unk0A + ((s16)element->unk20 << 8));
                    halfWidth = (f32)(right - left) * 0.5f;
                    halfHeight = (upperFloat - lowerFloat) * 0.5f;
                    negativeHalfWidth = -halfWidth;
                    negativeHalfHeight = -halfHeight;
                    leftFloat = (f32)left;
                    rightFloat = (f32)right;

                    vertices[0].x = (s32)(leftFloat +
                        (((negativeHalfWidth * sine) -
                          (halfHeight * cosine)) - negativeHalfWidth));
                    vertices[0].y = (s32)(upperFloat +
                        (((halfHeight * sine) +
                          (negativeHalfWidth * cosine)) - halfHeight));
                    vertices[0].z = 0;
                    vertices[0].red = 0xFF;
                    vertices[0].green = 0xFF;
                    vertices[0].blue = 0xFF;
                    vertices[0].alpha = 0xFF;

                    vertices[1].x = (s32)(rightFloat +
                        (((halfWidth * sine) - (halfHeight * cosine)) -
                         halfWidth));
                    vertices[1].y = (s32)(upperFloat +
                        (((halfHeight * sine) + (halfWidth * cosine)) -
                         halfHeight));
                    vertices[1].z = 0;
                    vertices[1].red = 0xFF;
                    vertices[1].green = 0xFF;
                    vertices[1].blue = 0xFF;
                    vertices[1].alpha = 0xFF;

                    vertices[2].x = (s32)(leftFloat +
                        (((negativeHalfWidth * sine) -
                          (negativeHalfHeight * cosine)) -
                         negativeHalfWidth));
                    vertices[2].y = (s32)(lowerFloat +
                        (((negativeHalfHeight * sine) +
                          (negativeHalfWidth * cosine)) -
                         negativeHalfHeight));
                    vertices[2].z = 0;
                    vertices[2].red = 0xFF;
                    vertices[2].green = 0xFF;
                    vertices[2].blue = 0xFF;
                    vertices[2].alpha = 0xFF;

                    vertices[3].x = (s32)(rightFloat +
                        (((halfWidth * sine) -
                          (negativeHalfHeight * cosine)) - halfWidth));
                    vertices[3].y = (s32)(lowerFloat +
                        (((negativeHalfHeight * sine) +
                          (halfWidth * cosine)) - negativeHalfHeight));
                    vertices[3].z = 0;
                    vertices[3].red = 0xFF;
                    vertices[3].green = 0xFF;
                    vertices[3].blue = 0xFF;
                    vertices[3].alpha = 0xFF;

                    triangles[0].flags = 0x40;
                    triangles[0].vertex0 = 0;
                    triangles[0].vertex1 = 1;
                    triangles[0].vertex2 = 2;
                    triangles[0].s0 = textureLeft;
                    triangles[0].t0 = textureTop;
                    triangles[0].s1 = textureRight;
                    triangles[0].t1 = textureTop;
                    triangles[0].s2 = textureLeft;
                    triangles[0].t2 = textureBottom;

                    triangles[1].flags = 0x40;
                    triangles[1].vertex0 = 1;
                    triangles[1].vertex1 = 3;
                    triangles[1].vertex2 = 2;
                    triangles[1].s0 = textureRight;
                    triangles[1].t0 = textureTop;
                    triangles[1].s1 = textureRight;
                    triangles[1].t1 = textureBottom;
                    triangles[1].s2 = textureLeft;
                    triangles[1].t2 = textureBottom;

                    vertices += 4;
                    triangles += 2;
                }
            }
        }
        element++;
        first = *current;
    }

    D_800D60E0 = savedFont;
    func_80034920(&dList);
    if (window != D_800D64E8) {
        gDPSetScissor(dList++, G_SC_NON_INTERLACE,
                      D_800D64E8[0].x1, D_800D64E8[0].y1,
                      D_800D64E8[0].x2, D_800D64E8[0].y2);
    }
    *displayList = dList;
    *vertexPtr = vertices;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o045/func_overlay_045_F0001158_188D5B0/func_overlay_045_F0001158_188D5B0.s")
#endif
