#include "PR/ultratypes.h"

typedef struct Overlay62Gfx {
    u32 word0;
    u32 word1;
} Overlay62Gfx;

typedef struct Overlay62Root {
    u8 pad00[0x25C];
    void *primary;
    void *items[6];
} Overlay62Root;

typedef struct Overlay62Transform {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[2];
    f32 scale;
    f32 screenX;
    f32 screenY;
    f32 zero0;
    u8 pad18[0x10];
    f32 zero1;
} Overlay62Transform;

extern f32 gOverlay62PulseReloc;
extern s32 gOverlay62InputMaskReloc;
extern u8 gOverlay62State[];
extern s32 gOverlay62Values[];
extern s32 gOverlay62Value8;
extern s32 gOverlay62ValueC;
extern Overlay62Root *gOverlay62Root;
extern Overlay62Gfx *gOverlay62DisplayListCursorReloc;
extern u8 gOverlay62ViewportReloc[];
extern u8 gOverlay62LayoutResourceAReloc[];
extern u8 gOverlay62LayoutResourceBReloc[];
extern u8 gOverlay62FirstItemReloc[];
extern u8 gOverlay62CommitContextReloc[];

#define gOverlay62Value4 gOverlay62Values[0]
#define gOverlay62Value10 (*(s32 *)(gOverlay62State + 0x10))
#define gOverlay62Handle14 (*(void **)(gOverlay62State + 0x14))
#define gOverlay62Handle18 (*(void **)(gOverlay62State + 0x18))

extern void overlay62SetHandleAlphaReloc(void *handle, s32 alpha);
extern void overlay62EntryColorReloc(s32 red, s32 green, s32 blue, s32 alpha);
extern void overlay62StartTransitionReloc(s32 type, s32 arg1, s32 arg2,
                                          s32 arg3, s32 arg4, s32 arg5);
extern void overlay62SetBackdropReloc(s32 red, s32 green, s32 blue, s32 alpha);
extern void overlay62PrepareDisplayReloc(Overlay62Gfx **commands,
                                         void *viewport);
extern void overlay62SelectLayerReloc(s32 layer);
extern void overlay62DrawLayoutReloc(Overlay62Gfx **commands, void *resourceA,
                                     void *resourceB,
                                     Overlay62Transform *transform,
                                     void *handle, s32 arg5, s32 alpha);
extern void overlay62SetRenderModeReloc(s32 mode);
extern void overlay62SetCombineModeReloc(s32 mode);
extern void overlay62SetGeometryReloc(s32 a, s32 b, s32 c, s32 d);
extern void overlay62SetPrimitiveColorReloc(s32 red, s32 green, s32 blue,
                                            s32 alpha, s32 intensity);
extern void overlay62DrawItemReloc(Overlay62Gfx **commands, s32 x, s32 y,
                                   void *item, s32 mode);
extern void overlay62CommitFrameReloc(s32 handle, s32 *state, s32 mode,
                                      void *context, s32 updateRate);
extern void overlay62DrawLabelReloc(Overlay62Gfx **commands, s32 *state,
                                    s32 x, s32 y, s32 red, s32 green,
                                    s32 blue, s32 alpha);

#define OVERLAY62_EMIT(commands_, op_, value_) do { \
    Overlay62Gfx *packet_ = *(commands_); \
    *(commands_) = packet_ + 1; \
    packet_->word0 = (op_); \
    packet_->word1 = (value_); \
} while (0)

