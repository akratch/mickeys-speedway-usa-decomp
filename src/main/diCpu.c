/*
 * CPU exception/debug monitor -- ROM 0x465B0-0x47A60 (VRAM 0x800459B0).
 *
 * PROVENANCE: the translation-unit identity and descriptive names are
 * adapted from Jet Force Gemini's public decompilation, src/diCpu.c. The
 * Tier-A diCpuTraceInit match, debug strings, direct callers and function
 * order establish the correspondence. JFG address-placeholder names are not
 * imported. The bodies remain Mickey's extracted assembly.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"
#include "game/memory.h"
#include "libc/stdarg.h"

typedef struct {
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[4];
    u32 words[68];
} MickeyEpcInfo;

extern s32 D_8007CFD8;
extern s32 D_8007CFDC;
extern u32 D_8007CFD0;
extern OSThread diCpuOSThread;
extern u64 diCpuThreadStack[];
extern f32 D_80083DBC;
extern OSMesgQueue D_800D5CD0;
extern OSMesg D_800D5CE8[8];
extern OSMesg D_800D5D08[8];
extern OSMesgQueue D_800D5D28;
extern void func_8004D5E0(s32 priority, OSMesgQueue *queue, OSMesg *messages,
                          s32 count);
extern void osViSetSpecialFeatures(u32 features);
extern void func_80045CAC(void);
extern s32 joyGetPressed(s32 controller);
extern void joyRead(s32 updateRate, s32 controllers);
extern void osWritebackDCacheAll(void);
extern s32 viGetVideoMode(void);
extern void viGetCurrentSize(s32 *width, s32 *height);
extern s32 runlinkGetAddressInfo(u32 address, s32 *moduleId,
                                 s32 *moduleAddress, u32 **outputAddress);
extern void render_epc_lock_up_display(MickeyEpcInfo *arg0);
extern void cpuXYPrintf(s32 x, s32 y, const char *format, ...);
extern void func_80046E00(void);
extern void func_80046BCC(s32 x, s32 y, char *text);
extern s32 vsprintf(char *text, const char *format, va_list args);
extern u16 D_8007D034[];
extern s32 func_80005820(s32 arg0);
extern s32 levelGetLevel(void);
extern s32 D_80000310;
extern s32 D_8007A210;
extern s32 D_8007A218;
extern s32 D_8007A21C;
extern s32 D_8007A220[];
extern s32 D_8007A1E0;
extern s32 D_8007A200;
extern s32 D_8007CFE0;
extern s32 D_8007CFE4;
extern s32 D_8007CFE8;
extern s32 D_8007CFEC[];
extern s32 D_8007D02C;
extern s32 D_8007D030;
extern u16 D_8007D2F0[];
extern u16 D_8007D2F8[];
extern u16 D_8007D300[];
extern char D_80083A80;
extern char D_80083A88;
extern char D_80083B2C[];
extern char D_80083B48[];
extern char D_80083B5C[];
extern char D_80083B78[];
extern char D_80083B84[];
extern char D_80083B90[];
extern char D_80083BA0[];
extern char D_80083BAC[];
extern char D_80083BB8[];
extern char D_80083BC8[];
extern char D_80083BE4[];
extern char D_80083BF4[];
extern char D_80083C04[];
extern char D_80083C0C[];
extern char D_80083C14[];
extern char D_80083C1C[];
extern char D_80083C28[];
extern char D_80083C38[];
extern char D_80083C48[];
extern char D_80083C58[];
extern char D_80083C68[];
extern char D_80083C7C[];
extern char D_80083C90[];
extern char D_80083CA4[];
extern char D_80083CB8[];
extern char D_80083CCC[];
extern char D_80083CE0[];
extern char D_80083CF4[];
extern char D_80083D08[];
extern char D_80083D1C[];
extern char D_80083D30[];
extern char D_80083D44[];
extern char D_80083D58[];
extern char D_80083D6C[];
extern char D_80083D80[];
extern char D_80083D8C[];
extern char D_80083D9C[];
extern void *D_800D5D40;
extern u8 D_800D5D48[];
extern s32 D_800D5DF0[];
extern s32 D_800D5E98[];
extern s32 D_800D5F40[];
extern s32 D_800D21B0;
extern s16 *D_800D2FA8;
extern s32 packWriteFile(s32 controllerIndex, s32 fileNumber, char *fileName,
                         char *fileExt, u8 *dataToWrite, s32 fileSize);

typedef struct {
    u8 pad0[0x44];
    s16 unk44;
} EpcDebugObject;

extern EpcDebugObject *D_8007A214;
void stop_all_threads_except_main(void);
void diCpuThread(void *unused);
void func_80045BBC(void *thread);
void func_80045D34();

/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceInit. */
void diCpuTraceInit(void) {
    osCreateThread(&diCpuOSThread, 0, diCpuThread, 0, diCpuThreadStack, 0xFF);
    osStartThread(&diCpuOSThread);
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuThread; Mickey's own
 * draft supplies its extra VI delay loop and exact event handling. */
void diCpuThread(void *unused) {
    s32 events;
    register s32 i;
    OSMesg message;
    register f32 divisor;
    register f32 sum;

    events = 0;
    osCreateMesgQueue(&D_800D5CD0, D_800D5CE8, 8);
    osSetEventMesg(12, &D_800D5CD0, (OSMesg)8);
    osSetEventMesg(10, &D_800D5CD0, (OSMesg)2);
    func_8004D5E0(150, &D_800D5D28, D_800D5D08, 8);
    for (divisor = D_80083DBC; ; ) {
        osRecvMesg(&D_800D5CD0, &message, OS_MESG_BLOCK);
        events |= (s32)message;
        if ((events & 8) || (events & 2)) {
            osViSetSpecialFeatures(0xA2);
            sum = 0.0f;
            i = 1000000;
            while (i--) {
                sum += (f32)i / divisor;
            }
            events &= ~8;
            stop_all_threads_except_main();
            func_80045CAC();
        }
    }
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::stop_all_threads_except_main. */
void stop_all_threads_except_main(void) {
    OSThread *thread = __osGetActiveQueue();

    while (thread->priority != -1) {
        if (thread->priority > OS_PRIORITY_IDLE && thread->priority < 128) {
            osStopThread(thread);
        }
        thread = thread->tlnext;
    }
}
#ifdef NON_MATCHING
/* PROVENANCE: body adapted from JFG src/diCpu.c::func_80066D28_67928;
 * Mickey's target fixes the dump-size calculation to use the copied range. */
/* Workbench plateau (retested 2026-08-28): two masked code words plus six
 * relocation identities, 60 instructions/frame exact, first +0x18. Earlier
 * symbolic-address, argument-hoist, phantom-pop, subtraction, flag-lattice,
 * and bounded-permutation levers remain closed. A scoped-local variant grew
 * the frame to -56; copy-size order/initializer and register-source variants
 * were baseline-equivalent. Remains: D_80705014/18/1C bindings and the final
 * packWriteFile temporary color. */
void func_80045BBC(void *thread) {
    s32 copySize;
    void *source;
    register u8 *destination;
    s32 writeSize;

    *(s32 *)0x80705014 = D_8007CFE8;
    *(s32 *)0x80705018 = D_8007CFE0;
    *(s32 *)0x8070501C = D_8007CFE4;
    destination = (u8 *)0x80705094;
    _bcopy(thread, destination, 0x230);
    destination += 0x200;
    source = *(void **)((u8 *)thread + 0xF4);
    copySize = 0x200;
    _bcopy(source, destination, copySize);
    D_800D5D40 = source;
    _bcopy(source, D_800D5D48, copySize);
    destination += 0x200;
    writeSize = (s32)destination + 0x7F900000;
    if (writeSize & 0x1F) {
        writeSize = (writeSize & ~0x1F) + 0x20;
    }
    packWriteFile(0, -1, &D_80083A80, &D_80083A88, (u8 *)0x80700000,
                  writeSize);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045BBC.s")
#endif
/* PROVENANCE: body adapted from JFG src/diCpu.c::func_80066E14_67A14. */
void func_80045CAC(void) {
    OSThread *thread;

    for (thread = __osGetActiveQueue(); thread->priority != -1;
         thread = thread->tlnext) {
        if (thread->priority > OS_PRIORITY_IDLE) {
            if ((thread->flags & 2) || (thread->flags & 1)) {
                break;
            }
        }
    }
    if (thread->priority != -1) {
        func_80045BBC(thread);
    }
    func_80045D34(thread);
}
#ifdef NON_MATCHING
/* Mickey-derived draft; JFG's closest peer, src/diCpu.c::func_80067880,
 * remains assembly-only and supplies no body. */
/* Workbench p4: structure-mismatch; 432 positional/438 raw words differ,
 * 451/459 instructions, first +0x0, frame -176 versus -168. Levers: oldPage
 * volatility and address-take; exact-size variants retained no frame fix. */
void func_80045D34(s32 arg0) {
    u32 oldPage;
    s32 pageCount;
    s32 moduleOffset;
    u32 printedValue;
    s32 selectedRegion;
    u32 address;
    s32 row;
    s32 *words;
    s32 *pageLabel;
    s32 buttons;
    s32 currentPage;
    s32 redraw;
    s32 nibble;
    s32 memoryIndex;
    s32 pageColumn;
    s32 index;
    s32 lines;
    s32 limit;
    s32 tag;
    u32 mask;
    u32 candidate;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;

    oldPage = -1U;
    currentPage = 0;
    redraw = 1;
    memoryIndex = 0;
    pageCount = 25;
    if (D_8007A200 == 0 || D_80000310 != 0x17D9) {
        while (1) {
        }
    }
    if (D_8007CFE8 == 0) {
        pageCount = D_8007CFE4 / 20;
        if ((D_8007CFE4 % 20) != 0) {
            pageCount++;
        }
    }
    selectedRegion = 0;
    pageCount += 5;
    D_8007D02C = viGetVideoMode() != 0;
    address = 0x80100000;
    nibble = 1;
    index = 0;
    do {
        func_80046E00();
        index++;
    } while (index != 100);
    if (D_8007A1E0 == 0) {
        osWritebackDCacheAll();
        while (1) {
        }
    }

    while (1) {
        joyRead(0, 2);
        buttons = joyGetPressed(0);
        if (buttons & 0x8000) {
            currentPage++;
        } else if (buttons & 0x4000) {
            currentPage--;
        }
        if (currentPage < 0) {
            currentPage = pageCount;
        }
        if (pageCount < currentPage) {
            currentPage = 0;
        }
        if (oldPage == 1) {
            if (buttons & 2) {
                redraw = 1;
                nibble++;
                if (nibble >= 6) {
                    nibble = 1;
                }
            }
            if (buttons & 1) {
                redraw = 1;
                nibble--;
                if (nibble <= 0) {
                    nibble = 5;
                }
            }
            if (buttons & 0xC) {
                mask = 0xF << (nibble * 4);
                if (buttons & 8) {
                    candidate = address + (1 << (nibble * 4));
                } else {
                    candidate = address - (1 << (nibble * 4));
                }
                candidate = (candidate & mask) | (address & ~mask);
                redraw = 1;
                if (candidate >= 0x803FFF60U) {
                    candidate = 0x803FFF60;
                }
                if (candidate < 0x80000451U) {
                    candidate = 0x80000450;
                }
                address = candidate;
                printedValue = candidate;
            }
        }
        if (oldPage == 5) {
            if (buttons & 0x800) {
                memoryIndex -= 18;
                redraw = 1;
                if (memoryIndex < 0) {
                    memoryIndex = 0;
                }
            }
            if (buttons & 0x400) {
                memoryIndex += 18;
                redraw = 1;
            }
            if ((buttons & 8) && memoryIndex > 0) {
                memoryIndex--;
                redraw = 1;
            }
            if (buttons & 4) {
                memoryIndex++;
                redraw = 1;
            }
            if (buttons & 0x2000) {
                selectedRegion ^= 1;
                redraw = 1;
            }
        }
        if (buttons & 0x10) {
            redraw = 1;
            D_8007D02C++;
            if (D_8007D02C >= 3) {
                D_8007D02C = 0;
            }
        }
        if (oldPage != (u32)currentPage || redraw != 0) {
            redraw = 0;
            switch (currentPage) {
                case 0:
                    oldPage = currentPage;
                    redraw = 0;
                    render_epc_lock_up_display((MickeyEpcInfo *)(u32)arg0);
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    func_80046E00();
                    pageColumn = 168;
                    if (currentPage == 1) {
                        words = (s32 *)(address + 0xA0);
                    } else if (currentPage == 2) {
                        words = D_800D5DF0;
                    } else if (currentPage == 3) {
                        words = D_800D5E98;
                    } else {
                        words = D_800D5F40;
                    }
                    do {
                        row = 20;
                        do {
                            words--;
                            cpuXYPrintf(pageColumn, (row + 3) * 8,
                                        "%08x:%08x", words, *words);
                            if (currentPage == 1 && row == 1) {
                                row = 0;
                            }
                            row--;
                        } while (row != 0);
                        pageColumn -= 148;
                    } while (pageColumn != -128);
                    oldPage = currentPage;
                    redraw = 0;
                    break;
                case 5:
                    index = 0;
                    row = 44;
                    func_80046E00();
                    D_8007D030 = 1;
                    cpuXYPrintf(100, 24, "MEMORY REGION %d", selectedRegion);
                    D_8007D030 = 2;
                    cpuXYPrintf(32, 32, "MOD");
                    cpuXYPrintf(72, 32, "OFFSET");
                    cpuXYPrintf(152, 32, "SIZE");
                    cpuXYPrintf(224, 32, "ADDRESS");
                    D_8007D030 = 0;
                    slots = mmGetSlotPtr(selectedRegion);
                    slot = slots;
                    do {
                        if (slot->flags != 0) {
                            if (index >= memoryIndex &&
                                index < memoryIndex + 18) {
                                printedValue = slot->colourTag & 0xFFFFFF;
                                tag = (slot->colourTag >> 24) & 0xFF;
                                if (tag == 0xFF) {
                                    cpuXYPrintf(32, row, "Texture %d",
                                                printedValue);
                                } else if (tag == 0xFE) {
                                    cpuXYPrintf(32, row, "Module %d",
                                                printedValue);
                                } else {
                                    cpuXYPrintf(32, row, "%d", tag);
                                    cpuXYPrintf(72, row, "%08x",
                                                printedValue);
                                }
                                cpuXYPrintf(152, row, "%d", slot->size);
                                cpuXYPrintf(224, row, "%08x", slot->data);
                                row += 8;
                            }
                            index++;
                        }
                        slot = &slots[slot->nextIndex];
                    } while (slot->nextIndex != -1);
                    oldPage = currentPage;
                    redraw = 0;
                    break;
                default:
                    currentPage = 0;
                    oldPage = 0;
                    break;
            }
            if (currentPage == 1) {
                if (runlinkGetAddressInfo(address, &printedValue,
                                          &moduleOffset, 0) != 0) {
                    D_8007D030 = 1;
                    cpuXYPrintf(20, 24, "Module %d at %08x", printedValue,
                                moduleOffset);
                    D_8007D030 = 0;
                }
                candidate = address;
                index = 0;
                do {
                    if (index == nibble) {
                        D_8007D030 = 1;
                    }
                    cpuXYPrintf(76 - (index * 8), 32, "%1x", candidate & 0xF);
                    index++;
                    D_8007D030 = 0;
                    candidate = (s32)candidate >> 4;
                } while (index != 8);
            }
            if (D_8007CFE8 == 0) {
                lines = D_8007CFE4;
            } else {
                lines = 500;
            }
            pageLabel = (s32 *)(currentPage + 1);
            printedValue = lines;
            limit = pageCount + 1;
            cpuXYPrintf(50, 200, "%d lines logged", lines);
            cpuXYPrintf(220, 200, "Page %d/%d", pageLabel, limit);
        }
        osWritebackDCacheAll();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045D34.s")
#endif
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuReportWatchpoint. */
void diCpuReportWatchpoint(u32 address) {
    s32 moduleAddress;
    s32 moduleId;
    s32 i;

    for (i = 0; i < 100; i++) {
        func_80046E00();
    }
    cpuXYPrintf(30, 80, D_80083B2C, address);
    if (runlinkGetAddressInfo(address, &moduleId, &moduleAddress, 0)) {
        cpuXYPrintf(30, 100, D_80083B48, moduleId, moduleAddress);
    }
    while (1) {
    }
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceGetFault. */
s32 func_80046504(void) {
    return 0;
}

/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceTick. */
void func_8004650C(s32 ticks) {
    D_8007CFDC += ticks;
    if (D_8007CFDC > 60) {
        D_8007CFDC = 0;
        D_8007CFD8++;
    }
}

#ifdef NON_MATCHING
/* PROVENANCE: adapted from the SDK-style crash-display control flow in JFG
 * src/diCpu.c; Mickey's own m2c control flow, globals, and ABI are authoritative. */
/* Workbench verdict: structure-mismatch; 328 differing words, first mismatch +0x3c.
 * Target 344 instructions/frame -80; candidate 346 instructions/frame -80.
 * Remaining gap is branch-local pointer setup; not shape-exact or permuter-ready. */
void render_epc_lock_up_display(MickeyEpcInfo *arg0) {
    u32 sp4c;
    u32 sp48;
    u32 sp44;
    char *region;
    u32 value;
    u32 *regs;

    func_80046E00();
    cpuXYPrintf(0x20, 0x18, D_80083B5C, arg0->unk14, D_8007CFD0);
    regs = (u32 *)((u8 *)arg0 + 0x20);
    value = regs[0xFC / 4];
    if (value == 0) {
        cpuXYPrintf(0x20, 0x22, D_80083B78);
    } else if (runlinkGetAddressInfo(value, (s32 *)&sp4c, (s32 *)&sp48,
                                     (u32 **)&sp44) != 0) {
        cpuXYPrintf(0x20, 0x22, D_80083B84, sp4c, sp48);
    } else {
        cpuXYPrintf(0x20, 0x22, D_80083B90, regs[0xFC / 4]);
    }
    if (regs[0xE4 / 4] == 0) {
        cpuXYPrintf(0x20, 0x28, D_80083BA0);
    } else if (runlinkGetAddressInfo(regs[0xE4 / 4], (s32 *)&sp4c, (s32 *)&sp48,
                                     (u32 **)&sp44) != 0) {
        cpuXYPrintf(0x20, 0x28, D_80083BAC, sp4c, sp48);
    } else {
        cpuXYPrintf(0x20, 0x28, D_80083BB8, regs[0xE4 / 4]);
    }
    if (regs[0x100 / 4] == -1U) {
        cpuXYPrintf(0x20, 0x2E, D_80083BC8, regs[0x1C / 4], regs[0x24 / 4]);
    } else {
        if ((((regs[0x100 / 4]) >> 2) & 0x1F) < 0x10) {
            cpuXYPrintf(0x20, 0x2E, D_80083BE4,
                        D_8007CFEC[(regs[0x100 / 4] >> 2) & 0x1F]);
        } else {
            cpuXYPrintf(0x20, 0x2E, D_80083BF4, regs[0x100 / 4]);
        }
    }
    if ((D_8007A21C != 4) && (D_8007A210 != 0)) {
        if ((D_8007A21C == 1) || (D_8007A21C == 3) || (D_8007A21C == 2)) {
            if (D_8007A21C == 1) {
                region = D_80083C04;
            } else if (D_8007A21C == 3) {
                region = D_80083C0C;
            } else {
                region = D_80083C14;
            }
            if (D_8007A218 != 0) {
                cpuXYPrintf(0x20, 0x34, D_80083C1C, region,
                            D_8007A220[D_8007A210], D_8007A218);
            } else if (D_8007A214 != NULL) {
                cpuXYPrintf(0x20, 0x34, D_80083C28, region,
                            D_8007A220[D_8007A210], D_8007A214->unk44);
            } else {
                cpuXYPrintf(0x20, 0x34, D_80083C38, region,
                            D_8007A220[D_8007A210]);
            }
        }
    } else {
        cpuXYPrintf(0x20, 0x34, D_80083C48, regs[0x104 / 4]);
    }
    cpuXYPrintf(0x20, 0x3A, D_80083C58, D_800D21B0);
    cpuXYPrintf(0x20, 0x44, D_80083C68, regs[0x4 / 4], regs[0xC / 4]);
    cpuXYPrintf(0x20, 0x4A, D_80083C7C, regs[0x14 / 4], regs[0x1C / 4]);
    cpuXYPrintf(0x20, 0x50, D_80083C90, regs[0x24 / 4], regs[0x2C / 4]);
    cpuXYPrintf(0x20, 0x56, D_80083CA4, regs[0x34 / 4], regs[0x3C / 4]);
    cpuXYPrintf(0x20, 0x5C, D_80083CB8, regs[0x44 / 4], regs[0x4C / 4]);
    cpuXYPrintf(0x20, 0x62, D_80083CCC, regs[0x54 / 4], regs[0x5C / 4]);
    cpuXYPrintf(0x20, 0x68, D_80083CE0, regs[0x64 / 4], regs[0x6C / 4]);
    cpuXYPrintf(0x20, 0x6E, D_80083CF4, regs[0x74 / 4], regs[0x7C / 4]);
    cpuXYPrintf(0x20, 0x74, D_80083D08, regs[0x84 / 4], regs[0x8C / 4]);
    cpuXYPrintf(0x20, 0x7A, D_80083D1C, regs[0x94 / 4], regs[0x9C / 4]);
    cpuXYPrintf(0x20, 0x80, D_80083D30, regs[0xA4 / 4], regs[0xAC / 4]);
    cpuXYPrintf(0x20, 0x86, D_80083D44, regs[0xB4 / 4], regs[0xBC / 4]);
    cpuXYPrintf(0x20, 0x8C, D_80083D58, regs[0xC4 / 4], regs[0xCC / 4]);
    cpuXYPrintf(0x20, 0x92, D_80083D6C, regs[0xD4 / 4], regs[0xDC / 4]);
    cpuXYPrintf(0x20, 0x98, D_80083D80, regs[0xF8 / 4]);
    value = levelGetLevel();
    if ((value != 0) && (value & 0x80000000)) {
        cpuXYPrintf(0x20, 0xA4, D_80083D8C, value);
    }
    value = func_80005820(0);
    if ((value != 0) && (value & 0x80000000)) {
        cpuXYPrintf(0x20, 0xAA, D_80083D9C, value + 0xC,
                    value + 0x10, value + 0x14);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/render_epc_lock_up_display.s")
#endif
/* Mickey-derived body; JFG's corresponding func_800680B0_68CB0 is
 * assembly-only and confirms the packed-glyph loop structure. */
void func_80046AA8(s32 x, s32 y, u16 *glyph) {
    s16 *destination;
    s16 *pixel;
    u16 *palette;
    u16 bits;
    s32 lines;
    s32 rows;
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    /* The do/while(0) closes the setup as its own basic block before the row
     * loop; without that boundary IDO schedules the blit loop's registers
     * differently and the function does not match. */
    do {
        destination = D_800D2FA8 + ((y * width) + x);
        if (D_8007D030 == 2) {
            palette = D_8007D300;
        } else if (D_8007D030 != 0) {
            palette = D_8007D2F8;
        } else {
            palette = D_8007D2F0;
        }
        rows = 5;
    } while (0);
    while (rows--) {
        lines = 1;
        if (D_8007D02C != 0) {
            lines = 2;
        }
        while (lines--) {
            bits = *glyph;
            pixel = destination;
            while (bits) {
                *pixel++ = palette[bits & 3];
                bits >>= 2;
            }
            destination += width;
        }
        glyph++;
    }
}
/* Workbench: structure-mismatch, 89 differing words, first mismatch +0x2C. */
/* Candidate shape: 107 instructions/frame -0x40 vs target 106/-0x40; not permuter-ready. */
/* Remaining structural gap: one extra loop move shifts the glyph-call relocations. */
/* PROVENANCE: adapted from Jet Force Gemini's public
 * asm/nonmatchings/diCpu/func_800681D0_68DD0.s; Mickey's glyph table,
 * helper symbol, and target bytes determine the final bindings. */
#ifdef NON_MATCHING
void func_80046BCC(s32 x, s32 y, char *text) {
    s32 temp_s6;
    s32 var_s0;
    s32 var_s1;
    s32 var_s2;
    s32 var_s3;
    s32 var_s5;
    u8 *var_s4;
    s32 var_v0;

    var_v0 = *(u8 *)text;
    var_s1 = x;
    var_s4 = (u8 *)text;
    var_s5 = y;
    var_s3 = 0;
    var_s2 = 0;
    if (var_v0 != 0) {
        do {
            temp_s6 = var_s2 & 0xFF;
            var_s2 = var_v0 & 0xFF;
            var_s4 += 1;
            if (var_s3 != 0) {
                var_s0 = var_s2;
                if ((var_s2 >= 0x41) && (var_s0 < 0x47)) {
                    var_s0 = (var_s0 + 0x20) & 0xFF;
                    var_s2 = var_s0;
                }
            } else {
                var_s0 = var_s2;
                if ((var_s2 >= 0x61) && (var_s0 < 0x7B)) {
                    var_s0 = (var_s0 - 0x20) & 0xFF;
                    var_s2 = var_s0;
                }
            }
            if (var_s0 == 0xA) {
                var_s5 += 6;
                var_s1 = 0x20;
            } else if (var_s0 == 9) {
                var_s1 = (var_s1 - (var_s1 & 0xF)) + 0x10;
            } else if (var_s0 == 0x20) {
                var_s1 += 4;
            } else if ((var_s0 >= 0x21) && (var_s0 < 0x67)) {
                func_80046AA8(var_s1, var_s5, &D_8007D034[(var_s0 * 5) - 0xA5]);
                var_s1 += 8;
            }
            if ((var_s3 != 0) && ((var_s0 < 0x30) || (var_s0 >= 0x3A)) &&
                ((var_s0 < 0x61) || (var_s0 >= 0x67))) {
                var_s3 = 0;
            }
            if ((temp_s6 == 0x30) && ((var_s0 == 0x78) || (var_s0 == 0x58))) {
                var_s3 = 1;
            }
            var_v0 = *var_s4;
        } while (var_v0 != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046BCC.s")
#endif
/* PROVENANCE: body adapted from JFG src/diCpu.c::cpuXYPrintf. */
void cpuXYPrintf(s32 x, s32 y, const char *format, ...) {
    va_list args;
    char text[255];

    va_start(args, format);
    vsprintf(text, format, args);
    va_end(args);

    if (D_8007D02C != 0) {
        if (D_8007D02C == 1) {
            y -= 8;
        } else {
            y -= 104;
        }
        if (y >= 0 && y < 116) {
            y *= 2;
            goto draw;
        }
    } else {
draw:
        func_80046BCC(x, y, text);
    }
}
/* PROVENANCE: body adapted from JFG src/diCpu.c::func_8006837C_68F7C. */
void func_80046E00(void) {
    s32 pad;
    s32 height;
    s32 width;
    s32 screenSize;
    s16 *screen;

    viGetCurrentSize(&height, &width);
    screenSize = height * width;
    screen = D_800D2FA8;
    while (screenSize--) {
        *screen++ = 0;
    }
}
