#ifndef _GAME_MODELS_H_
#define _GAME_MODELS_H_

#include "PR/ultratypes.h"

typedef struct ObjectModel {
    /* 0x00 */ u8 pad0[0x4C];
    /* 0x4C */ s16 references;
} ObjectModel;

typedef struct ModelInstance {
    /* 0x00 */ ObjectModel *objModel;
} ModelInstance;

void modInitModels(void);
void modFreeModel(ModelInstance *modInst);
void modelSetModelFlags(s32 flags);
s32 modelGetModelFlags(void);

#endif
