#include "ultra64.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay28Point;

typedef struct {
    u8 pad00[0x18];
    f32 baseScale;
    f32 xScale;
    f32 yScale;
    s16 xAngle;
    s16 yAngle;
    s16 xStep;
    s16 yStep;
    u8 pad2C[0x16];
    s16 bufferIndex;
    u8 pad44[0xA];
    Overlay28Point points[2][17];
} Overlay28State;

extern f32 ext_o0_2a470(s32 angle);
extern f32 ext_o0_2a46c(s32 angle);

void overlay28UpdateVertices(Overlay28State *state) {
    Overlay28Point *point;
    s32 angle;
    s32 xAngle;
    s32 yAngle;
    s32 remaining;
    f32 xScale;
    f32 yScale;
    f32 xWave;
    f32 scale;

    state->bufferIndex ^= 1;
    point = state->points[state->bufferIndex];
    angle = 0;
    xAngle = state->xAngle;
    xScale = state->xScale;
    yAngle = state->yAngle;
    yScale = state->yScale;
    remaining = 15;
    do {
        xWave = ext_o0_2a470(xAngle);
        scale = (ext_o0_2a470(yAngle) * yScale) +
                ((8.0f * state->baseScale) + (xScale * xWave));
        xAngle += state->xStep;
        yAngle += state->yStep;
        point->x = (s32) (ext_o0_2a470(angle) * scale);
        point->y = (s32) (ext_o0_2a46c(angle) * scale);
        angle += 0x1000;
        point++;
    } while (remaining--);
}
