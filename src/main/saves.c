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
#include "PR/os_pfs.h"

extern u8 D_8007A2F8;
extern u8 D_8007A2F0;
extern u8 D_8007A2F4;
extern u8 D_8007A2E4;
extern s32 D_8007A2E8;
extern s32 D_8007A2FC;
extern s32 D_8007A31C;
extern u8 D_8007A284[];
extern u8 D_8007A304[];
extern void *D_8007A280;
extern OSMesgQueue *D_800D21C0;
extern OSPfs D_800D21C8[];

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

typedef struct SavesEepromWriteState {
    s32 messageQueue;
    s32 unused;
} SavesEepromWriteState;

typedef struct SavesEepromReadState {
    s32 messageQueue;
    u8 unused[0x14];
} SavesEepromReadState;

typedef struct SavesGameWriteState {
    u32 footer[2];
    SavesBitWriter *writer;
    s32 messageQueue;
} SavesGameWriteState;

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
s32 packOpen(s32 controllerIndex);
s32 joyMessageQ(void);
s32 func_80070170(s32 messageQueue);
s32 mainResetPressed(void);
s32 func_8006FEF0(s32 arg0, u8 type, void *data, s32 size);
void func_80070030(s32 arg0, u8 arg1, void *arg2, s32 arg3);
s32 func_8002C7EC(s32 arg0, s32 arg1, void *arg2, s32 arg3);
void mainPreNMI(void);

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
/* Mickey-derived chunked save-device transfer. */
s32 func_8002C7EC(s32 arg0, s32 arg1, void *arg2, s32 arg3) {
    s32 result;
    u8 type;

    D_8007A31C = arg1 & 1;
    result = 0;
    if (arg3 > 0) {
        do {
            type = arg1;
            mainPreNMI();
            if (arg3 >= 0x21) {
                result = func_8006FEF0(arg0, type, arg2, 0x20);
                arg2 = (u8 *) arg2 + 0x20;
                arg3 -= 0x20;
                arg1 += 4;
            } else {
                result = func_8006FEF0(arg0, type, arg2, arg3);
                arg3 = 0;
            }
        } while (arg3 > 0 && result == 0);
    }
    return result;
}
void func_8002C8B4(s32 arg0, s32 arg1, void *arg2, s32 arg3) {
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
/* Mickey-derived reconstruction; JFG supplies only the neighboring saves TU
 * order, while Mickey's writer/checksum/I/O calls establish this body. */
void func_8002CD6C(void) {
    SavesGameWriteState state;

    state.writer = func_8002C60C(0x1C0, 1);
    func_8002CCE4();
    state.messageQueue = joyMessageQ();
    if (func_80070170(state.messageQueue) == 0) {
        func_8002C79C(state.writer);
        return;
    }
    state.footer[0] = packCalculateGameChecksum(
        (u8 *) func_8002C788((SavesRecord *) state.writer), 0x1C0);
    state.footer[1] = 0x12345678;
    if (mainResetPressed() == 0) {
        func_8002C8B4(state.messageQueue, 0,
                      (void *) func_8002C788((SavesRecord *) state.writer),
                      0x1C0);
        func_8002C8B4(state.messageQueue, 0x38, state.footer,
                      sizeof(state.footer));
    }
    func_8002C79C(state.writer);
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packCalculateGlobalFlagsChecksum. */
s32 packCalculateGlobalFlagsChecksum(u8 *buffer) {
    s32 bytesToChecksum = 22;
    s32 checksum = 5;

    while (bytesToChecksum--) {
        checksum += *buffer++;
    }
    return checksum;
}
/* PROVENANCE: adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packLoadGlobalFlagsEprom, using Mickey's record and I/O path. */
void func_8002CE54(void *globalFlags) {
    s32 failed;
    SavesEepromReadState state;
    u8 *src;
    u8 *dst;
    s32 bytesToCopy;

    failed = 0;
    state.messageQueue = joyMessageQ();
    if (func_80070170(state.messageQueue) == 0) {
        failed = 1;
    }
    if (failed == 0 &&
        func_8002C7EC(state.messageQueue, 0x39, globalFlags, 0x18) != 0) {
        failed = 1;
    }
    if (packCalculateGlobalFlagsChecksum(globalFlags) !=
        *(u16 *) ((u8 *) globalFlags + 0x16)) {
        failed = 1;
    }
    src = D_8007A304;
    dst = globalFlags;
    bytesToCopy = 0x17;
    if (failed != 0) {
        do {
            *dst++ = *src++;
        } while (bytesToCopy--);
        *(u16 *) ((u8 *) globalFlags + 0x16) =
            packCalculateGlobalFlagsChecksum(globalFlags);
    }
}
/* Mickey-derived reconstruction; JFG supplies only the neighboring TU order. */
void func_8002CF0C(void *globalFlags) {
    SavesEepromWriteState state;

    *(s16 *) ((u8 *) globalFlags + 0x16) =
        packCalculateGlobalFlagsChecksum(globalFlags);
    state.messageQueue = joyMessageQ();
    if (func_80070170(state.messageQueue) != 0 && mainResetPressed() == 0) {
        func_8002C8B4(state.messageQueue, 0x39, globalFlags, 0x18);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CF6C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packOpen.s")
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packClose. */
s32 packClose(s32 controllerIndex) {
    osContStartReadData(D_800D21C0);
    return 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packInit.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packIsPresent. */
s32 packIsPresent(s32 controllerIndex) {
    s32 ret;

    ret = packOpen(controllerIndex);
    packClose(controllerIndex);
    if (ret == 8) {
        D_8007A2E4 |= 1 << controllerIndex;
    }
    return ret;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packDirectory.s")
/* PROVENANCE: adapted from Jet Force Gemini's public decomp, src/saves.c:packDirectoryFree. */
void packDirectoryFree(void) {
    if (D_8007A280 != NULL) {
        mmFree(D_8007A280);
    }
    D_8007A280 = NULL;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packFreeSpace, with Mickey's status constants. */
s32 packFreeSpace(s32 controllerIndex, u32 *bytesFree, s32 *notesFree) {
    s32 ret;
    s32 bytesNotUsed;
    s32 maxNotes;
    s32 notesUsed;

    ret = packOpen(controllerIndex);
    if (ret == 0) {
        if (bytesFree != NULL) {
            ret = osPfsFreeBlocks(&D_800D21C8[controllerIndex], &bytesNotUsed);
            if (ret != 0) {
                packClose(controllerIndex);
                return 6;
            }
            *bytesFree = bytesNotUsed;
        }
        if (notesFree != NULL) {
            ret = osPfsNumFiles(&D_800D21C8[controllerIndex], &maxNotes,
                                &notesUsed);
            if (ret != 0) {
                packClose(controllerIndex);
                return 6;
            }
            if (notesUsed >= 16) {
                *notesFree = 0;
            } else {
                *notesFree = 16 - notesUsed;
            }
        }
    }

    packClose(controllerIndex);
    return ret;
}
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packDeleteFile. */
s32 packDeleteFile(s32 controllerIndex, s32 fileNum) {
    OSPfsState state;
    s32 ret;

    ret = packOpen(controllerIndex);
    if (ret != 0) {
        packClose(controllerIndex);
        return ret;
    }
    ret = 6;
    if (osPfsFileState(&D_800D21C8[controllerIndex], fileNum, &state) == 0) {
        if (osPfsDeleteFile(&D_800D21C8[controllerIndex], state.company_code,
                            state.game_code, (u8 *) state.game_name,
                            (u8 *) state.ext_name) == 0) {
            ret = 0;
        }
    }
    packClose(controllerIndex);
    return ret;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packOpenFile.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packReadFile. */
s32 packReadFile(s32 controllerIndex, s32 fileNum, u8 *data, s32 dataLength) {
    s32 readResult;

    readResult = osPfsReadWriteFile(&D_800D21C8[controllerIndex], fileNum,
                                    PFS_READ, 0, dataLength, data);
    if (readResult == 0) {
        return 0;
    }
    if (readResult == PFS_ERR_NOPACK || readResult == PFS_ERR_DEVICE) {
        return 1;
    }
    if (readResult == PFS_ERR_INCONSISTENT) {
        return 2;
    }
    if (readResult == PFS_ERR_ID_FATAL) {
        return 3;
    }
    if (readResult == PFS_ERR_INVALID) {
        return 5;
    }
    return 6;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/packWriteFile.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:packFileSize. */
s32 packFileSize(s32 controllerIndex, s32 fileNum, s32 *fileSize) {
    OSPfsState state;

    *fileSize = 0;
    if (osPfsFileState(&D_800D21C8[controllerIndex], fileNum, &state) == 0) {
        *fileSize = state.file_size;
        return 0;
    }
    return 6;
}
#ifdef NON_MATCHING
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:font_codes_to_string. */
char *font_codes_to_string(u8 *inString, char *outString, s32 stringLength) {
    s32 index = *inString;
    s32 roundedLength;
    s32 peel;
    char *ret = outString;

    while (index != 0 && stringLength != 0) {
        if (index < 66) {
            *outString = D_8007A284[index];
            outString++;
        } else {
            *outString = '-';
            outString++;
        }
        inString++;
        stringLength--;
        index = *inString;
    }
    if (stringLength != 0) {
        peel = -(stringLength & 3);
        roundedLength = peel + stringLength;
        if (peel != 0) {
            do {
                stringLength--;
                *outString++ = 0;
            } while (roundedLength != stringLength);
            if (stringLength == 0) {
                goto done;
            }
        }
        do {
            stringLength -= 4;
            outString[0] = 0;
            outString[1] = 0;
            outString[2] = 0;
            outString[3] = 0;
            outString += 4;
        } while (stringLength != 0);
    }
done:
    *outString = 0;
    return ret;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/font_codes_to_string.s")
#endif
#ifdef NON_MATCHING
/* PROVENANCE: body adapted from Jet Force Gemini's public decomp,
 * src/saves.c:string_to_font_codes. */
char *string_to_font_codes(char *inString, char *outString, s32 stringLength) {
    s32 i;
    char currentChar;
    char *ret;
    s32 peel;
    s32 roundedLength;

    ret = outString;
    while (*inString != 0 && stringLength != 0) {
        *outString = 0;
        for (i = 0; i < 65; i++) {
            currentChar = *inString;
            if (currentChar == D_8007A284[i]) {
                *outString = i;
                outString++;
                break;
            }
        }
        inString++;
        stringLength--;
    }
    if (stringLength != 0) {
        peel = -(stringLength & 3);
        roundedLength = peel + stringLength;
        if (peel != 0) {
            do {
                stringLength--;
                *outString++ = 0;
            } while (roundedLength != stringLength);
        }
        if (stringLength != 0) {
            do {
                stringLength -= 4;
                outString[0] = 0;
                outString[1] = 0;
                outString[2] = 0;
                outString[3] = 0;
                outString += 4;
            } while (stringLength != 0);
        }
    }
    *outString = 0;
    return ret;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/string_to_font_codes.s")
#endif
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
