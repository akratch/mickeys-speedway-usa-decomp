#include "PR/ultratypes.h"
#include "game/memory.h"
#include "n_audio/mbi.h"

/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/textures.c:resetColourCycle. Mickey's layout and compiler output remain
 * authoritative. */
typedef struct ColourCycle {
    s32 unk0;
    s32 unk4;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    struct ColourCycle *unkC;
} ColourCycle;

typedef struct ColourCycleFrame8 {
    s32 unk0;
    s32 unk4;
} ColourCycleFrame8;

typedef struct ColourCycleEntry {
    s32 unk0;
    s32 unk4;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    s32 unkC;
} ColourCycleEntry;

typedef struct ColourCycleTable {
    s32 numberFrames;
    s32 totalTime;
    u8 frameData[1];
} ColourCycleTable;

/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/textures.c:resetMixCycle. Mickey's layout and compiler output remain
 * authoritative. */
typedef struct PulsatingLightDataFrame {
    u16 value;
    u16 time;
} PulsatingLightDataFrame;

typedef struct PulsatingLightData {
    u16 numberFrames;
    u16 currentFrame;
    u16 time;
    u16 totalTime;
    s32 outColorValue;
    PulsatingLightDataFrame frames[1];
} PulsatingLightData;

extern MemoryPoolSlot *func_8002B280(s32 size, u32 tag);
extern u32 *piRomLoad(u32 assetIndex);
extern s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset,
                             s32 size);
extern s32 byteswap32(u8 *address);
extern u8 *func_8004D7E0(u8 *compressed, u8 *output);
extern void mmFree(void *address);
extern Gfx D_8007BDB0[];
extern Gfx D_8007BDD8[];
extern Gfx D_8007BE00[];
extern Gfx D_7BE08[];
extern void viGetCurrentSize(s32 *width, s32 *height);
extern void func_80034920(Gfx **dList);

#define FRONTEND_DMA_DISPLAY_LIST(pkt, address, numberOfCommands) \
    { \
        Gfx *_g = (Gfx *) (pkt); \
        _g->words.w0 = 0x07000000 | ((numberOfCommands) << 16) | \
                       ((numberOfCommands) * 8); \
        _g->words.w1 = (u32) (address); \
    }

void func_80036A80(ColourCycle *cycle) {
    ColourCycle *temp_v0;

    temp_v0 = cycle->unkC;
    cycle->unk0 = 0;
    cycle->unk4 = 0;
    cycle->unk8 = temp_v0->unk8;
    cycle->unk9 = temp_v0->unk9;
    cycle->unkA = temp_v0->unkA;
    cycle->unkB = temp_v0->unkB;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37680/func_80036AB0.s")

void func_80036C60(PulsatingLightData *data) {
    s32 i;

    data->currentFrame = 0;
    data->time = 0;
    data->totalTime = 0;
    data->outColorValue = data->frames[0].value;
    for (i = 0; i < data->numberFrames; i++) {
        data->totalTime += data->frames[i].time;
    }
}

/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/textures.c:updateMixCycle. Mickey's layout and compiler output remain
 * authoritative. */
void func_80036CAC(PulsatingLightData *data, s32 timeDelta) {
    s32 thisFrameIndex;
    s32 nextFrameIndex;

    if (data->numberFrames > 1) {
        data->time += timeDelta;
        while (data->time >= data->totalTime) {
            data->time -= data->totalTime;
        }
        while (data->time >= data->frames[data->currentFrame].time) {
            data->time -= data->frames[data->currentFrame].time;
            data->currentFrame++;
            if (data->currentFrame >= data->numberFrames) {
                data->currentFrame = 0;
            }
        }
        thisFrameIndex = data->currentFrame;
        nextFrameIndex = thisFrameIndex + 1;
        if (nextFrameIndex >= data->numberFrames) {
            nextFrameIndex = 0;
        }

        data->outColorValue = data->frames[thisFrameIndex].value +
                              ((data->frames[nextFrameIndex].value * data->time) /
                               data->frames[thisFrameIndex].time);
    }
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/screen.c:screenLoad. Mickey's loader calls and bytes remain authoritative. */
s32 *func_80036DD0(s32 screenIndex) {
    s32 *screenTable;
    s32 start;
    s32 size;
    s32 count;
    s32 uncompressedSize;
    u8 *decompressedAddr;
    u8 *sp1C;
    u32 compressedAddr;

    screenTable = (s32 *) piRomLoad(0x14);
    for (count = 0; screenTable[count] != -1; count++) {}
    count--;
    if (count == 0) {
        mmFree(screenTable);
        return (u8 *) 0x80100000;
    }
    if ((screenIndex < 0) || (screenIndex >= count)) {
        screenIndex = 0;
    }
    size = screenTable[screenIndex + 1] - screenTable[screenIndex];
    start = screenTable[screenIndex];
    decompressedAddr = NULL;
    sp1C = (u8 *) func_8002B280(0x10, 0x90);
    if (sp1C != NULL) {
        piRomLoadSection(0x13, (u32) sp1C, start, 0x10);
        uncompressedSize = byteswap32(sp1C) + 0x80;
        mmFree(sp1C);
        decompressedAddr = (u8 *) func_8002B280(uncompressedSize, 0x90);
        if (decompressedAddr != NULL) {
            compressedAddr = (u32) ((decompressedAddr + uncompressedSize) - size);
            compressedAddr -= compressedAddr & 0xF;
            piRomLoadSection(0x13, compressedAddr, start, size);
            func_8004D7E0((u8 *) (compressedAddr & 0xFFFFFFFF), decompressedAddr);
        }
    }
    mmFree(screenTable);
    return decompressedAddr;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/screen.c:screenDraw. Mickey's command data, VI calls, and ABI remain
 * authoritative. */
void func_80036F08(Gfx **dList, u8 *screenAddress, s32 arg2) {
    s32 yl;
    s32 yPos;
    s32 xh;
    s32 xl;
    u32 dsdx;
    u32 dtdy;
    u32 dy;
    u32 width;
    u32 height;

    screenAddress += 0x10;
    viGetCurrentSize((s32 *) &width, (s32 *) &height);
    if (((width == 0x140) && (height == 0xF0)) || arg2 == 0) {
        yl = (height - 0xF0) << 15;
        xl = (width - 0x140) << 1;
        dy = (0xF0 << 16) / 40;
        xh = xl + (0x140 << 2);
        dsdx = (0x140 << 10) / 0x140;
        dtdy = (0xF0 << 10) / 0xF0;
        gSPDisplayList((*dList)++, D_8007BDB0);
    } else {
        yl = 0;
        xl = 0;
        dy = (height << 16) / 40;
        xh = width << 2;
        dsdx = (0x140 << 10) / width;
        dtdy = (0xF0 << 10) / height;
        gSPDisplayList((*dList)++, D_8007BDD8);
    }

    for (yPos = 0; yPos != 0xF0; yPos += 6) {
        (*dList)->words.w0 = *((u32 *) D_8007BE00);
        (*dList)->words.w1 = (u32) screenAddress;
        (*dList)++;
        FRONTEND_DMA_DISPLAY_LIST((*dList)++, D_7BE08, 6);
        gSPTextureRectangle((*dList)++, xl, yl >> 14, xh,
                            (s32) (yl + dy) >> 14, 0, 0, 0, dsdx, dtdy);
        screenAddress += 0x140 * 6 * 2;
        yl += dy;
    }
    func_80034920(dList);
}
