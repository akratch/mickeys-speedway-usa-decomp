#include "PR/ultratypes.h"

typedef struct O63ChainEntry {
    s16 tableIndex;
    s16 value4;
    s16 value6;
} O63ChainEntry;

typedef struct O63Config {
    void *resource;
    s16 value4;
    s16 value6;
    s16 pad8;
    s16 randomA;
    s16 randomB;
    s16 randomC;
    u8 pad10[4];
} O63Config;

extern void o63InitAReloc(s32 value);
extern void o63InitBReloc(void);
extern void o63InitCReloc(void *resource);
extern void o63SetModeReloc(s32 mode);
extern void o63SetColorReloc(s32 red, s32 green, s32 blue, s32 alpha, s32 intensity);
extern void *o63CreateReloc(void *resource, s32 x, s32 y, s32 kind);
extern void o63AttachReloc(void *resource, s32 value);
extern s16 o63RandomReloc(s32 lower, s32 upper);
extern void o63ResetSlotReloc(s32 slot);

extern u8 gO63InitResource[];
extern u8 gO63CreateResource[];
extern void *gO63TableReloc[];
extern s32 gO63ExternalWordReloc;
extern u8 gO63ExternalByteReloc;
extern void *gO63LocalResource;
extern s32 gO63LocalWord;
extern f32 gO63LocalFloat;
extern s32 gO63SavedTableWord;
extern O63Config gO63Configs[];
extern O63ChainEntry gO63Chain[];
extern s32 gO63State18C;
extern s32 gO63State190;
extern s32 gO63State194;
extern s32 gO63State198;
extern s32 gO63State19C;

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 63 as none. */
void overlay63Initialize(void) {
    O63ChainEntry *chain;
    O63Config *config;
    s16 index;
    s32 slot;

    o63InitAReloc(0x16);
    o63InitBReloc();
    o63InitCReloc(gO63InitResource);
    gO63SavedTableWord = (s32)gO63TableReloc[6];
    o63SetModeReloc(3);
    o63SetColorReloc(0xFF, 0xFF, 0xFF, 0, 0xFF);
    gO63LocalResource = o63CreateReloc(gO63CreateResource, 0xA0, 0xC0, 0x204);
    gO63LocalWord = 0;
    gO63LocalFloat = 0.0f;
    o63AttachReloc(gO63LocalResource, 0);

    chain = gO63Chain;
    config = gO63Configs;
    if (chain->tableIndex != -1) {
        index = chain->tableIndex;
        do {
            config->resource = gO63TableReloc[index];
            config->value4 = chain->value4;
            config->value6 = chain->value6;
            config->randomA = o63RandomReloc(0, 0x8000);
            config->randomB = o63RandomReloc(0x600, 0xA00);
            config->randomC = o63RandomReloc(0x100, 0x300);
            if (o63RandomReloc(0, 1) == 0) {
                config->randomC = -config->randomC;
            }
            index = chain[1].tableIndex;
            chain++;
            config++;
        } while (index != -1);
    }

    gO63State18C = 0;
    gO63State190 = 1;
    gO63State194 = 0;
    gO63State198 = 0;
    gO63State19C = 0xF0;
    for (slot = 0; slot < 4; slot++) {
        o63ResetSlotReloc(slot);
    }
    gO63ExternalWordReloc = 0;
    gO63ExternalByteReloc = 0;
}
