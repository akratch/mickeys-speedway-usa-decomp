#include "PR/ultratypes.h"

typedef struct Overlay99Gfx {
    u32 w0;
    u32 w1;
} Overlay99Gfx;

typedef struct Overlay99RenderObject {
    s16 zero0;
    s16 zero2;
    s16 heading;
    s16 flags;
    u8 pad08[4];
    f32 x;
    f32 z;
    f32 y;
} Overlay99RenderObject;

typedef struct Overlay99Segment {
    f32 x0;
    f32 z0;
    f32 x1;
    f32 z1;
    u8 pad10[0x1A];
    s8 headingOffset;
    u8 pad2B;
    Overlay99RenderObject *object;
} Overlay99Segment;

extern s32 gOverlay99SegmentCount;
extern Overlay99Segment gOverlay99Segments[];
extern void *gOverlay99Texture;
extern void overlay99Begin(void **outA, s32 *outB);
extern void overlay99Setup(Overlay99Gfx **displayList, void *a, s32 b,
                           void *zero0, s32 zero1, void *a2, s32 b2);
extern void overlay99End(Overlay99Gfx **displayList);
extern s16 overlay99Angle(f32 x, f32 z);
extern f32 overlay99Sqrt(f32 squared);
extern void overlay99DrawObject(Overlay99Gfx **displayList, void *arg1,
                                s32 arg2, Overlay99RenderObject *object);
extern void func_overlay_099_F0000800_18D9DB0(
    Overlay99Gfx **displayList, void *arg1, s32 arg2,
    Overlay99RenderObject *object, f32 scale);

void overlay99RenderSegments(Overlay99Gfx **displayList, void *arg1, s32 arg2,
                             f32 scale) {
    struct {
        void *a;
        s32 b;
        u32 reserved;
    } setup;
    f32 dx;
    f32 dz;
    f32 length;
    Overlay99Segment *segment;
    Overlay99RenderObject *object;
    Overlay99Gfx *command;
    s32 initialized;
    s32 i;

    i = 0;
    segment = gOverlay99Segments;
    initialized = 0;
    if (gOverlay99SegmentCount > 0) {
        do {
            object = segment->object;
            if (object != 0) {
                if (initialized == 0) {
                    overlay99Begin(&setup.a, &setup.b);
                    initialized = 1;
                    command = *displayList;
                    *displayList = command + 1;
                    command->w0 = 0xBC000806;
                    command->w1 = (u32)gOverlay99Texture + 0x80000000;
                    overlay99Setup(displayList, setup.a, setup.b, 0, 0,
                                   setup.a, setup.b);
                    overlay99End(displayList);
                }

                dx = segment->x1 - segment->x0;
                dz = segment->z1 - segment->z0;
                object->zero0 = 0;
                object->zero2 = 0;
                object->heading = overlay99Angle(-dx, dz);
                dx *= scale;
                dz *= scale;
                object->x = segment->x0 + dx;
                object->z = segment->z0 + dz;
                object->y = 5.0f;
                length = overlay99Sqrt((dx * dx) + (dz * dz));
                if (segment->headingOffset != 0) {
                    object->heading += (s32)((f32)segment->headingOffset *
                                             65536.0f * scale);
                }
                object->flags &= ~0x400;
                overlay99DrawObject(displayList, arg1, arg2, object);
                object->flags |= 0x400;
                func_overlay_099_F0000800_18D9DB0(displayList, arg1, arg2,
                                                   object, length);
            }
            i++;
            segment++;
        } while (i < gOverlay99SegmentCount);
    }
}
