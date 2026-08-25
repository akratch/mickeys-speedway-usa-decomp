typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct O38Particle {
    f32 velocity;
    f32 x;
    f32 y;
    f32 z;
    s16 direction[4];
    f32 lifetime;
} O38Particle;

typedef struct O38Pool {
    s32 count;
    s32 alpha;
    O38Particle particles[20];
} O38Pool;

typedef struct O38Object {
    s16 type;
    s16 pad02;
    f32 pad04;
    f32 pad08;
    f32 x;
    f32 y;
    f32 z;
    char pad18[0x4C];
    O38Pool *pool;
} O38Object;

typedef struct O38Descriptor {
    char pad00[0xA];
    s16 type;
} O38Descriptor;

typedef struct O38DirectionInput {
    char pad00[0xC];
    s16 angles[2];
} O38DirectionInput;

extern s32 o38RandomRange(s32 minimum, s32 maximum);
extern void o38MakeDirection(s16 *source, s16 *destination);

/*
 * Plateau (2026-08-25, independently rechecked in the overlay 31/38/46 lane):
 * natural -O2 -mips2 -32 output is size- and opcode-shape exact but differs in
 * 7 of 85 words, first at +0x48. The 119-point flag lattice was neutral and
 * the closest permitted skeleton scored only 0.098. IDO folds the direction
 * address back to the pool base and schedules the particle/direction setup
 * later than the target; declaration initializers were codegen-inert, while
 * moving the pointer assignments ahead of the object update changed the saved
 * register family and worsened the candidate. Typed/byte-addressed forms,
 * coupling the particle to the zero loop offset, register hints, and loop
 * spellings likewise did not close the preserved pointer chain without
 * unsupported aliasing artifacts.
 */
#ifdef NON_MATCHING
void func_overlay_038_F0000000_1885D10(O38Object *object,
                                       O38Descriptor *descriptor)
{
    O38Pool *pool = object->pool;
    O38Particle *particle;
    s32 offset;
    s16 *direction;
    O38DirectionInput randomDirection;

    object->type = descriptor->type;
    particle = pool->particles;
    offset = 0;
    direction = particle->direction;
    pool->count = 60;
    pool->alpha = 255;
    for (; offset != 0x230; offset += sizeof(O38Particle)) {
        particle->x = object->x + (f32)o38RandomRange(-20, 20);
        particle->y = object->y;
        particle->z = object->z + (f32)o38RandomRange(-20, 20);
        particle->velocity = (f32)o38RandomRange(1, 10) / 10.0f;
        randomDirection.angles[0] = o38RandomRange(-0x7FFF, 0x7FFF);
        randomDirection.angles[1] = o38RandomRange(0x2000, 0x4000);
        particle->lifetime = (f32)o38RandomRange(20, 50) / -10.0f;
        o38MakeDirection(randomDirection.angles, direction);
        particle++;
        direction += sizeof(O38Particle) / sizeof(s16);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o038/func_overlay_038_F0000000_1885D10/func_overlay_038_F0000000_1885D10.s")
#endif
