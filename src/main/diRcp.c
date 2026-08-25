/*
 * Display-list disassembler -- ROM 0x459C0-0x465B0 (VRAM 0x80044DC0).
 *
 * PROVENANCE: the translation-unit identity and descriptive function names
 * are adapted from Jet Force Gemini's public decompilation, src/diRcp.c.
 * Mickey's opcode-name strings, helper call graph and source-order-equivalent
 * function sequence establish the correspondence. The two unnamed unpackers
 * retain Mickey address names. The bodies remain extracted assembly.
 */

#include "PR/ultratypes.h"

typedef struct Gfx {
    struct {
        u32 w0;
        u32 w1;
    } words;
} Gfx;

extern void func_800453C4(Gfx *dList, s32 *w0_24_31, s32 *w0_16_23,
                          s32 *w0_0_15, s32 *w1);
extern s32 sprintf(char *buffer, const char *format, ...);
extern char D_80083388[];
extern char D_80082D58[];
extern char D_80082D64[];
extern char D_80082D70[];
extern char D_80082D7C[];
extern char D_80082D88[];
extern char D_80082D90[];
extern char D_80082D98[];
extern char D_80082DA4[];
extern char D_80082DB0[];
extern char D_80082DBC[];
extern char D_80082DCC[];
extern char D_80082DDC[];
extern char D_80082DEC[];
extern char D_80082DFC[];
extern char D_80082E10[];
extern char D_80082E20[];
extern char D_80082E30[];
extern char D_80082E40[];
extern char D_80082E50[];
extern char D_80082E5C[];
extern char D_80082E68[];
extern char D_80082E74[];
extern char D_80082E80[];
extern char D_80082E90[];
extern char D_80082E9C[];
extern char D_80082EB0[];
extern char D_80082EC0[];
extern char D_80082ED0[];
extern char D_80082EE0[];
extern char D_80082EEC[];
extern char D_80082EF8[];
extern char D_80082F08[];
extern char D_80082F14[];
extern char D_80082F24[];
extern char D_80082F30[];
extern char D_80082F40[];
extern char D_80082F4C[];
extern char D_80082F5C[];
extern char D_80082F68[];
extern char D_80082F78[];
extern char D_80082F84[];
extern char D_80082F94[];
extern char D_80082FA0[];
extern char D_80082FAC[];
extern char D_80082FB8[];
extern char D_80082FCC[];
extern char D_80082FE0[];
extern char D_80082FE8[];
extern char D_80082FFC[];
extern char D_80083010[];
extern char D_80083024[];
extern char D_8008303C[];
extern char D_80083048[];
extern char D_8008305C[];
extern char D_80083068[];
extern char D_80083074[];
extern char D_80083084[];

s32 diRcpVertex(Gfx *dList);
s32 diRcpReserved1(Gfx *dList);
s32 diRcpMatrix(Gfx *dList);
s32 diRcpReserved0(Gfx *dList, char *name);
s32 diRcpReserved2(Gfx *dList);
s32 diRcpMoveMem(Gfx *dList);
s32 diRcpDisplayList(Gfx *dList);
s32 diRcpStrNameMacro(Gfx *dList, char *name, char *macroName);
s32 diRcpPrimColor(Gfx *dList);
s32 diRcpColor(Gfx *dList, char *name, char *macroName);
s32 diRcpDmaOffsets(Gfx *dList, char *name);
s32 diRcpMoveWd(Gfx *dList);
s32 diRcpStrName(Gfx *dList, char *name);
s32 diRcpOtherMode(Gfx *dList, char *name);
s32 diRcpGeometryMode(Gfx *dList, char *name, char *macroName);