void overlay62Update(s32 updateRate) {
    s32 alpha;
    volatile s32 screenBase;
    s32 intensity;
    register s32 red;
    s32 green;
    s32 blue;
    Overlay62Transform transform;

    if (gOverlay62Value8 > 0) {
        gOverlay62Value8 -= updateRate;
        if (gOverlay62Value8 < 0) {
            gOverlay62Value8 = 0;
        }

        alpha = 0xFF - gOverlay62Value8 * 8;
        screenBase = 0xF8;
        intensity = 0x78 + ((gOverlay62Value8 * 0xDC) >> 5);
        red = green = 0x40 + (((-gOverlay62Value8) << 6) >> 5);
        blue = 0x80 + (((-gOverlay62Value8) << 7) >> 5);

        overlay62SetHandleAlphaReloc(gOverlay62Handle14, alpha);
        overlay62EntryColorReloc((red | 0) & 0xFF, green & 0xFF, blue & 0xFF,
                                 green);
    } else if ((gOverlay62ValueC != 0) || (gOverlay62Value10 != 0)) {
        gOverlay62ValueC -= updateRate;
        if (gOverlay62ValueC <= 0) {
            gOverlay62ValueC = 0;
            overlay62StartTransitionReloc(0xC, 0, 0, 4, 1, 0);
            gOverlay62Value10 = 1;
        }

        alpha = gOverlay62ValueC * 8;
        screenBase = 0xF8;
        intensity = 0x154 + ((gOverlay62ValueC * -0xDC) >> 5);
        red = (gOverlay62ValueC << 6) >> 5;
        if ((gOverlay62ValueC << 6) >> 5) {
        }
        green = red;
        blue = (gOverlay62ValueC << 7) >> 5;
    } else {
        alpha = 0xFF;
        screenBase = 0xF8;
        intensity = 0x78;
        red = 0x40;
        green = 0x40;
        blue = 0x80;

        if ((gOverlay62InputMaskReloc & 0x9000) != 0) {
            gOverlay62ValueC = 0x20;
        }
    }

    overlay62SetBackdropReloc(red & 0xFF, green & 0xFF, blue & 0xFF, green);
    overlay62SetHandleAlphaReloc(gOverlay62Handle14, alpha);
    overlay62PrepareDisplayReloc(&gOverlay62DisplayListCursorReloc,
                                 gOverlay62ViewportReloc);

    OVERLAY62_EMIT(&gOverlay62DisplayListCursorReloc, 0xE7000000, 0);
    OVERLAY62_EMIT(&gOverlay62DisplayListCursorReloc, 0xFA000000, 0xFFFFFFFF);
    OVERLAY62_EMIT(&gOverlay62DisplayListCursorReloc, 0xFB000000, 0xFFFFFF00);

    transform.x = 0;
    transform.y = 0;
    transform.z = 0;
    transform.screenX = (f32)(screenBase - 0xA0);
    transform.screenY = (f32)(0x78 - intensity);
    transform.scale = 1.0f;
    transform.zero0 = 0.0f;
    transform.zero1 = 0.0f;

    overlay62SelectLayerReloc(0);
    overlay62DrawLayoutReloc(&gOverlay62DisplayListCursorReloc,
                             gOverlay62LayoutResourceAReloc,
                             gOverlay62LayoutResourceBReloc, &transform,
                             gOverlay62Handle18, 0, 0xFF);
    overlay62SetRenderModeReloc(1);
    overlay62SetCombineModeReloc(2);
    overlay62SetGeometryReloc(0, 0, 0, 0);
    overlay62SetPrimitiveColorReloc(0xFF, 0xFF, 0, 0xFF, alpha);

    overlay62DrawItemReloc(&gOverlay62DisplayListCursorReloc, 0xC4, 0x28,
                           gOverlay62FirstItemReloc, 4);
    overlay62DrawItemReloc(&gOverlay62DisplayListCursorReloc, 0x74, 0x50,
                           gOverlay62Root->items[0], 4);
    overlay62DrawItemReloc(&gOverlay62DisplayListCursorReloc, 0x74, 0x5A,
                           gOverlay62Root->items[1], 4);
    overlay62DrawItemReloc(&gOverlay62DisplayListCursorReloc, 0x74, 0x64,
                           gOverlay62Root->items[2], 4);
    overlay62DrawItemReloc(&gOverlay62DisplayListCursorReloc, 0x74, 0x6E,
                           gOverlay62Root->items[3], 4);
    overlay62DrawItemReloc(&gOverlay62DisplayListCursorReloc, 0x74, 0x78,
                           gOverlay62Root->items[4], 4);
    overlay62DrawItemReloc(&gOverlay62DisplayListCursorReloc, 0x74, 0xB4,
                           gOverlay62Root->items[5], 4);

    overlay62CommitFrameReloc(*(s32 *)(gOverlay62State + 4),
                              gOverlay62Values, 2,
                              gOverlay62CommitContextReloc, updateRate);
    gOverlay62Values[2] = (s32)(gOverlay62PulseReloc * 65536.0f);
    overlay62DrawLabelReloc(&gOverlay62DisplayListCursorReloc,
                            gOverlay62Values, 0x74, 0xCC,
                            0xFF, 0xFF, 0xFF, alpha);
}
