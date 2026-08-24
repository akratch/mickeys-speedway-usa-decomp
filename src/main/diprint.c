/*
 * Resident formatting and debug-text system -- ROM 0x43470-0x45760.
 *
 * PROVENANCE: The TU correspondence and names are adapted from Jet Force
 * Gemini's public src/diprint.c and its built diprint.c.o. Diddy Kong Racing's
 * public built unused_string.c.o/printf.c.o independently identify strcpy,
 * memset and sprintf byte-for-byte, and its public src/printf.c supplies the
 * debug_text_width name for the one routine absent from JFG. Mickey's bytes,
 * call graph and strings decide the mapping. Adapted bodies carry a
 * point-of-use PROVENANCE note.
 */

#include "PR/ultratypes.h"
#include "libc/stdarg.h"

s32 vsprintf(char *s, const char *format, va_list arg);
void *func_80034448(s16 resourceId);

extern char D_800D4150[];
extern char *D_8007CE94;
extern u16 D_800D4A5C;
extern u16 D_800D4A5E;
extern s32 D_8007CE90;
extern void *D_800D4A50;
extern void *D_800D4A54;
extern void *D_800D4A58;
extern s32 D_800D4A6C;
extern s32 D_800D4A70;
extern s32 D_800D4A74;
extern s32 D_800D4A78;
extern u16 D_800D4A80;
extern u16 D_800D4A82;

/* PROVENANCE: body adapted from DKR src/unused_string.c:strcpy. */
char *strcpy(char *src, const char *dest) {
    char *ret = src;

    while ((*src++ = *dest++) != '\0') {}
    return ret;
}
/* PROVENANCE: body adapted from DKR src/unused_string.c:memset. */
void *memset(void *s, int c, size_t n) {
    unsigned char *ret = s;

    while (n-- > 0) {
        *ret++ = c;
    }
    return s;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/_itoa.s")
/* PROVENANCE: body adapted from JFG src/diprint.c:sprintfSetSpacingCodes. */
void sprintfSetSpacingCodes(s32 setting) {
    D_8007CE90 = setting;
}
/* PROVENANCE: body adapted from DKR src/printf.c:sprintf. */
s32 sprintf(char *s, const char *format, ...) {
    va_list arg;
    s32 done;

    va_start(arg, format);
    done = vsprintf(s, format, arg);
    va_end(arg);

    return done;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/vsprintf.s")
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfInit. */
void diPrintfInit(void) {
    D_800D4A50 = func_80034448(0);
    D_800D4A54 = func_80034448(1);
    D_800D4A58 = func_80034448(2);
    D_8007CE94 = D_800D4150;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintf. */
s32 diPrintf(const char *format, ...) {
    va_list args;
    s32 written;

    va_start(args, format);
    if ((D_8007CE94 - D_800D4150) > 0x800) {
        return -1;
    }
    sprintfSetSpacingCodes(1);
    written = vsprintf(D_8007CE94, format, args);
    sprintfSetSpacingCodes(0);
    if (written > 0) {
        D_8007CE94 = &D_8007CE94[written] + 1;
    }
    return 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/diPrintfAll.s")
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfSetCol. */
void diPrintfSetCol(u8 red, u8 green, u8 blue, u8 alpha) {
    *D_8007CE94 = 0x81;
    D_8007CE94++;
    *D_8007CE94 = red;
    D_8007CE94++;
    *D_8007CE94 = green;
    D_8007CE94++;
    *D_8007CE94 = blue;
    D_8007CE94++;
    *D_8007CE94 = alpha;
    D_8007CE94++;
    *D_8007CE94 = 0;
    D_8007CE94++;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfSetBG. */
void diPrintfSetBG(u8 red, u8 green, u8 blue, u8 alpha) {
    *D_8007CE94 = 0x85;
    D_8007CE94++;
    *D_8007CE94 = red;
    D_8007CE94++;
    *D_8007CE94 = green;
    D_8007CE94++;
    *D_8007CE94 = blue;
    D_8007CE94++;
    *D_8007CE94 = alpha;
    D_8007CE94++;
    *D_8007CE94 = 0;
    D_8007CE94++;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:diPrintfSetXY. */
void diPrintfSetXY(u16 x, u16 y) {
    u16 tempX;
    u16 tempY;

    *D_8007CE94 = 0x82;
    D_8007CE94++;
    *D_8007CE94 = x & 0xFF;
    D_8007CE94++;
    tempX = x >> 8;
    *D_8007CE94 = tempX;
    D_8007CE94++;
    *D_8007CE94 = y & 0xFF;
    D_8007CE94++;
    tempY = y >> 8;
    *D_8007CE94 = tempY;
    D_8007CE94++;
    *D_8007CE94 = 0;
    D_8007CE94++;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_width.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_parse.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_background.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diprint/debug_text_character.s")
/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_bounds. */
void debug_text_bounds(void) {
    if (D_800D4A80 <= 320) {
        D_800D4A6C = 16;
        D_800D4A70 = D_800D4A80 - 16;
    } else {
        D_800D4A6C = 32;
        D_800D4A70 = D_800D4A80 - 32;
    }
    if (D_800D4A82 <= 240) {
        D_800D4A74 = 16;
        D_800D4A78 = D_800D4A82 - 16;
    } else {
        D_800D4A74 = 32;
        D_800D4A78 = D_800D4A82 - 32;
    }
}
/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_origin. */
void debug_text_origin(void) {
    D_800D4A5C = D_800D4A6C;
    D_800D4A5E = D_800D4A74;
}
/* PROVENANCE: body adapted from JFG src/diprint.c:debug_text_newline. */
void debug_text_newline(void) {
    D_800D4A5C = D_800D4A6C;
    D_800D4A5E += 11;
}
