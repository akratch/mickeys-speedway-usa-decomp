/*
 * Resident font and dialogue-window code -- ROM 0x4BC40-0x4E1E0
 * (VRAM 0x8004B040-0x8004D5E0).
 *
 * PROVENANCE -- the translation-unit identity, candidate function names,
 * declarations, and struct starting point come from Jet Force Gemini's
 * public decompilation, src/font.c and src/font.h. JFG is a permitted
 * published retail-derived decomp under docs/CLEANROOM.md. Mickey's own
 * instructions decide every field, body, boundary, and final name here.
 *
 * The boundary is supported at both ends rather than by a whole-object match:
 * JFG's first font.c function, fontSetWindow0, is byte-identical at 0x4BC40
 * (7 unmasked words, ROM-wide unique); its final fontYSpacing shape is the
 * leaf at 0x4E1C0; and the next function is the separate osCreatePiManager.
 *
 * Flags: -O2 -mips2 -32, inherited from the measured src/main rule.
 */

#include "game/font.h"
#include "n_audio/mbi.h"

typedef struct FontSpacingData {
    u8 width;
    u8 verticalExtent;
    u8 characterWidth;
    u8 height;
    u32 romOffset;
    s16 textureSize;
    u16 format;
    Gfx *displayList;
    u32 unused10;
} FontSpacingData;

typedef struct FontGlyphData {
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
} FontGlyphData;

typedef struct FontTextureHeader {
    u8 width;
    u8 height;
    u8 pad2[8];
    u16 format;
    Gfx *displayList[2];
} FontTextureHeader;

extern DialogueTextElement D_800D60E8[32];
extern DialogueBoxBackground D_800D64E8[];
extern DialogueBoxBackground D_800D6510[];
extern s32 D_8007D538;
extern s32 D_8007D53C;
extern s32 D_8007D540;
extern s32 D_8007D544[];
extern char D_8007D594[];
extern u8 D_800D60E0;
extern FontSpacingData *D_800D60E4;
extern char *D_800D6640;
extern u8 D_800D664D;
extern u32 D_800D6638;
extern Gfx D_8007D4C8[];
extern Gfx D_8007D4E8[];
extern Gfx D_8007D508[];
extern Gfx D_8007D490[];
extern Gfx D_7D528[];
extern u8 *D_800D6628[];
extern char *D_800D6644;
extern char *D_800D6648;
extern u8 D_800D664C;
extern s8 D_8007D570[];
extern FontGlyphData *D_800D663C;

void *func_8002B280(s32 size, s32 tag);
void viGetCurrentSize(u32 *width, u32 *height);
void camSetScissor(Gfx **displayList);
void func_80034920(Gfx **displayList);
FontGlyphData *func_8004C690(s32 character);
void func_8004D39C(char *input, char *output);
u8 *func_8004D40C(s32 font, char *text, s32 maxWidth, u8 **lineStart,
                  s32 *outWidth);
void func_8004C140(Gfx **displayList, s32 x1, s32 y1, s32 x2, s32 y2);
void func_8004C200(Gfx **displayList, void *matrix, void *vertices,
                   s32 windowId);
void piRomLoadSection(s32 resourceId, void *destination, u32 offset, s32 size);

void func_8004B13C(Gfx **displayList, s32 windowId, s32 xpos, s32 ypos,
                   char *text, s32 alignmentFlags);
void func_8004B1DC(Gfx **displayList, DialogueBoxBackground *window,
                   char *text, s32 alignmentFlags);
s32 func_8004BA8C(char *text, s32 font, s32 convertString);
void func_8004C5A4(char *input, char *output, s32 number);

void fontSetWindow0(s32 width, s32 height) {
    D_800D64E8[0].x2 = width - 1;
    D_800D64E8[0].y2 = height - 1;
    D_800D64E8[0].width = width;
    D_800D64E8[0].height = height;
}

void func_8004B064(s32 mode) {
    D_800D664D = mode;
}

void fontSetWindowNoise(u8 red, u8 green, u8 blue) {
    D_8007D538 = red;
    D_8007D53C = green;
    D_8007D540 = blue;
}

void func_8004B0A4(s32 font) {
    D_800D60E0 = font;
    D_800D64E8[0].font = font;
}

void fontColour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity) {
    D_800D64E8[0].textColourR = red;
    D_800D64E8[0].textColourG = green;
    D_800D64E8[0].textColourB = blue;
    D_800D64E8[0].textColourA = alpha;
    D_800D64E8[0].opacity = opacity;
}

void func_8004B0DC(s32 red, s32 green, s32 blue, s32 alpha) {
    D_800D64E8[0].textBGColourR = red;
    D_800D64E8[0].textBGColourG = green;
    D_800D64E8[0].textBGColourB = blue;
    D_800D64E8[0].textBGColourA = alpha;
}

