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

extern s32 D_8007CFD8;
extern s32 D_8007CFDC;
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
extern s32 runlinkGetAddressInfo(u32 address, u32 *moduleId,
                                 s32 *moduleOffset, s32 *size);
extern void render_epc_lock_up_display(s32 arg0, s32 buttons);
extern void cpuXYPrintf(s32 x, s32 y, const char *format, ...);
extern void func_80046E00(void);
extern s32 D_80000310;
extern s32 D_8007A1E0;
extern s32 D_8007A200;
extern s32 D_8007CFE4;
extern s32 D_8007CFE8;
extern s32 D_8007D02C;
extern s32 D_8007D030;
extern s32 D_800D5DF0[];
extern s32 D_800D5E98[];
extern s32 D_800D5F40[];
void stop_all_threads_except_main(void);
void diCpuThread(void *unused);

/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuTraceInit. */
void diCpuTraceInit(void) {
    osCreateThread(&diCpuOSThread, 0, diCpuThread, 0, diCpuThreadStack, 0xFF);
    osStartThread(&diCpuOSThread);
}
#ifdef NON_MATCHING
/* PROVENANCE: body adapted from JFG src/diCpu.c::diCpuThread; Mickey's own
 * draft supplies its extra VI delay loop and exact event handling. */
void diCpuThread(void *unused) {
    OSMesg message;
    s32 events;
    register s32 i;
    register f32 divisor;
    register f32 sum;

    events = 0;
    osCreateMesgQueue(&D_800D5CD0, D_800D5CE8, 8);
    osSetEventMesg(12, &D_800D5CD0, (OSMesg)8);
    osSetEventMesg(10, &D_800D5CD0, (OSMesg)2);
    func_8004D5E0(150, &D_800D5D28, D_800D5D08, 8);
    divisor = D_80083DBC;
    while (1) {
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuThread.s")
#endif
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045BBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80045CAC.s")
#ifdef NON_MATCHING
/* Mickey-derived draft; JFG's closest peer, src/diCpu.c::func_80067880,
 * remains assembly-only and supplies no body. */
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
                    render_epc_lock_up_display(arg0, buttons);
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/diCpuReportWatchpoint.s")
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

#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/render_epc_lock_up_display.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046AA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046BCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/cpuXYPrintf.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diCpu/func_80046E00.s")
