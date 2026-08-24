#ifndef _GAME_MODELS_H_
#define _GAME_MODELS_H_

#include "PR/ultratypes.h"

typedef struct ModelTexture {
    /* 0x00 */ void *texture;
    /* 0x04 */ u16 pad4;
    /* 0x06 */ s16 textureId;
} ModelTexture;

typedef struct ObjectModel {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numberOfTextures;
    /* 0x11 */ u8 pad11[7];
    /* 0x18 */ ModelTexture *textures;
    /* 0x1C */ u8 pad1C[0xC];
    /* 0x28 */ void *unk28;
    /* 0x2C */ u8 textureAnimationCount;
    /* 0x2D */ u8 pad2D[0x1F];
    /* 0x4C */ s16 references;
    /* 0x4E */ s8 animationCount;
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ void **animations;
    /* 0x54 */ u8 pad54[4];
    /* 0x58 */ void *unk58;
    /* 0x5C */ u8 pad5C[0xC];
    /* 0x68 */ void *unk68;
    /* 0x6C */ void *unk6C;
    /* 0x70 */ s32 nestedCount;
    /* 0x74 */ u8 pad74[4];
    /* 0x78 */ void **nestedAllocations;
} ObjectModel;

typedef struct ModelInstance {
    /* 0x00 */ ObjectModel *objModel;
} ModelInstance;

void modInitModels(void);
void modFreeModel(ModelInstance *modInst);
void func_80020278(ObjectModel *model);
void func_800203E0(ObjectModel *model);
void modelSetModelFlags(s32 flags);
s32 modelGetModelFlags(void);

#endif
