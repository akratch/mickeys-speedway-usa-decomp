#include "PR/ultratypes.h"

/*
 * Pinned DKR v77/v80 object scans are negative. DKR's HUD audio updater is a
 * semantic relative for the timed handle/volume fade, not an exact donor.
 */
typedef struct Overlay95AudioState {
    void *handle;
    s32 volume;
} Overlay95AudioState;

typedef struct Overlay95Object {
    u8 pad00[0x64];
    Overlay95AudioState *audio;
    u8 pad68[0x18];
    s32 flags;
    s32 active;
    s32 timer;
} Overlay95Object;

typedef struct Overlay95Racer {
    s8 type;
    u8 pad001[0x190];
    s8 blocked;
    u8 pad192[0x16];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 mode;
} Overlay95Racer;

typedef struct Overlay95RacerObject {
    u8 pad00[0x64];
    Overlay95Racer *racer;
} Overlay95RacerObject;

typedef struct Overlay95QueryResult {
    Overlay95RacerObject *object;
} Overlay95QueryResult;

extern u8 gOverlay95Disabled;
extern f32 gOverlay95FadeRate;
extern Overlay95QueryResult *overlay95QueryReloc(void **found);
extern void overlay95ActivateReloc(Overlay95Object *object, s32 updateRate);
extern void overlay95EmitReloc(s32 type, s32 kind, f32 scale, Overlay95Racer *racer);
extern void *overlay95CreateReloc(void);
extern void overlay95ConfigureReloc(void *resource, f32 x, f32 y, f32 z, s32 count);
extern void overlay95StopReloc(void *handle);
extern void overlay95PlayReloc(s32 soundId, Overlay95AudioState *audio);
extern void overlay95VolumeReloc(s32 soundId, void *handle, s32 volume);

void overlay95Update(Overlay95Object *object, s32 updateRate) {
    Overlay95AudioState *audio;
    Overlay95QueryResult *queryResult;
    Overlay95Racer *racer;
    Overlay95RacerObject *racerOwner;
    void *found;
    void *resource;
    s32 volume;

    if (gOverlay95Disabled == 0) {
        audio = object->audio;
        queryResult = overlay95QueryReloc(&found);
        if (found != 0) {
            racerOwner = queryResult->object;
            racer = racerOwner->racer;
            if (!(racer->flags & 1) && racer->blocked == 0) {
                if (racer->mode >= 0x30 && racer->mode < 0x40) {
                    object->flags |= 1;
                    overlay95ActivateReloc(object, updateRate);
                    if (object->active == 0) {
                        object->active = 1;
                        if (object->timer == 0) {
                            overlay95EmitReloc(racer->type, 0x4B, 2.0f, racer);
                            resource = overlay95CreateReloc();
                            overlay95ConfigureReloc(resource, 0.5f, 1.0f, 0.5f, 15);
                            if (audio->handle != 0) {
                                overlay95StopReloc(audio->handle);
                            }
                            overlay95PlayReloc(0x31E, audio);
                            audio->volume = 0x7F;
                            object->timer = 0xF0;
                        }
                    }
                } else {
                    object->active = 0;
                }
            }
        }
        if (object->timer != 0) {
            object->timer -= updateRate;
            if (object->timer < 0x3C) {
                volume = audio->volume - (s32) (gOverlay95FadeRate * (f32) updateRate);
                audio->volume = volume;
                if (volume < 0) {
                    audio->volume = 0;
                    volume = 0;
                }
                overlay95VolumeReloc(0x31E, audio->handle, volume & 0xFF);
            }
            if (object->timer <= 0) {
                object->timer = 0;
                if (audio->handle != 0) {
                    overlay95StopReloc(audio->handle);
                }
            }
        }
    }
}
