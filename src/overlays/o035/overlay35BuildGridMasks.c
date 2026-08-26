#include "ultra64.h"

typedef struct O35Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[4];
} O35Vertex;

typedef struct O35GridBounds {
    s16 x0;
    s16 z0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 z1;
} O35GridBounds;

typedef struct O35GridSource {
    O35Vertex *vertices;
    u8 *indices;
    u8 pad8[4];
    u8 *groups;
    u32 *masks;
    u8 *zMasks;
    u8 pad18[0xC];
    s16 groupCount;
} O35GridSource;

/* Workbench verdict=structure-mismatch; best 61 raw/195 matched words, exact 244-word body, first real mismatch +0x0.
 * Flag lattice, spill-home/liveness reuse, merged x/counter, and third-axis field-order levers were tried; the last aligns that loop.
 * Remains: -0x88 versus -0x80 frame, early group-home/pool slot 5, and first/second-axis loop structure. */
#ifdef NON_MATCHING
void func_overlay_035_F0000770_1882450(O35GridSource *source,
                                        O35GridBounds *bounds) {
    s32 groupIndex;
    s32 groupOffset;

    groupIndex = 0;
    if (source->groupCount > 0) {
        groupOffset = 0;
        do {
            u8 *group;
            s32 vertex;
            s32 end;
            s32 flags;
            s16 baseIndex;
            s16 startVertex;

            group = source->groups + groupOffset;
            startVertex = *(s16 *)(group + 8);
            end = *(s16 *)(group + 0x18);
            baseIndex = *(s16 *)(group + 6);
            flags = *(s32 *)(group + 0xC);
            vertex = startVertex;
            if (vertex < end) {
                s32 maskOffset;
                s32 masked;

                maskOffset = vertex * 4;
                masked = flags & 0x1080;
                do {
                    if ((masked != 0) && ((flags & 0x08010000) == 0)) {
                        *(u32 *)((u8 *)source->masks + maskOffset) = 0;
                        source->zMasks[vertex] = 0;
                    } else {
                        O35Vertex *v;
                        u8 *index;
                        s16 maxX;
                        s16 maxY;
                        s16 maxZ;
                        s16 minX;
                        s16 minY;
                        s16 minZ;
                        s32 i;
                        s32 bit;
                        s32 mask;
                        u8 zMask;
                        s16 start;
                        s16 step;
                        s16 value;
                        s16 origin;
                        s16 x;
                        s16 y;
                        s16 z;

                        maxX = -0x7FBC;
                        maxY = -0x7FBC;
                        maxZ = -0x7FBC;
                        minX = 0x7FBC;
                        minY = 0x7FBC;
                        minZ = 0x7FBC;
                        index = source->indices + vertex * 0x10;
                        i = 0;
                        do {
                            v = &source->vertices[index[1] + baseIndex];
                            x = v->x;
                            y = v->y;
                            z = v->z;
                            if (maxX < x) maxX = x;
                            if (x < minX) minX = x;
                            if (maxY < y) maxY = y;
                            if (y < minY) minY = y;
                            if (maxZ < z) maxZ = z;
                            if (z < minZ) minZ = z;
                            i++;
                            index++;
                        } while (i != 3);

                        bit = 1;
                        mask = 0;
                        origin = bounds->x0;
                        step = ((bounds->x1 - origin) >> 4) + 1;
                        value = step + origin;
                        start = origin;
                        x = 0;
                        do {
                            if ((value >= minX) && (maxX >= start)) {
                                mask |= bit;
                            }
                            value += step;
                            start += step;
                            bit *= 2;
                            x++;
                        } while (x < 16);

                        origin = bounds->y0;
                        step = ((bounds->z1 - origin) >> 4) + 1;
                        value = step + origin;
                        start = origin;
                        x = 0;
                        do {
                            if ((value >= minZ) && (maxZ >= start)) {
                                mask |= bit;
                            }
                            value += step;
                            start += step;
                            bit *= 2;
                            x++;
                        } while (x < 16);
                        *(u32 *)((u8 *)source->masks + maskOffset) = mask;

                        bit = 1;
                        zMask = 0;
                        step = ((bounds->y1 - bounds->z0) >> 3) + 1;
                        origin = bounds->z0;
                        value = step + origin;
                        start = origin;
                        x = 0;
                        do {
                            if ((value >= minY) && (maxY >= start)) {
                                zMask |= bit;
                            }
                            value += step;
                            start += step;
                            bit *= 2;
                            x++;
                        } while (x < 8);
                        source->zMasks[vertex] = zMask;
                    }
                    vertex++;
                    maskOffset += 4;
                } while (vertex < end);
            }
            groupIndex++;
            groupOffset += 0x10;
        } while (groupIndex < source->groupCount);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o035/overlay35BuildGridMasks/func_overlay_035_F0000770_1882450.s")
#endif
