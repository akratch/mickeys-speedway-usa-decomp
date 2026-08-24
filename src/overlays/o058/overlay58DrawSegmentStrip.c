#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58StripVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay58StripVertex;

typedef struct Overlay58StripGfx {
    u32 w0;
    u32 w1;
} Overlay58StripGfx;

extern Overlay58StripGfx *gOverlay58StripDisplayListReloc;
extern Overlay58StripVertex *gOverlay58StripVertexCursorReloc;
extern u8 gOverlay58StripIndexPayload58Reloc[];
extern f32 overlay58SqrtReloc(f32 value);
extern void overlay58PrepareStripReloc(Overlay58StripGfx **displayList,
                                       void *resource, s32 mode, s32 arg3);

void overlay58DrawSegmentStrip(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1,
                               f32 z1, f32 limit) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 t;
    f32 dummy0;
    f32 dummy1;
    f32 dummy2;
    f32 dummy3;
    f32 zPerpendicular;
    f32 xPerpendicular;
    f32 hole58;
    f32 stripStep;
    f32 quadSpan;

    dx = x1 - x0;
    dy = y1 - y0;
    dz = z1 - z0;
    distance = overlay58SqrtReloc((dx * dx) + (dy * dy) + (dz * dz));

    zPerpendicular = (1.25f * dx) / distance;
    t = 0.0f;
    xPerpendicular = (1.25f * dz) / distance;
    stripStep = 12.0f / distance;
    quadSpan = 8.0f / distance;

    overlay58PrepareStripReloc(&gOverlay58StripDisplayListReloc, (void *)0,
                               5, 0);
    if (limit != 0.0f) {
    }

    while (t < limit) {
        Overlay58StripVertex *vertices;
        Overlay58StripVertex *vertex;
        Overlay58StripGfx *command;
        s32 vertexCommand;
        f32 next;
        f32 startX;
        f32 startZ;
        f32 endX;
        f32 endZ;
        f32 currentStep;
        s16 y;

        vertices = gOverlay58StripVertexCursorReloc;
        vertices++;
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices++;
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices++;
        vertices->a = 0xFF;
        vertices->b = 0xFF;
        vertices->g = 0xFF;
        vertices->r = 0xFF;
        vertices -= 3;
        vertices->r = 0xFF;
        vertices->g = 0xFF;
        vertices->b = 0xFF;
        vertices->a = 0xFF;
        vertices += 3;

        next = t + quadSpan;
        if (1.0f < next) {
            next = 1.0f;
        }
        currentStep = stripStep;
        if (next <= limit) {
            command = gOverlay58StripDisplayListReloc;
            vertexCommand =
                (((s32)gOverlay58StripVertexCursorReloc + 0x80000000) & 6) |
                0x20;
            gOverlay58StripDisplayListReloc = command + 1;
            command->w0 =
                (((vertexCommand & 0xFF) << 16) | 0x04000000) | 0x30;
            command->w1 =
                (s32)gOverlay58StripVertexCursorReloc + 0x80000000;

            command = gOverlay58StripDisplayListReloc;
            gOverlay58StripDisplayListReloc = command + 1;
            command->w0 = 0x05110020;
            command->w1 = (u32)&gOverlay58StripIndexPayload58Reloc[0];

            startX = x0 + (t * dx);
            startZ = z0 + (t * dz);
            y = (s16)y0;

            gOverlay58StripVertexCursorReloc->x =
                (s16)(startX - xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = y;
            gOverlay58StripVertexCursorReloc->z =
                (s16)(startZ + zPerpendicular);
            gOverlay58StripVertexCursorReloc++;

            gOverlay58StripVertexCursorReloc->x =
                (s16)(startX + xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = y;
            gOverlay58StripVertexCursorReloc->z =
                (s16)(startZ - zPerpendicular);
            gOverlay58StripVertexCursorReloc++;

            endZ = z0 + (next * dz);
            endX = x0 + (next * dx);
            gOverlay58StripVertexCursorReloc->x =
                (s16)(endX - xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = (s16)y0;
            gOverlay58StripVertexCursorReloc->z =
                (s16)(endZ + zPerpendicular);
            gOverlay58StripVertexCursorReloc++;

            gOverlay58StripVertexCursorReloc->x =
                (s16)(endX + xPerpendicular);
            gOverlay58StripVertexCursorReloc->y = y;
            gOverlay58StripVertexCursorReloc->z =
                (s16)(endZ - zPerpendicular);
            gOverlay58StripVertexCursorReloc++;
        }

        t += currentStep;
    }
}
