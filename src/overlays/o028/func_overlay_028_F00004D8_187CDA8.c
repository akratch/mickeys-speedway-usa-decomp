typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;
typedef float f32;

typedef union {
    struct {
        u32 w0;
        u32 w1;
    } words;
    u32 force_alignment;
} Gfx;

typedef struct Overlay28Work Overlay28Work;
typedef struct Overlay28Owner Overlay28Owner;

typedef struct {
    s16 pitch;
    s16 yaw;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay28View;

typedef struct {
    s16 pitch;
    s16 yaw;
    s16 roll;
    u16 pad06;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} Overlay28Transform;

struct Overlay28Work {
    Overlay28Owner *related;
    f32 x;
    f32 y;
    f32 z;
    f32 scale;
    void *handle;
    f32 valueB;
    f32 scaleA;
    f32 scaleB;
    s16 angleA;
    s16 angleB;
    s16 stepA;
    s16 stepB;
    s16 stepC;
    s16 stepD;
    u8 object[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad3B;
    void (*reset)(Overlay28Work *);
    s16 intensity;
    s16 bufferIndex;
    u8 vertices[0x154];
};

struct Overlay28Owner {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay28Work *work;
    void **matrix;
};

extern u8 gOverlay28DisplayList28[];
extern Overlay28View *ext_o0_241dc(void);
extern void ext_o0_29e00(s32, Overlay28Owner *, f32 *, f32 *);
extern f32 ext_o0_6ec00(f32);
extern f32 ext_o0_9ab8(Overlay28Owner *);
extern void ext_o0_2409c(Gfx **, void *, Overlay28Transform *, f32, f32);
extern void ext_o0_34554(Gfx **, void *, s32, s32);
extern void ext_o0_241bc(Gfx **);

/*
 * PROVENANCE: Gfx.words and the _SHIFTL/display-list macro spelling were
 * adapted from Diddy Kong Racing and Jet Force Gemini include/f3ddkr.h.
 * Mickey's own bytes decide every type, constant, command and operation order.
 */
#define _SHIFTL(v, s, w) \
    ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define VERTEX_ADDRESS(work) \
    ((u32)(work) + ((u32)(work)->bufferIndex * 0xAAU) + 0x80000044U)
#define WRITE_PRIM(pkt, rgba) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xFA, 24, 8); \
    _g->words.w1 = rgba; \
}
#define WRITE_ENV(pkt, rgba) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xFB, 24, 8); \
    _g->words.w1 = rgba; \
}
#define WRITE_VERTEX(pkt, address) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(4, 24, 8) | \
                   _SHIFTL((((address) & 6) | 0x88), 16, 8) | \
                   _SHIFTL(0xB2, 0, 16); \
    _g->words.w1 = (u32)(address); \
}
#define WRITE_DL(pkt, address) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(5, 24, 8) | _SHIFTL(0xF1, 16, 8) | \
                   _SHIFTL(0x100, 0, 16); \
    _g->words.w1 = (u32)(address); \
}
#define WRITE_SYNC(pkt) { \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(0xE7, 24, 8); \
    _g->words.w1 = 0; \
}

#ifdef NON_MATCHING
void func_overlay_028_F00004D8_187CDA8(Gfx **dl, void *matrixArg,
                                        Overlay28Owner *owner) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distanceSquared;
    f32 ratio;
    Overlay28Transform transformHome;
    void *matrix;
    Overlay28Work *work;
    Overlay28Owner *related;
    Overlay28View *view;
    s32 mask;

    view = ext_o0_241dc();
    work = owner->work;
    related = work->related;
    if (related != 0) {
        ext_o0_29e00(1, related, &work->x, &transformHome.x);
        transformHome.x += related->x;
        transformHome.y += related->y;
        transformHome.z += related->z;
        dx = view->x - transformHome.x;
        dy = view->y - transformHome.y;
        dz = view->z - transformHome.z;
        distanceSquared = (((dx * dx) + (dz * dz)) + (dy * dy));
        ratio = distanceSquared;
        if (distanceSquared > 0.0f) {
            ratio = work->scale / ext_o0_6ec00(distanceSquared);
        }
        transformHome.pitch = -view->pitch;
        transformHome.yaw = view->yaw;
        transformHome.roll = 0;
        transformHome.scale = ext_o0_9ab8(related) * 0.125f;
        transformHome.x += dx * ratio;
        transformHome.y += dy * ratio;
        transformHome.z += dz * ratio;
        matrix = *owner->matrix;
        ext_o0_2409c(dl, matrixArg, &transformHome, 1.0f, 0.0f);
        ext_o0_34554(dl, matrix, 0x214, 0);

        mask = -0x100;
        WRITE_PRIM((*dl)++, (work->intensity & 0xFF) | mask);
        WRITE_ENV((*dl)++,
                  _SHIFTL(work->red, 24, 8) |
                  _SHIFTL(work->green, 16, 8) |
                  _SHIFTL(work->blue, 8, 8) | 0xFF);
        WRITE_VERTEX((*dl)++, VERTEX_ADDRESS(work));
        WRITE_DL((*dl)++, gOverlay28DisplayList28);
        WRITE_SYNC((*dl)++);
        if (work->intensity != 0xFF) {
            WRITE_PRIM((*dl)++, -1);
        }
        WRITE_ENV((*dl)++, mask);
        ext_o0_241bc(dl);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o028/func_overlay_028_F00004D8_187CDA8/func_overlay_028_F00004D8_187CDA8.s")
#endif
