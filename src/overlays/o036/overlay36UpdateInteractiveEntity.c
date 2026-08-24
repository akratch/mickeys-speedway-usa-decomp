#include "PR/ultratypes.h"

typedef struct Overlay36State {
    s32 value0;
    u16 timer4;
    u8 flags6;
    u8 usePosition7;
    u8 positionMode8;
    u8 pad9;
    u8 alphaA;
    u8 countdownB;
} Overlay36State;

typedef struct Overlay36Entity Overlay36Entity;
typedef struct Overlay36Found Overlay36Found;

typedef struct Overlay36Record {
    s16 kind0;
    u8 callbackArg2;
    u8 flags3;
    void (*timerCallback4)(Overlay36Entity *, s32);
    void (*normalCallback8)(Overlay36Found *, Overlay36Entity *, s32, u8);
    void (*alternateCallbackC)(Overlay36Found *, Overlay36Entity *);
    u8 pad10[0xC];
    u16 normalEffect1C;
    u16 alternateEffect1E;
    u8 pad20[0xC];
    f32 queryRadius2C;
    f32 minimumY30;
    f32 maximumY34;
    u8 pad38[2];
    s16 targetAlpha3A;
} Overlay36Record;

typedef struct Overlay36Marker {
    u8 pad0[4];
    u8 active4;
} Overlay36Marker;

typedef struct Overlay36FoundState {
    u8 pad000[0x1A0];
    Overlay36Record *record1A0;
} Overlay36FoundState;

struct Overlay36Found {
    u8 pad00[0x10];
    f32 y10;
    u8 pad14[0x50];
    Overlay36FoundState *state64;
};

typedef struct Overlay36Work {
    Overlay36FoundState *foundState;
    Overlay36Found *found;
    u8 pad08[0x24];
    s32 animationMode;
} Overlay36Work;

struct Overlay36Entity {
    s16 angle0;
    u8 pad02[4];
    s16 flags6;
    f32 scale8;
    f32 xC;
    f32 y10;
    f32 z14;
    u8 pad18[0x28];
    u8 *model40;
    u8 pad44[2];
    s16 kind46;
    u8 pad48[8];
    Overlay36Marker *marker50;
    u8 pad54[0x10];
    Overlay36State *state64;
    void **resource68;
    u8 pad6C[0x14];
    void *update80;
    u8 pad84[0xF];
    u8 modelIndex93;
};

extern Overlay36Record gOverlay36Records[];
extern u8 gOverlay36RemapEnabled;
extern s32 gOverlay36Elapsed;
extern Overlay36Entity *gOverlay36CurrentEntity;
extern Overlay36State *gOverlay36CurrentState;
extern s32 gOverlay36CurrentValue;

extern void overlay36ExpireReloc(void);
extern void overlay36UpdateReloc(Overlay36Entity *entity, s32 elapsed);
extern void overlay36ApplyPositionReloc(s32 value, u8 mode, f32 *x, f32 *y,
                                        f32 *z);
extern s32 overlay36QueryReloc(f32 x, f32 y, f32 z, f32 radius, s32 one,
                               Overlay36Found **found);
extern void overlay36CreateEffectReloc(u16 kind, f32 x, f32 y, f32 z,
                                       s32 mode, void *owner);
extern void overlay36ReleaseEntityReloc(Overlay36Entity *entity);
extern void overlay36AnimateReloc(void *resource, s32 *mode, s32 count,
                                  void *angles, s32 elapsed);

