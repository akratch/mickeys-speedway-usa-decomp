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
extern s32 D_8007A2E8;
extern s32 D_8007A2FC;
extern void *D_8007A280;
extern OSMesgQueue *D_800D21C0;

typedef struct SavesRecord {
    u8 pad00[0xC];
    s32 unkC;
    s32 unk10;
} SavesRecord;

void mmFree(void *address);
s32 osContStartReadData(OSMesgQueue *messageQueue);
void rumbleStop(s32 controllerIndex, s32 arg1);
s32 func_800290A0(void);

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
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleProcessing.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleStart.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/rumbleStop.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C60C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C69C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C70C.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C8B4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002C94C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CB18.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002CCE4.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/saves/func_8002E020.s")
