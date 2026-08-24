#include "PR/ultratypes.h"

typedef struct Overlay20Resource {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s16 stepX;
    s16 columns;
    s16 rows;
    s16 value0E;
    s16 value10;
    u8 pad12;
    s8 current;
    f32 start;
    f32 end;
    f32 scale;
    void *buffers[3];
    void *owner;
} Overlay20Resource;

extern void *func_overlay_020_F0000000_18765D8();

#ifdef NON_MATCHING
Overlay20Resource *overlay20ConfigureResource(
    Overlay20Resource *resource, s32 x, s32 y, s32 width, s32 height,
    s32 stepX, s32 columns, s32 rows, void *owner, s32 value0E,
    s32 value10, s32 start, s32 current, s32 end, s32 scaleDivisor) {
    if (resource == 0) {
        resource = func_overlay_020_F0000000_18765D8(0x30, 0x87);
        if (resource != 0) {
            resource->buffers[0] = 0;
            resource->buffers[1] = 0;
            resource->buffers[2] = 0;
        }
    } else if ((columns != resource->columns) || (rows != resource->rows)) {
        func_overlay_020_F0000000_18765D8(resource->buffers[0]);
        func_overlay_020_F0000000_18765D8(resource->buffers[1]);
        func_overlay_020_F0000000_18765D8(resource->buffers[2]);
        resource->buffers[0] = 0;
        resource->buffers[1] = 0;
        resource->buffers[2] = 0;
    }

    if (resource != 0) {
        resource->x = x;
        resource->y = y;
        resource->width = width;
        resource->height = height;
        resource->stepX = stepX;
        resource->columns = columns;
        resource->rows = rows;
        resource->value0E = value0E;
        resource->current = current;
        resource->value10 = value10;
        resource->start = (f32)(current - start);
        resource->scale = 1.0f;
        resource->end = (f32)(end - current);
        if (scaleDivisor > 0) {
            resource->scale /= (f32)scaleDivisor;
        }
        resource->owner = owner;
        resource = func_overlay_020_F0000000_18765D8(resource);
    }
    return resource;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/overlay20ConfigureResource/func_overlay_020_F00000A8_1876680.s")
#endif