void overlay36UpdateInteractiveEntity(Overlay36Entity *entity,
                                       s32 elapsed) {
    Overlay36State *state;
    Overlay36Record *record;
    Overlay36Work work;
    u8 alpha;
    s32 recordCount;
    s32 kind;
    f32 deltaY;

    state = entity->state64;
    gOverlay36Elapsed = elapsed;
    gOverlay36CurrentEntity = entity;
    gOverlay36CurrentState = entity->state64;

    if (state->countdownB != 0) {
        if (elapsed >= state->countdownB) {
            state->countdownB = 0;
            overlay36ExpireReloc();
            return;
        }
        if (state->countdownB != 0xFF) {
            state->countdownB -= elapsed;
        }
    }

    kind = entity->kind46;
    if (gOverlay36RemapEnabled != 0) {
        if (kind == 0xEA) {
            kind = 0x1B;
        }
        if (kind == 0xEB) {
            kind = 0x86;
        }
    }

    record = gOverlay36Records;
    recordCount = 13;
    do {
        if (kind == record->kind0) {
            break;
        }
        record++;
    } while (recordCount--);

    if (state->timer4 != 0) {
        if (elapsed >= state->timer4) {
            state->timer4 = 0;
        } else {
            state->timer4 -= elapsed;
        }
        if (record->timerCallback4 != 0) {
            record->timerCallback4(entity, elapsed);
        }
    }

    if (state->value0 != 0) {
        alpha = state->alphaA;
        if (alpha < record->targetAlpha3A) {
            alpha = (state->alphaA = alpha + elapsed * 4);
            if (alpha > record->targetAlpha3A) {
                state->alphaA = record->targetAlpha3A;
                alpha = state->alphaA;
            }
        } else if (alpha > record->targetAlpha3A) {
            alpha = (state->alphaA = alpha - elapsed * 4);
            if (alpha < record->targetAlpha3A) {
                state->alphaA = record->targetAlpha3A;
                alpha = state->alphaA;
            }
        }
    } else {
        alpha = state->alphaA;
        if (alpha < 0x80) {
            alpha = (state->alphaA = alpha + elapsed * 4);
            if (alpha >= 0x81) {
                alpha = (state->alphaA = 0x80);
            }
        }
    }

    entity->scale8 = (*(f32 *)entity->model40 * (f32)alpha) * 0.0078125f;
    if ((s8)entity->model40[entity->modelIndex93 + 0x1E] == 1) {
        work.animationMode = 9;
        overlay36AnimateReloc(*entity->resource68, &work.animationMode, 0x14,
                              (u8 *)entity + 0x28, elapsed);
    } else {
        entity->angle0 += elapsed << 8;
    }

    if (entity->update80 != 0) {
        overlay36UpdateReloc(entity, elapsed);
    }

    if (state->usePosition7 != 0) {
        overlay36ApplyPositionReloc(state->value0, state->positionMode8,
                                    &entity->xC, &entity->y10, &entity->z14);
        return;
    }

    if (entity->marker50 != 0) {
        entity->marker50->active4 = 1;
    }

    if ((state->countdownB == 0) && !(entity->flags6 & 0x400)) {
        if (state->timer4 != 0) {
            gOverlay36CurrentValue = state->value0;
        } else {
            gOverlay36CurrentValue = 0;
        }

        if (overlay36QueryReloc(entity->xC, entity->y10, entity->z14,
                                record->queryRadius2C, 1, &work.found) != 0) {
            deltaY = work.found->y10 - entity->y10;
            if ((record->minimumY30 < deltaY) &&
                (deltaY < record->maximumY34)) {
                work.foundState = work.found->state64;
                if (state->flags6 & 2) {
                    if (record->alternateCallbackC != 0) {
                        record->alternateCallbackC(work.found, entity);
                    }
                    if (record->alternateEffect1E != 0) {
                        overlay36CreateEffectReloc(record->alternateEffect1E,
                                                   entity->xC, entity->y10,
                                                   entity->z14, 4, 0);
                    }
                } else {
                    if (record->normalCallback8 != 0) {
                        record->normalCallback8(
                            work.found, entity,
                            (s32)(record - gOverlay36Records),
                            record->callbackArg2);
                    }
                    if (record->normalEffect1C != 0) {
                        overlay36CreateEffectReloc(record->normalEffect1C,
                                                   entity->xC, entity->y10,
                                                   entity->z14, 4, 0);
                    }
                    if (!(record->flags3 & 1)) {
                        work.foundState->record1A0 = record;
                    }
                }
                if ((state->countdownB == 0) && (state->timer4 == 0)) {
                    overlay36ReleaseEntityReloc(entity);
                }
            }
        }
    }
}
