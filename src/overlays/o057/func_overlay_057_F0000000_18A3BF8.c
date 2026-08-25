#include "PR/ultratypes.h"

typedef union O57PrefixCommand {
    s32 word;
    u8 bytes[4];
} O57PrefixCommand;

typedef struct O57PrefixSourcePosition {
    s16 id;
    u8 pad02[6];
    f32 value;
    f32 x;
    f32 y;
    f32 z;
} O57PrefixSourcePosition;

typedef struct O57PrefixSource {
    u8 pad00[8];
    O57PrefixSourcePosition *position;
} O57PrefixSource;

typedef struct O57PrefixSpawnDesc {
    s16 kind;
    u8 type;
    u8 subtype;
    s16 x;
    s16 y;
    s16 z;
    s16 sourceId;
    s32 mode;
    f32 value;
} O57PrefixSpawnDesc;

typedef struct O57PrefixLocals {
    O57PrefixSpawnDesc objectDesc;
    u8 pad14[4];
    O57PrefixSpawnDesc sourceDesc;
    u8 pad2C[3];
    u8 mask;
} O57PrefixLocals;

typedef struct O57PrefixObject {
    u8 pad00[0x16];
    u8 flags;
    u8 pad17[0x21];
    s32 state3C;
    u8 pad40[0x28];
    void **node68;
} O57PrefixObject;

typedef struct O57PrefixNode {
    u8 pad00[8];
    s16 kind;
} O57PrefixNode;

typedef struct O57PrefixPlayer {
    u8 pad00[0x2A];
    s8 enabled;
    u8 pad2B[9];
} O57PrefixPlayer;

typedef struct O57PrefixConfig {
    u8 pad00[0x0C];
    s32 value0C;
    s32 value10;
    s32 value14;
    s32 value18;
    s32 value1C;
    u8 pad20[0x1CC];
    s32 value1EC;
    u8 pad1F0[4];
    s32 value1F4;
} O57PrefixConfig;

extern void **gO57PrefixAssetTableReloc;
extern void *gO57PrefixInitialResourceReloc;
extern void *gO57PrefixResource100;
extern s32 gO57PrefixState104;
extern void *gO57PrefixResource108;
extern void *gO57PrefixOutputArrayReloc;
extern void *gO57PrefixResource0;
extern void *gO57PrefixResourceF8;
extern void *gO57PrefixResource6C;
extern void *gO57PrefixResource70;
extern void *gO57PrefixResource74;
extern void *gO57PrefixResource78;
extern void *gO57PrefixResourceFC;
extern void *gO57PrefixGroupA[9];
extern void *gO57PrefixGroupB[9];
extern void *gO57PrefixGroupC[24];
extern void *gO57PrefixGroupD[6];
extern void *gO57PrefixGroupE[5];
extern s16 gO57PrefixIndicesA[9];
extern s16 gO57PrefixIndicesB[9];
extern s16 gO57PrefixIndicesC[24];
extern s16 gO57PrefixIndicesD[6];
extern O57PrefixConfig *gO57PrefixConfigReloc;
extern s16 gO57PrefixTable100[];
extern s16 gO57PrefixTable130[];
extern s32 gO57PrefixCopy1FC;
extern s32 gO57PrefixCopy21C;
extern s32 gO57PrefixCopy23C;
extern s32 gO57PrefixCopy25C;
extern s32 gO57PrefixCopy27C;
extern s32 gO57PrefixCopy28C;
extern s32 gO57PrefixCopy32C;
extern s32 gO57PrefixCopy34C;
extern s32 gO57PrefixFill29C[0x20];
extern s32 gO57PrefixState118;
extern s32 gO57PrefixState11C;
extern s32 gO57PrefixState124;
extern s32 gO57PrefixState128;
extern s32 gO57PrefixState12C;
extern s32 gO57PrefixState134;
extern s32 gO57PrefixState138;
extern s32 gO57PrefixState13C;
extern s32 gO57PrefixState144;
extern s32 gO57PrefixState148;
extern s32 gO57PrefixState14C;
extern s32 gO57PrefixState160;
extern s32 gO57PrefixState164;
extern s32 gO57PrefixGate50C;
extern O57PrefixCommand gO57PrefixCommand180;
extern s32 gO57PrefixPrimaryIds134[];
extern s32 gO57PrefixPrimaryValues17C[];
extern s16 gO57PrefixSetupTable1A8[];
extern O57PrefixPlayer gO57PrefixPlayers[];
extern u8 gO57PrefixEnabledMask;
extern u8 gO57PrefixCurrentMask;
extern u8 gO57PrefixExpectedByte;
extern u8 gO57PrefixCurrentByte;
extern u8 gO57PrefixLatchedByte;
extern u8 gO57PrefixChangedByte;
extern s32 gO57PrefixResetOnMaskChange;
extern u16 gO57PrefixControllerFlagsReloc;
extern s32 gO57PrefixMode188;
extern s32 gO57PrefixMode18C;
extern s32 gO57PrefixMode190;
extern s32 gO57PrefixState198;
extern s32 gO57PrefixState19C;
extern f32 gO57PrefixY110;
extern s16 gO57PrefixPositions3E8[][2];
extern O57PrefixObject *gO57PrefixObjects150[4];
extern s32 gO57PrefixSpecialModeReloc;
extern s32 gO57PrefixSpecialStateReloc;