void func_8004B0F8(Gfx **displayList, s32 xpos, s32 ypos, char *text,
                   s32 alignmentFlags) {
    func_8004B13C(displayList, 0, xpos, ypos, text, alignmentFlags);
}

void func_8004B13C(Gfx **displayList, s32 windowId, s32 xpos, s32 ypos,
                   char *text, s32 alignmentFlags) {
    if (windowId >= 0 && windowId < 8) {
        DialogueBoxBackground *window = &D_800D64E8[windowId];

        window->xpos = xpos == -0x8000 ? window->width >> 1 : xpos;
        window->ypos = ypos == -0x8000 ? window->height >> 1 : ypos;
        func_8004B1DC(displayList, window, text, alignmentFlags);
    }
}

#ifdef NON_MATCHING
/*
 * PROVENANCE -- source-level organization was adapted from Diddy Kong
 * Racing's permitted published render_text_string body. Mickey's own
 * instructions, m2c draft, fields, control bytes, and display-list words
 * determine this candidate.
 */
void func_8004B1DC(Gfx **displayList, DialogueBoxBackground *window,
                   char *text, s32 alignmentFlags) {
    s32 savedFont;
    s32 x;
    s32 y;
    s32 width;
    s32 activeColour;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 textureS;
    s32 textureT;
    s32 spacing;
    u8 first;
    u8 second;
    char *current;
    FontSpacingData *font;
    FontGlyphData *glyph;
    Gfx *dList;
    Gfx *fontCommands;

    if (text == NULL) {
        return;
    }

    dList = *displayList;
    func_8004D39C(text, D_800D6648);
    savedFont = D_800D60E0;
    D_800D60E0 = window->font;
    font = &D_800D60E4[D_800D60E0];
    current = D_800D6648;

    gSPDisplayList((*displayList)++, D_8007D490);
    if (window != D_800D64E8) {
        s32 x1 = window->x1;
        s32 y1 = window->y1;
        s32 x2 = window->x2;
        s32 y2 = window->y2;

        if (D_800D64E8[0].x2 >= x1 && D_800D64E8[0].y2 >= y1 &&
            x2 >= 0 && y2 >= 0) {
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
            dList->words.w0 = 0xED000000 |
                (((s32) ((f32) x1 * 4.0f) & 0xFFF) << 12) |
                ((s32) ((f32) y1 * 4.0f) & 0xFFF);
            dList->words.w1 =
                (((s32) ((f32) x2 * 4.0f) & 0xFFF) << 12) |
                ((s32) ((f32) y2 * 4.0f) & 0xFFF);
            dList++;
        } else {
            return;
        }
    }

    x = window->xpos;
    y = window->ypos;
    width = -1;
    if (alignmentFlags & 5) {
        width = func_8004BA8C(current, window->font, 0);
        if (alignmentFlags & 1) {
            x = (x - width) + 1;
        } else {
            x -= width >> 1;
        }
    }
    if (alignmentFlags & 2) {
        y = (y - font->verticalExtent) + 1;
    }
    if (alignmentFlags & 8) {
        y -= font->verticalExtent >> 1;
    }

    if (window->textBGColourA != 0) {
        dList->words.w0 = 0xFB000000;
        dList->words.w1 = (window->textBGColourR << 24) |
                          (window->textBGColourG << 16) |
                          (window->textBGColourB << 8) |
                          window->textBGColourA;
        dList++;
        if (width == -1) {
            width = func_8004BA8C(current, window->font, 0);
        }
        dList->words.w0 = 0xFA000000;
        dList->words.w1 = (D_8007D538 << 24) |
                          ((D_8007D53C & 0xFF) << 16) |
                          ((D_8007D540 & 0xFF) << 8);
        dList++;
        dList->words.w0 = 0x07020010;
        dList->words.w1 = (u32) D_7D528;
        dList++;
        dList->words.w0 = 0xF6000000 |
            (((window->x1 + x + width) & 0x3FF) << 14) |
            (((font->verticalExtent + y + window->y1) & 0x3FF) * 4);
        dList->words.w1 = (((window->x1 + x) & 0x3FF) << 14) |
                          (((y + window->y1) & 0x3FF) * 4);
        dList++;
        dList->words.w0 = 0xE7000000;
        dList->words.w1 = 0;
        dList++;
    }

    dList->words.w0 = 0xFA000000;
    dList->words.w1 = 0xFFFFFF00 | window->opacity;
    dList++;
    dList->words.w0 = 0xFB000000;
    dList->words.w1 = (window->textColourR << 24) |
                      (window->textColourG << 16) |
                      (window->textColourB << 8) |
                      window->textColourA;
    dList++;

    activeColour = 0;
    first = *current;
    while (first != 0 && window->y2 >= y) {
        current++;
        spacing = 0;
        if (first & 0x80) {
            second = *current++;
            if (second == 0 || second == 0xF) {
                spacing = font->characterWidth;
            } else {
                if (D_800D664D != 0) {
                    if (second == 2) {
                        dList->words.w0 = 0xE7000000;
                        dList->words.w1 = 0;
                        dList++;
                        dList->words.w0 = 0xFB000000;
                        dList->words.w1 = 0x0000FFFF;
                        dList++;
                        activeColour = 1;
                    } else if (second == 0xE) {
                        dList->words.w0 = 0xE7000000;
                        dList->words.w1 = 0;
                        dList++;
                        dList->words.w0 = 0xFB000000;
                        dList->words.w1 = 0x00FF00FF;
                        dList++;
                        activeColour = 1;
                    } else if (second >= 0x41 && second < 0x45) {
                        dList->words.w0 = 0xE7000000;
                        dList->words.w1 = 0;
                        dList++;
                        dList->words.w0 = 0xFB000000;
                        dList->words.w1 = 0xFFFF00FF;
                        dList++;
                        activeColour = 1;
                    } else if (activeColour != 0) {
                        dList->words.w0 = 0xE7000000;
                        dList->words.w1 = 0;
                        dList++;
                        dList->words.w0 = 0xFB000000;
                        dList->words.w1 = (window->textColourR << 24) |
                                          (window->textColourG << 16) |
                                          (window->textColourB << 8) |
                                          window->textColourA;
                        dList++;
                        activeColour = 0;
                    }
                }

                left = window->x1 + x;
                top = window->y1 + y;
                spacing = D_800D6628[D_800D60E0][second];
                if (font->width + left > 0 && font->verticalExtent + top > 0 &&
                    left < window->x2 && top < window->y2) {
                    glyph = func_8004C690(second);
                    if (glyph != NULL) {
                        right = ((glyph->right - glyph->left) * 4) + left * 4;
                        bottom = ((glyph->bottom - glyph->top) * 4) +
                                 (glyph->top + top) * 4;
                        left *= 4;
                        top = (glyph->top + top) * 4;
                        spacing = glyph->advance;
                        textureS = glyph->left << 5;
                        textureT = glyph->top << 5;
                        if (left < 0 && right > 0) {
                            textureS -= left * 8;
                            left = 0;
                        }
                        if (top < 0 && bottom > 0) {
                            textureT -= top * 8;
                            top = 0;
                        }

                        fontCommands = font->displayList;
                        dList->words.w0 = fontCommands->words.w0;
                        dList->words.w1 = D_800D6638 + glyph->textureOffset;
                        dList++;
                        fontCommands++;
                        if (font->format == 4) {
                            dList->words.w0 = 0x07060030;
                            dList->words.w1 = (u32) fontCommands + 0x80000000;
                            dList++;
                            dList->words.w0 = fontCommands[6].words.w0;
                            dList->words.w1 = D_800D6638 + glyph->textureOffset2;
                            dList++;
                            fontCommands += 7;
                        }
                        dList->words.w0 = 0x07080040;
                        dList->words.w1 = (u32) fontCommands + 0x80000000;
                        dList++;
                        dList->words.w0 = 0xE4000000 |
                                          ((right & 0xFFF) << 12) |
                                          (bottom & 0xFFF);
                        dList->words.w1 = ((left & 0xFFF) << 12) |
                                          (top & 0xFFF);
                        dList++;
                        dList->words.w0 = 0xB3000000;
                        dList->words.w1 = (textureS << 16) |
                                          (textureT & 0xFFFF);
                        dList++;
                        dList->words.w0 = 0xB2000000;
                        dList->words.w1 = 0x04000400;
                        dList++;
                    }
                }
            }
            if (D_800D664C != 0 && spacing != 0) {
                spacing--;
            }
            x += spacing;
        } else {
            switch (first) {
                default:
                    x += font->characterWidth;
                    break;
                case '\n':
                    x = window->textOffsetX;
                    y += font->height;
                    break;
                case '\t':
                    width = font->characterWidth * 4;
                    x = (x + width) - ((x - window->textOffsetX) % width);
                    break;
                case '\v':
                    y += font->height;
                    break;
                case '\r':
                    x = window->textOffsetX;
                    break;
            }
        }
        first = *current;
    }

    window->xpos = x - window->textOffsetX;
    window->ypos = y - window->textOffsetY;
    D_800D60E0 = savedFont;
    *displayList = dList;
    func_80034920(displayList);
    if (window != D_800D64E8) {
        camSetScissor(displayList);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B1DC.s")
#endif
/* Plateau (batch 14): exact 0xB8, best 4 words first at +0x30.
 * Width hoist and u32 glyph index fix the schedule/homes; fontData remains a3 vs v0.
 * Flags, types, declarations, expression forms, and bounded permuter found no zero. */
#ifdef NON_MATCHING
/*
 * PROVENANCE -- source organization was cross-checked against JFG's
 * permitted published fontStringWidth assembly and DKR's Japanese
 * get_text_width body. Mickey's own m2c draft and data layout are authority.
 * Plateau: exact 46-word size/relocation surface, with 10 positional words
 * differing from +0x18. The remaining blocker is the 0x28-versus-0x30 frame
 * and font-record pointer spill/register web around the conversion call.
 */
s32 func_8004BA8C(char *text, s32 font, s32 convertString) {
    FontSpacingData *fontData;
    u8 *spacing;
    s32 width;
    u8 current;
    u8 defaultWidth;
    u8 glyphWidth;

    fontData = &D_800D60E4[font];
    spacing = D_800D6628[font];
    if (convertString != 0) {
        func_8004D39C(text, D_800D6644);
        text = D_800D6644;
    }

    current = *text;
    width = 0;
    defaultWidth = fontData->characterWidth;
    if (current != 0) {
        do {
            text++;
            glyphWidth = defaultWidth;
            if (current & 0x80) {
                current = *text++;
                glyphIndex = current;
                if (current != 0 && glyphIndex != 0xF) {
                    glyphWidth = spacing[glyphIndex];
                }
            }
            current = *text;
            width += glyphWidth;
        } while (current != 0);
    }
    return width;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BA8C.s")
#endif
void func_8004BB44(s32 windowId, s32 x1, s32 y1, s32 x2, s32 y2) {
    if (windowId > 0 && windowId < 8) {
        DialogueBoxBackground *window = &D_800D64E8[windowId];

        window->xpos = 0;
        window->ypos = 0;
        if (x1 < x2) {
            window->x1 = (s16) x1;
            window->x2 = (s16) x2;
        } else {
            window->x2 = (s16) x1;
            window->x1 = (s16) x2;
        }
        if (y1 < y2) {
            window->y1 = (s16) y1;
            window->y2 = (s16) y2;
        } else {
            window->y2 = (s16) y1;
            window->y1 = (s16) y2;
        }
        window->width = (window->x2 - window->x1) + 1;
        window->height = (window->y2 - window->y1) + 1;
    }
}

void func_8004BBE0(s32 windowId, s32 font) {
    D_800D64E8[windowId].font = font;
}

void fontWindowColour(s32 windowId, s32 red, s32 green, s32 blue, s32 alpha) {
    if (windowId > 0 && windowId < 8) {
        DialogueBoxBackground *window = &D_800D64E8[windowId];

        window->backgroundColourR = red;
        window->backgroundColourG = green;
        window->backgroundColourB = blue;
        window->backgroundColourA = alpha;
    }
}


void fontWindowFontColour(s32 windowId, s32 red, s32 green, s32 blue,
                          s32 alpha, s32 opacity) {
    if (windowId > 0 && windowId < 8) {
        DialogueBoxBackground *window = &D_800D64E8[windowId];

        window->textColourR = red;
        window->textColourG = green;
        window->textColourB = blue;
        window->textColourA = alpha;
        window->opacity = opacity;
    }
}

void fontWindowFontBackground(s32 windowId, s32 red, s32 green, s32 blue,
                              s32 alpha) {
    if (windowId > 0 && windowId < 8) {
        DialogueBoxBackground *window = &D_800D64E8[windowId];

        window->textBGColourR = red;
        window->textBGColourG = green;
        window->textBGColourB = blue;
        window->textBGColourA = alpha;
    }
}

/* Workbench: relocation-only; all 168 words, the frame, and the relocation sites are exact.
 * The alias lever and prior direct-alias/pointer/stride forms cannot preserve both raw addends and schedule.
 * Four HI16/LO16 identities remain linked-equivalent but object-distinct at +0x34 and +0x98. */
#ifdef NON_MATCHING
void *func_8004BCC4(s32 windowId, s32 posX, s32 posY, char *text, s32 number,
                    s32 flags) {
    s32 i;
    s32 width;
    DialogueTextElement *result;
    s32 pad;
    DialogueBox *textBox;
    DialogueBox **textBoxPtr;
    FontSpacingData *fontData;

    if (text == NULL) {
        return NULL;
    }

    for (i = 0, result = NULL; i < 32 && result == NULL; i++) {
        if (D_800D60E8[i].number == 0xFF) {
            result = &D_800D60E8[i];
        }
    }

    if (result != NULL) {
        if (posX == -0x8000) {
            posX = D_800D64E8[windowId].width >> 1;
        }
        if (posY == -0x8000) {
            posY = D_800D64E8[windowId].height >> 1;
        }
        if (D_800D64E8[windowId].font != 0xFF) {
            fontData = &D_800D60E4[D_800D64E8[windowId].font];
            if (flags & 5) {
                func_8004C5A4(text, D_800D6640, number);
                width = func_8004BA8C(D_800D6640,
                                      D_800D64E8[windowId].font, 1);
                if (flags & 1) {
                    posX = (posX - width) + 1;
                } else {
                    posX -= width >> 1;
                }
            }
            if (flags & 2) {
                posY = (posY - fontData->verticalExtent) + 1;
            }
            if (flags & 8) {
                posY -= fontData->verticalExtent >> 1;
            }
        }
        if (D_800D64E8[windowId].textBox == NULL) {
            D_800D64E8[windowId].textBox = (DialogueBox *) result;
            result->nextBox = NULL;
        } else {
            textBoxPtr = &D_800D64E8[windowId].textBox;
            textBox = *textBoxPtr;
            while (textBox != NULL && number < textBox->textNum) {
                textBoxPtr = &textBox->nextBox;
                textBox = textBox->nextBox;
            }
            *textBoxPtr = (DialogueBox *) result;
            result->nextBox = textBox;
        }
        result->number = number;
        result->text = text;
        result->posX = posX;
        result->posY = posY;
        result->offsetX = 0;
        result->offsetY = 0;
        result->textColourR = D_800D64E8[windowId].textColourR;
        result->textColourG = D_800D64E8[windowId].textColourG;
        result->textColourB = D_800D64E8[windowId].textColourB;
        result->textColourA = D_800D64E8[windowId].textColourA;
        result->textBGColourR = D_800D64E8[windowId].textBGColourR;
        result->textBGColourG = D_800D64E8[windowId].textBGColourG;
        result->textBGColourB = D_800D64E8[windowId].textBGColourB;
        result->textBGColourA = D_800D64E8[windowId].textBGColourA;
        result->opacity = D_800D64E8[windowId].opacity;
        result->font = D_800D64E8[windowId].font;
        result->flags = D_800D64E8[windowId].flags;
    }

    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BCC4.s")
#endif

void func_8004BF64(s32 windowId) {
    DialogueBoxBackground *window;
    DialogueBox *textBox;
    DialogueBox *current;

    window = &D_800D64E8[windowId];
    textBox = window->textBox;
    if (textBox != NULL) {
        current = textBox;
        while (current != NULL) {
            current->textNum = 0xFF;
            current = current->nextBox;
        }
        window->textBox = NULL;
    }
}

void func_8004BFB0(s32 windowId) {
    D_800D64E8[windowId].flags |= 0x8000;
}

void func_8004BFD8(s32 windowId) {
    D_800D64E8[windowId].flags &= 0x7FFF;
}

void func_8004C000(char **outString, s32 number) {
    u8 digit;
    s32 i;
    s32 hasDigit;
    s32 quotient;
    s32 *power;
    char *output = *outString;

    if (number < 0) {
        *output = '-';
        output++;
        number = -number;
    }

    hasDigit = 0;
    for (i = 0; D_8007D544[i] != 0; i++) {
        digit = '0';
        power = &D_8007D544[i];
        if (number >= *power) {
            quotient = number / *power;
            number -= quotient * *power;
            digit += quotient;
            hasDigit = 1;
        }
        if (hasDigit) {
            *output = digit;
            output++;
        }
    }

    *output++ = '0' + number;
    *outString = output;
}

/*
 * PROVENANCE -- source organization was cross-checked against JFG's
 * permitted published fontWindowsDraw assembly. Mickey's own flags, window
 * alias, and callee interface determine this body.
 */
void func_8004C0C4(Gfx **displayList, void *matrix, void *vertices) {
    DialogueBoxBackground *window;
    s32 i;

    window = D_800D6510;
    i = 1;
    do {
        if (window->flags & 0x8000) {
            func_8004C200(displayList, NULL, NULL, i);
        }
        i++;
        window++;
    } while (i != 8);
}
/*
 * PROVENANCE -- adapted from DKR's permitted published
 * render_fill_rectangle. Mickey's own call target and assembly determine
 * the framebuffer-size interface and final code shape.
 */
void func_8004C140(Gfx **displayList, s32 x1, s32 y1, s32 x2, s32 y2) {
    u32 width;
    u32 height;

    viGetCurrentSize(&width, &height);
    if (x2 >= 0 && (u32) x1 < width && y2 >= 0 && (u32) y1 < height) {
        if (x1 < 0) {
            x1 = 0;
        }
        if (y1 < 0) {
            y1 = 0;
        }
        gDPFillRectangle((*displayList)++, x1, y1, x2, y2);
    }
}
/*
 * PROVENANCE -- adapted from Diddy Kong Racing's permitted published
 * render_dialogue_box body. Mickey's own instructions and data layout
 * determine the commands, dimensions, and omission of DKR's 3D setup.
 */
void func_8004C200(Gfx **displayList, void *matrix, void *vertices,
                   s32 windowId) {
    DialogueBoxBackground *window;
    DialogueTextElement *element;
    s32 i;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    window = &D_800D64E8[windowId];
    if (window->backgroundColourA != 0) {
        gSPDisplayList((*displayList)++, D_8007D490);

        gDPSetPrimColor((*displayList)++, 0, 0, D_8007D538,
                        D_8007D53C, D_8007D540, 0);

        gDma1p((*displayList)++, 7, D_7D528, 0x10, 2);

        gDPSetEnvColor((*displayList)++, 0, 0, 0, 0);

        if ((window->x2 - window->x1) < 10 ||
            (window->y2 - window->y1) < 10) {
            func_8004C140(displayList, window->x1 - 2, window->y1 - 2,
                          window->x2 + 2, window->y2 + 2);
        } else {
            func_8004C140(displayList, window->x1 - 2, window->y1 + 2,
                          window->x1 + 2, window->y2 - 2);
            func_8004C140(displayList, window->x1 - 2, window->y1 - 2,
                          window->x2 + 2, window->y1 + 2);
            func_8004C140(displayList, window->x2 - 2, window->y1 + 2,
                          window->x2 + 2, window->y2 - 2);
            func_8004C140(displayList, window->x1 - 2, window->y2 - 2,
                          window->x2 + 2, window->y2 + 2);
        }

        gDPPipeSync((*displayList)++);
        gDPSetEnvColor((*displayList)++, window->backgroundColourR,
                       window->backgroundColourG, window->backgroundColourB,
                       window->backgroundColourA);

        for (i = 0; D_8007D570[i] >= 0; i += 5) {
            x1 = D_8007D570[i] + window->x1;
            y1 = D_8007D570[i + 1] != 0
                     ? D_8007D570[i + 2] + window->y2
                     : D_8007D570[i + 2] + window->y1;
            x2 = window->x2 - D_8007D570[i];
            y2 = D_8007D570[i + 3] != 0
                     ? D_8007D570[i + 4] + window->y2
                     : D_8007D570[i + 4] + window->y1;
            func_8004C140(displayList, x1, y1, x2, y2);
        }

        gDPPipeSync((*displayList)++);
    }

    element = (DialogueTextElement *) window->textBox;
    while (element != NULL) {
        window->xpos = element->posX + element->offsetX;
        window->ypos = element->posY + element->offsetY;
        window->textColourR = element->textColourR;
        window->textColourG = element->textColourG;
        window->textColourB = element->textColourB;
        window->textColourA = element->textColourA;
        window->textBGColourR = element->textBGColourR;
        window->textBGColourG = element->textBGColourG;
        window->textBGColourB = element->textBGColourB;
        window->textBGColourA = element->textBGColourA;
        window->opacity = element->opacity;
        window->font = element->font;
        func_8004C5A4(element->text, D_800D6640, element->number);
        func_8004B1DC(displayList, window, D_800D6640, 0);
        element = (DialogueTextElement *) element->nextBox;
    }
}
void func_8004C5A4(char *input, char *output, s32 number) {
    char currentChar;

    do {
        currentChar = *input++;
        if (currentChar & 0x80) {
            char nextChar = *input++;

            if (nextChar == 0xE) {
                func_8004C000(&output, number);
            } else {
                *output++ = currentChar;
                *output++ = nextChar;
            }
        } else if (currentChar == '~') {
            func_8004C000(&output, number);
        } else {
            *output++ = currentChar;
        }
    } while (currentChar);
}

/* Plateau (2026-08-25): -O2 -mips2 is 0x8 short with 105 differing words; first mismatch +0x0.
 * A separate character index removes the s0 save and reusing blockCount fixes the chain lifetime;
 * font/index allocation and IDO's peeled four-word header copy remain the likely blockers. */
#ifdef NON_MATCHING
/*
 * PROVENANCE -- source organization was cross-checked against JFG's
 * permitted published func_80071B08 cache allocator. Mickey's own m2c
 * draft, constants, structure offsets, and loader call determine this body.
 */
FontGlyphData *func_8004C690(s32 character) {
    s32 savedHeader[4];
    FontSpacingData *font;
    FontGlyphData *entries;
    FontGlyphData *entry;
    FontGlyphData *result;
    s32 *header;
    s32 *source;
    s32 *destination;
    s32 index;
    s32 runLength;
    s32 remaining;
    u32 blockCount;
    u32 copyIndex;
    s32 fontIndex;
    s32 characterIndex;

    characterIndex = character & 0xFF;
    fontIndex = D_800D60E0;
    result = NULL;
    index = 0;
    font = &D_800D60E4[fontIndex];
    entries = D_800D663C;
    entry = entries;
    do {
        index++;
        if (fontIndex == entry->font && characterIndex == entry->character) {
            result = entry;
        }
        entry++;
    } while (index < 0x100 && result == NULL);

    entry = entries;
    if (result != NULL) {
        u8 nextLength;

        entry = result;
        do {
            nextLength = entry->chainLength;
            entry->state = 2;
            entry++;
        } while (nextLength != 0);
    } else {
        index = 0;
        runLength = 0;
        blockCount = ((u32) font->textureSize + 0xEF) >> 8;
        if (blockCount != 0) {
            do {
                index++;
                if (entry->state == 0) {
                    if (runLength == 0) {
                        result = entry;
                    }
                    runLength++;
                } else {
                    runLength = 0;
                }
                entry++;
            } while (index < 0x100 && runLength != blockCount);
        }

        entry = result;
        if (runLength == blockCount) {
            remaining = blockCount - 1;
            if (remaining >= 0) {
                do {
                    blockCount = entry->chainLength;
                    entry->chainLength = remaining;
                    remaining--;
                    entry->font = fontIndex;
                    entry->character = characterIndex;
                    entry->state = 2;
                    entry++;
                } while (remaining >= 0);
            }
            if (blockCount != 0) {
                do {
                    blockCount = entry->chainLength;
                    entry->font = 0xFF;
                    entry->chainLength = 0;
                    entry++;
                } while (blockCount != 0);
            }

            header = (s32 *)
                ((D_800D6638 + result->allocationOffset) - 0x10);
            source = header;
            destination = savedHeader;
            copyIndex = 0;
            do {
                *destination++ = *source++;
                copyIndex++;
            } while (copyIndex < 4);

            piRomLoadSection(0x39, header,
                          font->romOffset + (characterIndex * font->textureSize),
                          font->textureSize);
            result->textureOffset =
                (result->allocationOffset + ((u16 *) header)[0]) - 0x10;
            result->textureOffset2 =
                (result->allocationOffset + ((u16 *) header)[1]) - 0x10;
            result->left = ((u8 *) header)[4];
            result->top = ((u8 *) header)[5];
            result->right = ((u8 *) header)[6];
            result->bottom = ((u8 *) header)[7];
            result->advance = ((u8 *) header)[8];
            header[0] = savedHeader[0];
            header[1] = savedHeader[1];
            header[2] = savedHeader[2];
            header[3] = savedHeader[3];
        } else {
            result = NULL;
        }
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C690.s")
#endif
void func_8004C8D8(FontTextureHeader *texture, s32 unused) {
    Gfx *displayList;
    Gfx *state;
    s32 i;

    texture->displayList[0] = func_8002B280(0x100, 0x86);
    texture->displayList[1] = texture->displayList[0] + 16;

    for (i = 0; i < 2; i++) {
        displayList = texture->displayList[i];
        if (displayList != NULL) {
            state = D_8007D4C8;
            switch (texture->format) {
                case 4:
                    gDPLoadTextureBlockS(displayList++, D_800D6638,
                                         G_IM_FMT_RGBA, G_IM_SIZ_16b,
                                         texture->width, texture->height, 0,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    gDPLoadMultiBlock_4bS(displayList++, D_800D6638, 0x100,
                                         1, G_IM_FMT_I, texture->width,
                                         texture->height, 0,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    state = D_8007D4E8;
                    break;
                case 2:
                    gDPLoadTextureBlockS(displayList++, D_800D6638,
                                         G_IM_FMT_RGBA, G_IM_SIZ_32b,
                                         texture->width, texture->height, 0,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 3:
                    gDPLoadTextureBlockS(displayList++, D_800D6638,
                                         G_IM_FMT_RGBA, G_IM_SIZ_16b,
                                         texture->width, texture->height, 0,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 1:
                    gDPLoadTextureBlockS(displayList++, D_800D6638,
                                         G_IM_FMT_IA, G_IM_SIZ_8b,
                                         texture->width, texture->height, 0,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 0:
                    gDPLoadTextureBlock_4bS(displayList++, D_800D6638,
                                            G_IM_FMT_IA, texture->width,
                                            texture->height, 0,
                                            G_TX_NOMIRROR | G_TX_WRAP,
                                            G_TX_NOMIRROR | G_TX_WRAP,
                                            8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 5:
                    gDPLoadTextureBlock_4bS(displayList++, D_800D6638,
                                            G_IM_FMT_I, texture->width,
                                            texture->height, 0,
                                            G_TX_NOMIRROR | G_TX_WRAP,
                                            G_TX_NOMIRROR | G_TX_WRAP,
                                            8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 6:
                    gDPLoadTextureBlockS(displayList++, D_800D6638,
                                         G_IM_FMT_I, G_IM_SIZ_8b,
                                         texture->width, texture->height, 0,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         G_TX_NOMIRROR | G_TX_WRAP,
                                         8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 7:
                    gDPLoadTextureBlock_4bS(displayList++, D_800D6638,
                                            G_IM_FMT_I, texture->width,
                                            texture->height, 0,
                                            G_TX_NOMIRROR | G_TX_WRAP,
                                            G_TX_NOMIRROR | G_TX_WRAP,
                                            8, 8, G_TX_NOLOD, G_TX_NOLOD);
                    state = D_8007D508;
                    break;
            }
            if (i == 1) {
                state += 2;
            }
            displayList->words.w0 = state[0].words.w0;
            displayList->words.w1 = state[0].words.w1;
            displayList++;
            displayList->words.w0 = state[1].words.w0;
            displayList->words.w1 = state[1].words.w1;
        }
    }
}
void func_8004D32C(void) {
    u8 *entry;
    s32 i;

    entry = (i = 0, (u8 *) D_800D663C);
    do {
        if (entry[4] != 0) {
            entry[4]--;
        }
        entry += sizeof(FontGlyphData);
        i++;
    } while (i != 0x100);
}
/*
 * PROVENANCE -- source organization was adapted from JFG's permitted
 * published fontConvertString body. Mickey's own character table and
 * instruction flow determine this implementation.
 */
void func_8004D39C(char *input, char *output) {
    char currentChar;
    char *conversionTable = D_8007D594;

    do {
        currentChar = *input++;
        if (currentChar & 0x80) {
            *output++ = currentChar;
            *output++ = *input++;
        } else if (currentChar < 0x20) {
            *output++ = currentChar;
        } else {
            *output++ = 0x80;
            *output++ = conversionTable[currentChar - 0x20];
        }
    } while (currentChar);
}

/* Workbench: mixed structural/register; 109 words, five differences, first +0x5C.
 * Structure-buckets and distinct-initial-value levers preserve the same optimized object.
 * A delay-slot copy and its dependent temporary web remain unrecovered; canonical assembly stays. */
#ifdef NON_MATCHING
/*
 * PROVENANCE -- source organization was cross-checked against JFG's
 * permitted published fontGetLine assembly. Mickey's own m2c draft,
 * constants, branch structure, and font-record layout determine this body.
 */
u8 *func_8004D40C(s32 font, char *text, s32 maxWidth, u8 **lineStart,
                  s32 *outWidth) {
    s32 totalWidth;
    s32 firstLine;
    s32 overflow;
    s32 end;
    s32 delimiter;
    s32 segmentWidth;
    char *segmentStart;
    s32 current;
    u8 code;

    totalWidth = 0;
    firstLine = 1;
    overflow = 0;
    end = 0;

    do {
        delimiter = 0;
        segmentWidth = 0;
        segmentStart = text;
        do {
            current = *text;
            if (0x80 == current && text[1] == 0xF) {
                text += 2;
                if (firstLine == 0) {
                    segmentWidth += D_800D60E4[font].characterWidth;
                }
            } else if (current == 0) {
                end = 1;
            } else if (0x80 != current) {
                text++;
            } else {
                delimiter = 1;
            }
        } while (delimiter == 0 && end == 0);

        if (firstLine != 0) {
            firstLine = 0;
            if (end != 0) {
                *outWidth = totalWidth;
                return NULL;
            }
            *lineStart = text;
        }

        current = *text;
        if (0x80 == current) {
            code = text[1];
            if (code != 0xF) {
                for (;;) {
                    current = text[2];
                    text += 2;
                    segmentWidth += D_800D6628[font][code];
                    if (0x80 == current) {
                        code = text[1];
                        if (code != 0xF) {
                            continue;
                        }
                    }
                    break;
                }
            }
        }

        totalWidth += segmentWidth;
        if (maxWidth < totalWidth) {
            overflow = 1;
            totalWidth -= segmentWidth;
            text = segmentStart;
            current = *segmentStart;
        }
        if (current == 0) {
            end = 1;
        }
    } while (overflow == 0 && end == 0);

    if (outWidth != NULL) {
        *outWidth = totalWidth;
    }
    if (text == *lineStart) {
        return NULL;
    }
    return text;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D40C.s")
#endif
u8 func_8004D5C0(s32 font) {
    return D_800D60E4[font].height;
}
