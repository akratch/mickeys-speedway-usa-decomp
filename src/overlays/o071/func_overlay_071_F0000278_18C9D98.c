#include "PR/ultratypes.h"

typedef struct O71Vec3 {
    f32 x;
    f32 y;
    f32 z;
} O71Vec3;

typedef struct O71Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} O71Vertex;

typedef struct O71State {
    O71Vertex vertices[2][8];
    f32 centerX;
    f32 centerY;
    f32 centerZ;
    f32 extentX;
    f32 extentY;
    f32 radius;
    f32 scaleX;
    f32 scaleY;
    f32 lowerLimit;
    f32 upperLimit;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 flags;
    u8 bank;
    u16 status;
} O71State;

typedef struct O71Object {
    u8 pad0[0x64];
    O71State *state;
    u8 pad68[0x1C];
    s32 active;
} O71Object;

extern void *gOverlay71InitialResourceReloc;
extern u8 gOverlay71GlobalFlagReloc;
extern void func_80032BF0(void *resource, s32 mode, s32 flags);
extern f32 sqrtf(f32 value);

/* P5 plateau: workbench operand-mismatch, 28 positional words; normalized instructions, frame, registers, and schedule exact, first +0xA8.
 * Levers: while-loop control plus output pointer-increment order reached the stable shape; scalar/volatile types, scopes, and separate counters regressed.
 * Remains: 26 stack-offset operands and six relocation identities; no exact C path found within the attempt cap. */
#ifdef NON_MATCHING
void func_overlay_071_F0000278_18C9D98(O71Object *object) {
    O71Vec3 points[8];
    O71State *state;
    f32 halfX;
    f32 halfY;
    f32 factor;
    f32 radius;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distanceSquared;
    s32 count;
    s32 i;
    O71Vec3 *point;
    O71Vertex *vertex;

    func_80032BF0(gOverlay71InitialResourceReloc, 2, 2);
    state = object->state;
    state->bank ^= 1;
    state->status = 0;

    halfX = state->extentX * state->scaleX * 0.5f;
    halfY = state->extentY * state->scaleY * 0.5f;
    if ((halfX != 0.0f) && (halfY != 0.0f)) {
        gOverlay71GlobalFlagReloc = 1;
        points[0].x = -halfX;
        points[0].y = halfY;
        points[0].z = 0.0f;
        points[1].x = halfX;
        points[1].y = halfY;
        points[1].z = 0.0f;
        points[2].x = halfX;
        points[2].y = -halfY;
        points[2].z = 0.0f;
        points[3].x = -halfX;
        points[3].y = -halfY;
        points[3].z = 0.0f;

        state->status |= state->flags & 1;
        if (state->flags & 2) {
            factor = state->scaleX * state->scaleY;
            if (state->lowerLimit <= factor) {
                factor = (factor - state->lowerLimit) /
                         (state->upperLimit - state->lowerLimit);
                if (factor > 1.0f) {
                    factor = 1.0f;
                }
                radius = state->radius * factor;
                i = 0;
                point = points;
                while (i < 4) {
                    dx = point->x - state->centerX;
                    dy = point->y - state->centerY;
                    dz = point->z - state->centerZ;
                    distanceSquared = (dx * dx) + (dy * dy) + (dz * dz);
                    factor = distanceSquared;
                    if (distanceSquared != 0.0f) {
                        factor = radius / sqrtf(distanceSquared);
                    }
                    point[4].x = point->x + (dx * factor);
                    point[4].y = point->y + (dy * factor);
                    point[4].z = point->z + (dz * factor);
                    point++;
                    i++;
                }
                state->status |= state->flags & 6;
            }
        }

        count = 4;
        if (state->status & 2) {
            count = 8;
        }
        vertex = state->vertices[state->bank];
        i = 0;
        point = points;
        while (i < count) {
            vertex->x = point->x;
            vertex->y = point->y;
            vertex->z = point->z;
            if (object->active != 0) {
                if (i >= 4) {
                    vertex->red = state->red;
                    vertex->green = state->green;
                    vertex->blue = state->blue;
                } else {
                    vertex->alpha = state->alpha;
                }
            }
            vertex++;
            point++;
            i++;
        }
    }
    object->active = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o071/func_overlay_071_F0000278_18C9D98/func_overlay_071_F0000278_18C9D98.s")
#endif
