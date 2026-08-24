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