extern void *o57PrefixAcquireInitialReloc(void);
extern void *o57PrefixAllocateOutputReloc(void);
extern void o57PrefixSelectHeapReloc(s32 heap);
extern void o57PrefixSetColorReloc(s32 r, s32 g, s32 b, s32 a, s32 extra);
extern void *o57PrefixCreateResourceReloc(void *asset, s32 size, s32 y, s32 flags);
extern void o57PrefixSetResourceModeReloc(void *resource, s32 mode);
extern void o57PrefixSetResourceAlphaReloc(void *resource, s32 alpha);
extern void o57PrefixInitializeTableReloc(s16 *table);
extern void o57PrefixPrepareReloc(s32 value);
extern O57PrefixSource *o57PrefixGetSourceReloc(s32 id);
extern O57PrefixObject *o57PrefixSpawnReloc(O57PrefixSpawnDesc *desc, s32 mode);
extern void o57PrefixSetupTableReloc(s16 *table, s32 count);
extern void o57PrefixResetReloc(void);
extern s32 o57PrefixGetInitialModeReloc(void);
extern void o57PrefixActivateIdReloc(u8 id);
extern O57PrefixObject *o57PrefixFindObjectReloc(u8 id);
extern void o57PrefixMaskChangedReloc(void);
extern void o57PrefixFinalizeReloc(void);
extern void o57PrefixConfigureObjectReloc(O57PrefixObject *object, s32 arg1,
                                          s32 arg2, s32 arg3);
extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);

/* Overlay 57 text +0x0000..+0x0954. */
/* Plateau: canonical -O2 -mips2 is 0x1C bytes smaller than the 0x954-byte
 * target and differs in 552 of 597 masked words, first at +0x8. The best
 * structure reproduces the target's s0-s3 save set and relative 0x30-byte
 * descriptor/mask layout, but IDO places that aggregate 0x24 bytes too high
 * in a 0x90-byte frame instead of the original 0x78-byte frame. Bounded
 * pointer-type, end-pointer, declaration-order, register, scope, and explicit
 * local-layout variants did not remove the reconstructed helper-local homes. */
