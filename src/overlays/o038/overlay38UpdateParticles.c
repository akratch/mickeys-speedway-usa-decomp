typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct O38Particle {
    f32 velocity, x, y, z, dx, dy, dz;
} O38Particle;

typedef struct O38Pool {
    s32 count, alpha;
    O38Particle particles[20];
} O38Pool;

typedef struct O38Object {
    s16 type, pad02;
    f32 pad04, age, x, y, z;
    char pad18[0x4C];
    O38Pool *pool;
} O38Object;

extern f32 gO38AgeRate;
extern f32 gO38AccelerationPosition;
extern f32 gO38AccelerationVelocity;
extern void o38ReleaseObject(O38Object *object);

#define P(cursor, index) \
    (*(O38Particle *)((cursor) + 8 + (index) * sizeof(O38Particle)))

#define UPDATE_XYZ(particle, delta, position, velocity, yResult) do { \
    (particle).x += (particle).dx * (delta); \
    (yResult) = (particle).y + (particle).dy * (delta) + (position); \
    (particle).z += (particle).dz * (delta); \
    (particle).dy += (velocity); \
} while (0)

#define UPDATE_YZX(particle, delta, position, velocity, yResult) do { \
    (yResult) = (particle).y + (particle).dy * (delta) + (position); \
    (particle).z += (particle).dz * (delta); \
    (particle).x += (particle).dx * (delta); \
    (particle).dy += (velocity); \
} while (0)

#define UPDATE_XYZ_DEFER_Z(particle, delta, position, velocity, yResult, zResult) do { \
    (particle).x += (particle).dx * (delta); \
    (yResult) = (particle).y + (particle).dy * (delta) + (position); \
    (zResult) = (particle).z + (particle).dz * (delta); \
    (particle).dy += (velocity); \
} while (0)

/*
 * Plateau (2026-08-25, independently rechecked in the overlay 31/38/46 lane):
 * -O2 -mips2 with -Wab,-r4300_mul is exact-size at 202 words, with 159
 * differing words and the first mismatch at +0x20. The 119-point lattice
 * independently confirmed that flag boundary as best; omitting r4300_mul is
 * eight bytes short. Collapsing dt/loopDt into one lifetime was also eight
 * bytes short and worsened the result to 199 words, first at +0x0, so the
 * split delta and deferred stores remain the source-backed best candidate.
 * Direct typed updates, update-order changes, and carrying the next vertical
 * delta had previously worsened it as well. A two-thread bounded permuter
 * improved 11810 to 7405 in 602 seconds without reaching zero. No permitted
 * skeleton exceeds 0.067, leaving the target FP allocation and pipelined store
 * schedule without another supported structural lever.
 */
#ifdef NON_MATCHING
void func_overlay_038_F0000154_1885E64(O38Object *object, s32 ticks)
{
    O38Pool *pool = object->pool;
    char *cursor;
    f32 dt = (f32)ticks;
    f32 accelerationPosition, accelerationVelocity;
    f32 loopDt;
    f32 y0, y1, y2, y3, z2;
    s32 i;

    if (pool->alpha == 0) {
        o38ReleaseObject(object);
        return;
    }
    object->age += gO38AgeRate * dt;
    pool->alpha -= (s32)(4.25f * dt);
    if (pool->alpha < 0) {
        pool->alpha = 0;
    }
    accelerationPosition = gO38AccelerationPosition * dt;
    accelerationPosition *= dt;
    accelerationVelocity = gO38AccelerationVelocity * dt;
    loopDt = dt;
    cursor = (char *)pool;

    for (i = 0; i < 20; i += 4, cursor += 4 * sizeof(O38Particle)) {
        UPDATE_YZX(P(cursor, 0), loopDt, accelerationPosition, accelerationVelocity, y0);
        UPDATE_XYZ(P(cursor, 1), loopDt, accelerationPosition, accelerationVelocity, y1);
        UPDATE_XYZ_DEFER_Z(P(cursor, 2), loopDt, accelerationPosition,
                           accelerationVelocity, y2, z2);
        UPDATE_XYZ(P(cursor, 3), dt, accelerationPosition, accelerationVelocity, y3);
        P(cursor, 0).y = y0;
        P(cursor, 1).y = y1;
        P(cursor, 2).y = y2;
        P(cursor, 3).y = y3;
        P(cursor, 2).z = z2;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o038/overlay38UpdateParticles/func_overlay_038_F0000154_1885E64.s")
#endif
