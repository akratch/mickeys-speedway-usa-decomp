#include "PR/ultratypes.h"

typedef struct Overlay83Command { u32 w0; u32 w1; } Overlay83Command;
typedef struct Overlay83Strip {
    u8 pad00[0x20];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad23[0xB];
    u8 count;
    u8 vertexIndex;
    u8 pad30[0x84];
} Overlay83Strip;

extern u8 gOverlay83VertexBase[];

#define SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))
#define SET_PRIM(packet, red, green, blue, alpha) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = 0xFA000000; \
    cmd->w1 = ((red) << 24) | ((green) << 16) | ((blue) << 8) | (alpha); \
}
#define SET_ENV(packet, red, green, blue, alpha) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = 0xFB000000; \
    cmd->w1 = ((red) << 24) | ((green) << 16) | ((blue) << 8) | (alpha); \
}
#define VERTEX(packet, address, vertexCount, firstVertex) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = SHIFTL(4, 24, 8) | \
              SHIFTL(((vertexCount) << 3) | ((u32)(address) & 6) | \
                         (firstVertex), 16, 8) | \
              SHIFTL(((vertexCount) * 10) + 8, 0, 16); \
    cmd->w1 = (u32)(address); \
}
#define POLYGON(packet, triangles, triangleCount, textured) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = SHIFTL((((triangleCount) - 1) << 4) | (textured), 16, 8) | \
              SHIFTL(5, 24, 8) | SHIFTL((triangleCount) * 16, 0, 16); \
    cmd->w1 = (u32)(triangles); \
}

#ifdef NON_MATCHING
void overlay83DrawStrip(Overlay83Command **displayList, Overlay83Strip *strip) {
    Overlay83Command **savedDisplayList;
    u8 count;
    s32 doubledCount;
    s32 vertexCount;

    savedDisplayList = displayList;
    count = strip->count;
    if (count != 0) {
        SET_PRIM((*displayList)++, 255, 255, 255, 255);
        SET_ENV((*displayList)++, strip->red, strip->green, strip->blue, 255);
        doubledCount = count * 2;
        vertexCount = doubledCount + 2;
        VERTEX((*displayList)++,
               (u8 *)&strip[strip->vertexIndex] + 0x800000F0,
               vertexCount, 0);
        POLYGON((*savedDisplayList)++, gOverlay83VertexBase, doubledCount, 1);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o083/overlay83DrawStrip/func_overlay_083_F0000850_18D0010.s")
#endif
