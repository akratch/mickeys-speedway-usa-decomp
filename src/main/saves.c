/*
 * Save-device and rumble support -- ROM 0x2C8C0-0x2ECA0.
 *
 * PROVENANCE -- the translation-unit identity and the descriptive names used
 * below are adapted from Jet Force Gemini's public decompilation, src/saves.c.
 * Mickey's function order, sizes and call graph establish the correspondence;
 * adapted C bodies are identified in docs/modules.md; all remaining functions
 * stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"
#include "PR/os_message.h"

extern u8 D_8007A2F8;
extern u8 D_8007A2F0;
extern u8 D_8007A2F4;
extern s32 D_8007A2E8;
extern s32 D_8007A2FC;
extern s32 D_8007A31C;
extern void *D_8007A280;
extern OSMesgQueue *D_800D21C0;

typedef struct SavesRecord {
    u8 pad00[0xC];
    s32 unkC;
    s32 unk10;
} SavesRecord;

typedef struct SavesBitWriter {
    s32 size;
    s32 unk04;
    u8 mask;
    u8 pad09[3];
    u8 *cursor;
    u8 *start;
} SavesBitWriter;

typedef struct SavesSlot {
    s32 unk00;
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    s32 unk08;
    u8 unk0C;
    u8 unk0D;
    u8 unk0E;
    u8 unk0F;
    s32 unk10;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    s32 unk18;
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 unk1F;
} SavesSlot;

typedef struct RumbleState {
    u8 state;
    u8 pad01[2];
    u8 flag;
    u8 pad04[0xA];
} RumbleState;

void mmFree(void *address);
void *func_8002B280();
SavesSlot *func_800291C4(void);
s32 joyGetController(s32 controllerIndex);
extern RumbleState D_800D2368[];
s32 osContStartReadData(OSMesgQueue *messageQueue);
extern s32 packReadFile(s32 controllerIndex, s32 fileNum, u8 *data,
                        s32 dataLength);
void rumbleStop(s32 controllerIndex, s32 arg1);
s32 func_800290A0(void);
void func_8006FEF0(s32 arg0, s32 type, void *data, s32 size);
void func_80070030(s32 arg0, u8 arg1, s32 arg2, s32 arg3);

/* PROVENANCE: body adapted from Jet Force Gemini's public decomp, src/saves.c:func_8004B070_4BC70. */
s32 func_8002BCC0(void) {
    s32 result;

    result = D_8007A2F8 != 0;
    if (result != 0) {
        return func_800290A0() == 0;
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:rumbleRumbles. */
void rumbleRumbles(s32 value) {
    D_8007A2F8 = value;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:rumbleProcessing, with Mickey's outer enable guard. */
void rumbleProcessing(s32 enabled) {
    if (D_8007A2E8 != 0) {
        if (enabled != 0 && D_8007A2F4 == 0) {
            D_8007A2F0 = 1;
            D_8007A2F4 = 1;
            return;
        }
        D_8007A2F4 = 0;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleStart.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:rumbleStop, with Mickey's force argument. */
void rumbleStop(s32 controllerIndex, s32 force) {
    s32 controllerNum;
    RumbleState *rumble;

    if (controllerIndex >= 0 && controllerIndex < 4) {
        controllerNum = joyGetController(controllerIndex);
        if (force != 0 ||
            (D_800D2368[controllerNum].state != 0 &&
             D_800D2368[controllerNum].state != 3)) {
            rumble = &D_800D2368[controllerNum];
            rumble->state = 3;
            rumble->flag = 1;
        }
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:rumbleKill. */
void rumbleKill(s32 arg0) {
    s32 i = 4;

    while (i--) {
        rumbleStop(i, arg0);
    }
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:rumbleUpdate. */
void rumbleUpdate(void) {
    D_8007A2F0 = 1;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002BF54.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleTick.s")
void func_8002C5F4(void) {
    D_8007A2E8 = 0;
    D_8007A2FC = 1;
}
SavesBitWriter *func_8002C60C(s32 size, s32 clear) {
    SavesBitWriter *writer;
    u8 *data;

    writer = func_8002B280(size + 0x14, 0x85, size);
    if (writer == NULL) {
        return NULL;
    }
    data = (u8 *) writer + 0x14;
    writer->size = size;
    writer->mask = 0x80;
    writer->cursor = data;
    writer->start = data;
    if (clear != 0) {
        while (size--) {
            writer->cursor[size] = 0;
        }
    }
    return writer;
}
#ifdef NON_MATCHING
void func_8002C69C(SavesBitWriter *writer, s32 value, s32 bitCount) {
    u32 mask;
    u32 bit;
    s32 isSet;
    u32 nextBit;
    u8 *nextCursor;
    u8 *cursor;

    if (bitCount != 0) {
        bit = 1 << (bitCount + 0x1F);
        do {
            mask = writer->mask;
            isSet = value & bit;
            nextBit = bit >> 1;
            bit = nextBit;
            if (mask == 0) {
                mask = 0x80;
                nextCursor = writer->cursor + 1;
                writer->cursor = nextCursor;
                *nextCursor = 0;
                writer->mask = 0x80;
            }
            if (isSet != 0) {
                cursor = writer->cursor;
                *cursor |= mask;
                mask = writer->mask;
            }
            writer->mask = (u8) (mask >> 1);
        } while (nextBit != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C69C.s")
#endif
#ifdef NON_MATCHING
void func_8002C70C(SavesBitWriter *reader, s32 *value, s32 bitCount) {
    u32 nextBit;
    u32 bit;
    u32 shiftedMask;
    u32 mask;

    if (bitCount != 0) {
        *value = 0;
        bit = 1 << (bitCount + 0x1F);
        do {
            mask = reader->mask;
            nextBit = bit >> 1;
            if (mask == 0) {
                reader->mask = 0x80;
                mask = 0x80U & 0xFF;
                reader->cursor++;
            }
            shiftedMask = mask;
            if (*reader->cursor & mask) {
                *value |= bit;
                shiftedMask = reader->mask;
            }
            bit = nextBit;
            reader->mask = (u8) (shiftedMask >> 1);
        } while (nextBit != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C70C.s")
#endif
s32 func_8002C788(SavesRecord *record) {
    return record->unk10;
}
s32 func_8002C790(SavesRecord *record) {
    return record->unkC = record->unk10;
}
void func_8002C79C(void *address) {
    mmFree(address);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packCalculateGameChecksum. */
s32 packCalculateGameChecksum(u8 *buffer, s32 count) {
    s32 checksum = 15;

    while (count--) {
        checksum += *buffer++;
    }
    return checksum;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C7EC.s")
void func_8002C8B4(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    u8 data[16];

    if (D_8007A31C != 0) {
        if (arg1 & 1) {
            func_8006FEF0(arg0, 0x3E, data, 8);
            D_8007A31C = 0;
        }
    } else if (!(arg1 & 1)) {
        func_8006FEF0(arg0, 0x3F, data, 8);
        D_8007A31C = 1;
    }
    func_80070030(arg0, (u8) arg1, arg2, arg3);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C94C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CB18.s")
void func_8002CCE4(void) {
    s32 i;
    s32 limit;
    SavesSlot *slot;

    slot = func_800291C4();
    i = 0;
    limit = 0x18;
    do {
        i++;
        slot->unk0C = 0;
        slot->unk0D = 0;
        slot->unk0E = 0;
        slot->unk08 = 0;
        slot->unk0F = 0;
        slot->unk14 = 0;
        slot->unk15 = 0;
        slot->unk16 = 0;
        slot->unk10 = 0;
        slot->unk17 = 0;
        slot->unk1C = 0;
        slot->unk1D = 0;
        slot->unk1E = 0;
        slot->unk18 = 0;
        slot->unk1F = 0;
        slot->unk04 = 0;
        slot->unk05 = 0;
        slot->unk06 = 0;
        slot->unk00 = 0;
        slot->unk07 = 0;
        slot++;
    } while (i != limit);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CD6C.s")
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packCalculateGlobalFlagsChecksum. */
s32 packCalculateGlobalFlagsChecksum(u8 *buffer) {
    s32 bytesToChecksum = 22;
    s32 checksum = 5;

    while (bytesToChecksum--) {
        checksum += *buffer++;
    }
    return checksum;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CE54.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CF0C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CF6C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packOpen.s")
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packClose. */
s32 packClose(s32 controllerIndex) {
    osContStartReadData(D_800D21C0);
    return 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packIsPresent.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packDirectory.s")
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packDirectoryFree. */
void packDirectoryFree(void) {
    if (D_8007A280 != NULL) {
        mmFree(D_8007A280);
    }
    D_8007A280 = NULL;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packFreeSpace.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packDeleteFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packOpenFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packReadFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packWriteFile.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packFileSize.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/font_codes_to_string.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/string_to_font_codes.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packGetFileType, while retaining Mickey's placeholder name. */
s32 func_8002E020(s32 controllerIndex, s32 fileNum) {
    s32 *data;
    s32 pad;
    s32 result = 1;

    data = func_8002B280(0x100, 0xFF);
    if (packReadFile(controllerIndex, fileNum, (u8 *) data, 0x100) == 0) {
        if (*data == 0x43484152) {
            result = 0;
        } else {
            result = 1;
        }
    } else {
        result = 1;
    }
    mmFree(data);
    return result;
}
