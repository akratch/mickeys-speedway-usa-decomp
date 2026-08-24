#ifndef MICKEY_GAME_FONT_H
#define MICKEY_GAME_FONT_H

#include "PR/ultratypes.h"

/* Resident dialogue-window state; field offsets are confirmed by Mickey. */
typedef struct DialogueBoxBackground {
    /* 0x00 */ s16 xpos;
    /* 0x02 */ s16 ypos;
    /* 0x04 */ s16 x1;
    /* 0x06 */ s16 y1;
    /* 0x08 */ s16 x2;
    /* 0x0A */ s16 y2;
    /* 0x0C */ s16 width;
    /* 0x0E */ s16 height;
    /* 0x10 */ u8 backgroundColourR;
    /* 0x11 */ u8 backgroundColourG;
    /* 0x12 */ u8 backgroundColourB;
    /* 0x13 */ u8 backgroundColourA;
    /* 0x14 */ u8 textColourR;
    /* 0x15 */ u8 textColourG;
    /* 0x16 */ u8 textColourB;
    /* 0x17 */ u8 textColourA;
    /* 0x18 */ u8 textBGColourR;
    /* 0x19 */ u8 textBGColourG;
    /* 0x1A */ u8 textBGColourB;
    /* 0x1B */ u8 textBGColourA;
    /* 0x1C */ u8 opacity;
    /* 0x1D */ u8 font;
    /* 0x1E */ u16 flags;
    /* 0x20 */ s16 textOffsetX;
    /* 0x22 */ s16 textOffsetY;
    /* 0x24 */ void *textBox;
} DialogueBoxBackground;

void fontSetWindowNoise(u8 red, u8 green, u8 blue);
void fontSetWindow0(s32 width, s32 height);
void fontWindowFontColour(s32 windowId, s32 red, s32 green, s32 blue,
                          s32 alpha, s32 opacity);

#endif
