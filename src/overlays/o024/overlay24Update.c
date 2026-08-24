#include "PR/ultratypes.h"

typedef struct Overlay24TargetState {
    s8 status;
    u8 pad001[0x191];
    u8 adjustment;
    u8 pad193[0x15];
    u16 flags;
} Overlay24TargetState;

typedef struct Overlay24Target {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x16];
    s16 copiedValue;
    u8 pad030[0x34];
    Overlay24TargetState *state;
} Overlay24Target;

typedef struct Overlay24State {
    u8 mode;
    s8 remaining;
    s16 phaseTicks;
    f32 height;
    f32 velocity;
    s32 progress;
    Overlay24Target *target;
} Overlay24State;

typedef struct Overlay24Object {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x10];
    f32 relationValue;
    u8 pad02C[2];
    s16 copiedValue;
    u8 pad030[0x34];
    Overlay24State *state;
    void **relationResource;
} Overlay24Object;

typedef struct Overlay24InputState {
    u8 mode;
} Overlay24InputState;

extern s16 gOverlay24InputFlagsReloc;
extern void overlay24QueueCleanupReloc(Overlay24Object *object);
extern s32 overlay24UpdateRelationReloc(void *resource, s32 *eventId,
                                        s32 limit, f32 *relationValue,
                                        s32 updateRate);
extern void overlay24EmitEventReloc(s32 eventId, void *handle);
extern Overlay24InputState *overlay24GetInputStateReloc(void);

void overlay24Update(Overlay24Object *object, s32 updateRate) {
    Overlay24State *state;
    s32 eventId;
    s32 integrationTicks;
    Overlay24Target *target;
    Overlay24TargetState *targetState;

    state = object->state;
    target = state->target;

    if (target == 0) {
        overlay24QueueCleanupReloc(object);
    } else {
        targetState = target->state;
        eventId = 9;
        overlay24UpdateRelationReloc(*object->relationResource, &eventId, 20,
                                     &object->relationValue, updateRate);

        integrationTicks = 0;
        while (updateRate != 0) {
            switch (state->mode) {
            case 0:
                state->mode = 1;
                state->phaseTicks = 0;
                state->progress = 0;
                state->height = 16.0f;
                state->velocity = 4.5f;
                break;

            case 1:
                if (integrationTicks == 0) {
                    integrationTicks = updateRate;
                }

                state->phaseTicks =
                    (s16)((u32)(s32)state->phaseTicks + (u32)updateRate);
                state->progress =
                    (s32)((u32)state->progress + ((u32)updateRate << 5));
                updateRate = 0;

                if (state->progress >= 257) {
                    state->progress = 256;
                }

                if (state->phaseTicks >= 15) {
                    if (((targetState->flags & 1) == 0) ||
                        (targetState->status == 0)) {
                        overlay24EmitEventReloc(9, 0);

                        if (((u16)gOverlay24InputFlagsReloc & 2) != 0) {
                            if (overlay24GetInputStateReloc()->mode != 1) {
                                if (targetState->adjustment > 0) {
                                    targetState->adjustment =
                                        (u8)(targetState->adjustment - 1);
                                }
                                goto adjustment_done;
                            }
                        }
                        if (targetState->adjustment < 20) {
                            targetState->adjustment =
                                (u8)(targetState->adjustment + 1);
                        }
                    adjustment_done:
                        ;
                    }

                    state->mode = 2;
                    state->remaining = (s8)((s32)state->remaining - 1);
                    updateRate = *(volatile s16 *)&state->phaseTicks;
                    updateRate -= 15;
                }
                break;

            default:
                if (integrationTicks == 0) {
                    integrationTicks = updateRate;
                }

                state->progress =
                    (s32)((u32)state->progress - ((u32)updateRate << 5));
                updateRate = 0;

                if (state->progress < 0) {
                    integrationTicks = 0;
                    if (state->remaining > 0) {
                        state->mode = 0;
                    } else {
                        overlay24QueueCleanupReloc(object);
                    }
                    state->progress = 0;
                }
                break;
            }
        }

        while (integrationTicks--) {
            state->height = state->height + state->velocity;
            state->velocity = state->velocity - 0.3f;
        }

        object->x = target->x;
        object->y = target->y;
        object->z = target->z;
        object->copiedValue = target->copiedValue;
    }
}