#ifdef NON_MATCHING
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpPrintDL. */
void diRcpPrintDL(Gfx *dList, s32 unused, s32 count) {
    u8 *gListIt;
    s32 i;

    gListIt = (u8 *)dList;
    for (i = 0; i < count; i++) {
        switch (gListIt[0]) {
            case 0x04:
                gListIt += diRcpVertex((Gfx *)gListIt);
                break;
            case 0x05:
                gListIt += diRcpReserved1((Gfx *)gListIt);
                break;
            case 0x00:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082D58);
                break;
            case 0x01:
                gListIt += diRcpMatrix((Gfx *)gListIt);
                break;
            case 0x02:
                gListIt += diRcpReserved0((Gfx *)gListIt, D_80082D64);
                break;
            case 0x03:
                gListIt += diRcpMoveMem((Gfx *)gListIt);
                break;
            case 0x06:
                gListIt += diRcpDisplayList((Gfx *)gListIt);
                break;
            case 0x07:
                gListIt += diRcpReserved2((Gfx *)gListIt);
                break;
            case 0x08:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082D70);
                break;
            case 0x09:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082D7C);
                break;
            case 0xC0:
                gListIt += diRcpStrNameMacro((Gfx *)gListIt, D_80082D88,
                                             D_80082D90);
                break;
            case 0xFF:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082D98);
                break;
            case 0xFE:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082DA4);
                break;
            case 0xFD:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082DB0);
                break;
            case 0xFC:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082DBC);
                break;
            case 0xFB:
                gListIt += diRcpColor((Gfx *)gListIt, D_80082DCC,
                                      D_80082DDC);
                break;
            case 0xFA:
                gListIt += diRcpPrimColor((Gfx *)gListIt);
                break;
            case 0xF9:
                gListIt += diRcpColor((Gfx *)gListIt, D_80082DEC,
                                      D_80082DFC);
                break;
            case 0xF8:
                gListIt += diRcpColor((Gfx *)gListIt, D_80082E10,
                                      D_80082E20);
                break;
            case 0xF7:
                gListIt += diRcpColor((Gfx *)gListIt, D_80082E30,
                                      D_80082E40);
                break;
            case 0xF6:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082E50);
                break;
            case 0xF5:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082E5C);
                break;
            case 0xF4:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082E68);
                break;
            case 0xF3:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082E74);
                break;
            case 0xF2:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082E80);
                break;
            case 0xF0:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082E90);
                break;
            case 0xEF:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082E9C);
                break;
            case 0xEE:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082EB0);
                break;
            case 0xED:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082EC0);
                break;
            case 0xEC:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082ED0);
                break;
            case 0xEB:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082EE0);
                break;
            case 0xEA:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082EEC);
                break;
            case 0xE9:
                gListIt += diRcpStrNameMacro((Gfx *)gListIt, D_80082EF8,
                                             D_80082F08);
                break;
            case 0xE8:
                gListIt += diRcpStrNameMacro((Gfx *)gListIt, D_80082F14,
                                             D_80082F24);
                break;
            case 0xE7:
                gListIt += diRcpStrNameMacro((Gfx *)gListIt, D_80082F30,
                                             D_80082F40);
                break;
            case 0xE6:
                gListIt += diRcpStrNameMacro((Gfx *)gListIt, D_80082F4C,
                                             D_80082F5C);
                break;
            case 0xE5:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082F68);
                break;
            case 0xE4:
                gListIt += diRcpStrName((Gfx *)gListIt, D_80082F78);
                break;
            case 0xF1:
            default:
                switch ((s8)gListIt[0]) {
                    case -65:
                        gListIt += diRcpDmaOffsets((Gfx *)gListIt,
                                                  D_80082F84);
                        break;
                    case -66:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80082F94);
                        break;
                    case -67:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80082FA0);
                        break;
                    case -68:
                        gListIt += diRcpMoveWd((Gfx *)gListIt);
                        break;
                    case -69:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80082FAC);
                        break;
                    case -70:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80082FB8);
                        break;
                    case -71:
                        gListIt += diRcpOtherMode((Gfx *)gListIt, D_80082FCC);
                        break;
                    case -72:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80082FE0);
                        break;
                    case -73:
                        gListIt += diRcpGeometryMode(
                            (Gfx *)gListIt, D_80082FE8, D_80082FFC);
                        break;
                    case -74:
                        gListIt += diRcpGeometryMode(
                            (Gfx *)gListIt, D_80083010, D_80083024);
                        break;
                    case -75:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_8008303C);
                        break;
                    case -76:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80083048);
                        break;
                    case -77:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_8008305C);
                        break;
                    case -78:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80083068);
                        break;
                    case -79:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80083074);
                        break;
                    default:
                        gListIt += diRcpStrName((Gfx *)gListIt, D_80083084);
                        break;
                }
                break;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpPrintDL.s")
#endif
/* PROVENANCE: body adapted from JFG src/diRcp.c::func_80066594_67194. */
void func_800453C4(Gfx *dList, s32 *w0_24_31, s32 *w0_16_23,
                   s32 *w0_0_15, s32 *w1) {
    *w0_24_31 = (dList->words.w0 >> 24) & 0xFF;
    *w1 = (dList->words.w0 >> 16) & 0xFF;
    *w0_0_15 = dList->words.w0 & 0xFFFF;
    *w0_16_23 = dList->words.w1;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::func_800665D8_671D8. */
void func_80045400(Gfx *dList, s32 *w0_24_31, s32 *w0_8_23,
                   s32 *w0_0_7, s32 *w1) {
    *w0_24_31 = (dList->words.w0 >> 24) & 0xFF;
    *w0_8_23 = (dList->words.w0 >> 8) & 0xFFFF;
    *w0_0_7 = dList->words.w0 & 0xFF;
    *w1 = dList->words.w1;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpVertex. */
s32 diRcpVertex(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[4];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpReserved1. */
s32 diRcpReserved1(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[4];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpMatrix. */
s32 diRcpMatrix(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[8];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpReserved0. */
s32 diRcpReserved0(Gfx *dList, char *name) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpReserved2. */
s32 diRcpReserved2(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[2];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpMoveMem. */
s32 diRcpMoveMem(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[2];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpDisplayList. */
s32 diRcpDisplayList(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[2];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpStrNameMacro. */
s32 diRcpStrNameMacro(Gfx *dList, char *name, char *macroName) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpPrimColor. */
s32 diRcpPrimColor(Gfx *dList) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpColor. */
s32 diRcpColor(Gfx *dList, char *name, char *macroName) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpDmaOffsets. */
s32 diRcpDmaOffsets(Gfx *dList, char *name) {
    if (dList) {
    }
    return 8;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpMoveWd.s")
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpStrName. */
s32 diRcpStrName(Gfx *dList, char *name) {
    char buffer[0x50];
    s32 pad[2];

    sprintf(buffer, D_80083388, name);
    return 8;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpOtherMode.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpGeometryMode.s")
