typedef unsigned char u8; typedef short s16; typedef int s32; typedef float f32;
typedef struct Overlay28Work Overlay28Work; typedef struct Overlay28Owner Overlay28Owner;
struct Overlay28Work {
    Overlay28Owner *related; f32 x, y, z, valueA; void *handle;
    f32 valueB, scaleA, scaleB; s16 angleA, angleB, stepA, stepB, stepC, stepD;
    u8 object[0xC]; void (*reset)(Overlay28Work *); s16 intensity, bufferIndex;
};
struct Overlay28Owner {
    u8 pad00[0xC]; f32 x, y, z; u8 pad18[0x16]; s16 angle;
    u8 pad30[9]; u8 intensityScale; u8 pad3A[0x2A]; Overlay28Work *work;
    u8 pad68[0x29]; u8 suppressRelease;
};
typedef struct { u8 pad00[0x16A]; s16 intensity; } Overlay28Context;
extern void ext_o0_29e00(void *);
extern void overlay28UpdateVertices(Overlay28Work *);
extern void ext_o0_2b90(s32, f32, f32, f32, s32, void **);
extern void ext_o0_2d70(void *, f32, f32, f32);
extern void ext_o0_2c4c(void *, s32);
extern void ext_o0_6a50(Overlay28Owner *);
void overlay28UpdateWork(Overlay28Owner *owner, s32 updateRate) {
    Overlay28Work *work;
    Overlay28Context *context;
    Overlay28Owner *related;
    s16 sourceIntensity;
    work = owner->work;
    related = work->related;
    if (related != 0) {
        context = (Overlay28Context *)related->work;
        work->angleA += work->stepC * updateRate;
        work->angleB += work->stepD * updateRate;
        ext_o0_29e00(work->object);
        sourceIntensity = context->intensity;
        if (sourceIntensity < 0) work->intensity = 0;
        else if (sourceIntensity < 0x80) work->intensity = sourceIntensity * 2;
        else {
            work->intensity += updateRate * 0x10;
            if (work->intensity >= 0x101) work->intensity = 0x100;
        }
        work->intensity = (work->intensity * related->intensityScale) >> 8;
        owner->x = related->x; owner->y = related->y; owner->z = related->z;
        owner->angle = related->angle;
        overlay28UpdateVertices(work);
        if (work->handle == 0) {
            ext_o0_2b90(0x1C2, owner->x, owner->y, owner->z, 1, &work->handle);
        } else {
            s16 alpha;
            ext_o0_2d70(work->handle, owner->x, owner->y, owner->z);
            alpha = work->intensity >> 1;
            if (alpha >= 0x80) alpha = 0x7F;
            ext_o0_2c4c(work->handle, alpha & 0xFF);
        }
    }
    if ((owner->suppressRelease == 0) && ((work->intensity == 0) || (work->related == 0))) {
        ext_o0_6a50(owner);
    }
}