#ifdef NON_MATCHING
void func_overlay_057_F0000000_18A3BF8(void) {
    O57PrefixLocals locals;

    gO57PrefixResource100 = gO57PrefixInitialResourceReloc;
    gO57PrefixState104 = 0;
    locals.mask = 0;
    gO57PrefixResource108 = gO57PrefixInitialResourceReloc;
    gO57PrefixOutputArrayReloc = o57PrefixAllocateOutputReloc();
    o57PrefixSelectHeapReloc(3);
    o57PrefixSetColorReloc(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    {
        void **assetTable;
        void **resource;
        void **resourceEnd;
        s16 *index;
        s32 i;

        assetTable = gO57PrefixAssetTableReloc;
        gO57PrefixResource0 = o57PrefixCreateResourceReloc(assetTable[0x40], 0xA0, -0x28, 4);
        gO57PrefixResourceF8 = o57PrefixCreateResourceReloc(assetTable[0x3C], 0xA0, -0x28, 4);
        gO57PrefixResource6C = o57PrefixCreateResourceReloc(assetTable[0x41], 0xA0, 0x104, 4);
        gO57PrefixResource70 = o57PrefixCreateResourceReloc(assetTable[0x42], 0xA0, 0x104, 4);
        gO57PrefixResource74 = o57PrefixCreateResourceReloc(assetTable[0x43], 0xA0, 0x104, 4);
        gO57PrefixResource78 = o57PrefixCreateResourceReloc(assetTable[0x44], 0xA0, 0x104, 4);
        gO57PrefixResourceFC = o57PrefixCreateResourceReloc(assetTable[0x6D], 0xA0, -0x20, 4);

        resource = gO57PrefixGroupA;
        resourceEnd = &gO57PrefixGroupA[9];
        index = gO57PrefixIndicesA;
        do {
            *resource = o57PrefixCreateResourceReloc(assetTable[*index], 0xA0, 0xBE, 0x204);
            o57PrefixSetResourceModeReloc(*resource, 0);
            resource++;
            index++;
        } while (resource < resourceEnd);

        resource = gO57PrefixGroupB;
        resourceEnd = &gO57PrefixGroupB[9];
        index = gO57PrefixIndicesB;
        do {
            *resource = o57PrefixCreateResourceReloc(assetTable[*index], 0xA0, 0xBE, 0x204);
            o57PrefixSetResourceModeReloc(*resource, 0);
            resource++;
            index++;
        } while (resource < resourceEnd);

        resource = gO57PrefixGroupC;
        resourceEnd = &gO57PrefixGroupC[24];
        index = gO57PrefixIndicesC;
        do {
            *resource = o57PrefixCreateResourceReloc(assetTable[*index], 0xA0, 0x104, 4);
            o57PrefixSetResourceModeReloc(*resource, 0);
            resource++;
            index++;
        } while (resource < resourceEnd);

        resource = gO57PrefixGroupD;
        resourceEnd = &gO57PrefixGroupD[6];
        index = gO57PrefixIndicesD;
        do {
            *resource = o57PrefixCreateResourceReloc(assetTable[*index], 0xA0, 0x104, 4);
            o57PrefixSetResourceModeReloc(*resource, 0);
            resource++;
            index++;
        } while (resource < resourceEnd);

        resource = gO57PrefixGroupE;
        resourceEnd = &gO57PrefixGroupE[5];
        i = 0;
        do {
            *resource = o57PrefixCreateResourceReloc(assetTable[(0xD8 / 4) + i], 0xA0, -0x28, 4);
            resource++;
            i++;
        } while (resource < resourceEnd);
    }

    o57PrefixSetResourceAlphaReloc(gO57PrefixResource0, 0xFF);
    o57PrefixSetResourceAlphaReloc(gO57PrefixResourceF8, 0xFF);
    o57PrefixSetResourceAlphaReloc(gO57PrefixResource6C, 0xFF);
    o57PrefixSetResourceAlphaReloc(gO57PrefixResource70, 0xFF);
    o57PrefixSetResourceAlphaReloc(gO57PrefixResource74, 0xFF);
    o57PrefixSetResourceAlphaReloc(gO57PrefixResource78, 0xFF);
    o57PrefixInitializeTableReloc(gO57PrefixTable100);
    o57PrefixInitializeTableReloc(gO57PrefixTable130);

    gO57PrefixCopy1FC = gO57PrefixConfigReloc->value0C;
    gO57PrefixCopy21C = gO57PrefixConfigReloc->value10;
    gO57PrefixCopy23C = gO57PrefixConfigReloc->value14;
    gO57PrefixCopy25C = gO57PrefixConfigReloc->value10;
    gO57PrefixCopy32C = gO57PrefixConfigReloc->value1EC;
    gO57PrefixCopy34C = gO57PrefixConfigReloc->value1F4;
    gO57PrefixCopy27C = gO57PrefixConfigReloc->value1C;
    gO57PrefixCopy28C = gO57PrefixConfigReloc->value1C;
    {
        s32 *fill;

        fill = gO57PrefixFill29C;
        do {
            fill[0] = gO57PrefixConfigReloc->value1C;
            fill[4] = gO57PrefixConfigReloc->value1C;
            fill[8] = gO57PrefixConfigReloc->value1C;
            fill[12] = gO57PrefixConfigReloc->value1C;
            fill += 16;
        } while (fill != &gO57PrefixFill29C[0x20]);
    }

    gO57PrefixState11C = 0;
    gO57PrefixState128 = -0x50;
    gO57PrefixState12C = 0;
    gO57PrefixState134 = 0x40000;
    gO57PrefixState138 = 0x41800;
    gO57PrefixState14C = 0;
    gO57PrefixState160 = 0;
    gO57PrefixState164 = 0;
    gO57PrefixGate50C = 0;
    gO57PrefixState148 = 0;
    gO57PrefixState13C = 0;

    {
        O57PrefixSource *source;
        O57PrefixObject *object;

        o57PrefixPrepareReloc(0x3C);
        source = o57PrefixGetSourceReloc(0x3C);
        if (source->position != 0) {
            locals.sourceDesc.type = 0x14;
            locals.sourceDesc.subtype = 0;
            locals.sourceDesc.x = (s16)source->position->x;
            locals.sourceDesc.y = (s16)source->position->y;
            locals.sourceDesc.z = (s16)source->position->z;
            locals.sourceDesc.sourceId = source->position->id;
            locals.sourceDesc.kind = 0x35;
            locals.sourceDesc.mode = 0;
            locals.sourceDesc.value = source->position->value;
            object = o57PrefixSpawnReloc(&locals.sourceDesc, 1);
            if (object != 0) {
                object->state3C = 0;
            }
            locals.sourceDesc.kind = 0x38;
            locals.sourceDesc.mode = 1;
            object = o57PrefixSpawnReloc(&locals.sourceDesc, 1);
            if (object != 0) {
                object->state3C = 0;
            }
        }
    }

    o57PrefixSetupTableReloc(gO57PrefixSetupTable1A8, 0xA);
    gO57PrefixState144 = 0;
    o57PrefixResetReloc();
    {
        s32 *id;
        s32 itemId;
        O57PrefixObject *object;

        switch (o57PrefixGetInitialModeReloc()) {
        case 4:
            gO57PrefixState118 = 0xB;
            gO57PrefixCommand180.word = 0x2E;
            o57PrefixActivateIdReloc(gO57PrefixCommand180.bytes[3]);
            object = o57PrefixFindObjectReloc(gO57PrefixCommand180.bytes[3]);
            if (object != 0) {
                object->flags |= 2;
            }
            gO57PrefixSpecialModeReloc = 0;
            gO57PrefixChangedByte = 1;
            break;
        case 10:
            gO57PrefixState118 = 7;
            id = gO57PrefixPrimaryIds134;
            while (*id != -1) {
                o57PrefixActivateIdReloc((u8)*id);
                itemId = *id;
                overlay57SetNodeValue(itemId, gO57PrefixPrimaryValues17C[itemId], 0.0070000002f);
                id++;
            }
            break;
        case 12:
            gO57PrefixState118 = 1;
            id = gO57PrefixPrimaryIds134;
            while (*id != -1) {
                o57PrefixActivateIdReloc((u8)*id);
                itemId = *id;
                overlay57SetNodeValue(itemId, gO57PrefixPrimaryValues17C[itemId], 0.0070000002f);
                id++;
            }
            gO57PrefixState11C = 1;
            break;
        case 17:
            gO57PrefixState118 = 0xA;
            gO57PrefixCommand180.word = 0x50;
            o57PrefixActivateIdReloc(gO57PrefixCommand180.bytes[3]);
            object = o57PrefixFindObjectReloc(gO57PrefixCommand180.bytes[3]);
            if (object != 0) {
                object->flags |= 2;
            }
            gO57PrefixSpecialStateReloc = 5;
            break;
        case 18:
            gO57PrefixState118 = 0x14;
            gO57PrefixCommand180.word = 0x54;
            o57PrefixActivateIdReloc(gO57PrefixCommand180.bytes[3]);
            object = o57PrefixFindObjectReloc(gO57PrefixCommand180.bytes[3]);
            if (object != 0) {
                object->flags |= 2;
            }
            if (gO57PrefixSpecialModeReloc == 1) {
                gO57PrefixSpecialStateReloc = 7;
            } else {
                gO57PrefixSpecialStateReloc = 6;
            }
            break;
        }
    }

    {
        s32 i;

        i = 0;
        do {
            if (gO57PrefixPlayers[i].enabled != 0) {
                locals.mask = (u8)(locals.mask | (1 << i));
            }
            i++;
        } while (i < 4);
    }

    if ((gO57PrefixEnabledMask != locals.mask) ||
        (gO57PrefixExpectedByte != gO57PrefixCurrentByte)) {
        o57PrefixMaskChangedReloc();
        gO57PrefixResetOnMaskChange = 0;
        gO57PrefixEnabledMask = locals.mask;
        gO57PrefixChangedByte = 1;
        gO57PrefixLatchedByte = gO57PrefixCurrentByte;
    }

    gO57PrefixMode188 = 0;
    gO57PrefixMode18C = ((((gO57PrefixControllerFlagsReloc & 0x1C0) >> 6) < 3) ^ 1);
    if (gO57PrefixMode18C != 0) {
        gO57PrefixMode190 = 3;
    } else {
        gO57PrefixMode190 = 2;
    }
    o57PrefixFinalizeReloc();
    gO57PrefixState198 = 0;
    gO57PrefixState19C = 0;
    gO57PrefixY110 = -140.0f;
    gO57PrefixState124 = 0;

    {
        O57PrefixObject *object;
        s32 i;

        i = 0;
        do {
            locals.objectDesc.kind = 0x138;
            locals.objectDesc.type = 0xE;
            locals.objectDesc.z = 0;
            locals.objectDesc.subtype = 0;
            locals.objectDesc.sourceId = 0x80;
            locals.objectDesc.mode = 0;
            locals.objectDesc.x = gO57PrefixPositions3E8[i][0];
            locals.objectDesc.y = gO57PrefixPositions3E8[i][1];
            object = o57PrefixSpawnReloc(&locals.objectDesc, 0);
            gO57PrefixObjects150[i] = object;
            (*(O57PrefixNode **)object->node68)->kind = 2;
            o57PrefixConfigureObjectReloc(object, 0, 0, 0);
            i++;
        } while (i != 4);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0000000_18A3BF8/func_overlay_057_F0000000_18A3BF8.s")
#endif
