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

extern DialogueBoxBackground D_800D64E8[];
extern s32 D_8007D538;
extern s32 D_8007D53C;
extern s32 D_8007D540;
extern s32 D_8007D544[];
extern u8 D_800D60E0;
extern u8 D_800D664D;

void func_8004B13C(void **displayList, s32 windowId, s32 xpos, s32 ypos,
                   char *text, s32 alignmentFlags);
void func_8004B1DC(void **displayList, DialogueBoxBackground *window,
                   char *text, s32 alignmentFlags);

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

void func_8004B0F8(void **displayList, s32 xpos, s32 ypos, char *text,
                   s32 alignmentFlags) {
    func_8004B13C(displayList, 0, xpos, ypos, text, alignmentFlags);
}

void func_8004B13C(void **displayList, s32 windowId, s32 xpos, s32 ypos,
                   char *text, s32 alignmentFlags) {
    if (windowId >= 0 && windowId < 8) {
        DialogueBoxBackground *window = &D_800D64E8[windowId];

        window->xpos = xpos == -0x8000 ? window->width >> 1 : xpos;
        window->ypos = ypos == -0x8000 ? window->height >> 1 : ypos;
        func_8004B1DC(displayList, window, text, alignmentFlags);
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B1DC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BA8C.s")
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

#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BCC4.s")
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

#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C0C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C140.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C200.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C5A4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C690.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C8D8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D32C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D39C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D40C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D5C0.s")
