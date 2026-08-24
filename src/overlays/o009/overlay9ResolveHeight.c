#include "PR/ultratypes.h"
typedef struct O9Point { u8 p[0xC]; f32 x, y, z; } O9Point;
typedef struct O9Height { u8 p[0xC]; f32 height; } O9Height;
typedef struct O9Hit { f32 height; } O9Hit;
extern f32 D_54;
extern s32 ext_o0_1353c(f32, f32, s32, O9Hit ***);
extern void ext_o0_7cd8(O9Point *, f32, f32, f32);

void func_overlay_009_F0000F6C_18675E4(O9Point *point, O9Height *offset,
                                       s32 steps) {
    O9Hit **hits;
    f32 current, result, distance, candidate;
    s32 count, i;

    count = ext_o0_1353c(point->x, point->z, 0x1000, &hits);
    current = point->y;
    result = current;
    distance = 1000.0f;
    i = count - 1;
    if (count != 0) {
        do {
            candidate = current - hits[i]->height;
            if (candidate < 0.0f) candidate = 0.0f;
            if (candidate < distance) {
                distance = candidate;
                result = hits[i]->height + offset->height;
            }
        } while (i--);
    }
    candidate = current;
    while (steps--) {
        candidate += (result - candidate) * D_54;
    }
    result += 40.0f - offset->height;
    if (candidate < result) candidate = result;
    ext_o0_7cd8(point, 0.0f, candidate - current, 0.0f);
}
