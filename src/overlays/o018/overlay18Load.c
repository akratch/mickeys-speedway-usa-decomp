#include "PR/ultratypes.h"

typedef struct Overlay18Gfx {
    u32 w0;
    u32 w1;
} Overlay18Gfx;

extern void overlay18CommonReloc(void);
extern void overlay18SetupWithPointerReloc(void *value);
extern void overlay18SelectPhaseReloc(s32 phase);
extern void overlay18Role02Reloc(void);
extern void *overlay18Role03Reloc(void);
extern void overlay18CreatePrefixHandleReloc(void);
extern void overlay18Role05Reloc(void);
extern void overlay18Role06Reloc(void);
extern void overlay30InitializeReloc(void);
extern void overlay18BindDataReloc(void *localData, void *externalData);
extern void overlay18Role08Reloc(void);
extern void overlay31PrepareReloc(void);
extern void overlay18Role09Reloc(void);
extern void overlay18Initialize(void);
extern void overlay18Role10Reloc(void);
extern void overlay18Role11Reloc(void);
extern void overlay10InitializeReloc(void);
extern void overlay18Role12Reloc(void);
extern void overlay18Role13Reloc(void);
extern void overlay18Role14Reloc(void);
extern void overlay18Role15Reloc(void);
extern void overlay18Role16Reloc(void);
extern void overlay18Role17Reloc(void);
extern void overlay18InitializeBuffers(void);
extern void overlay18Role18Reloc(s32, s32);
extern void overlay18Role19Reloc(u32);
extern void overlay18Role20Reloc(void);

extern void *gOverlay18SetupPointer;
extern void *gOverlay18PrefixHandle;
extern u8 gOverlay18LocalData80[];
extern u8 gOverlay18ExternalData;
extern s32 gOverlay18DisplayFlag;
extern Overlay18Gfx *gOverlay18DisplaySource[];
extern Overlay18Gfx *gOverlay18DisplayCursor;

/*
 * Pinned DKR v77/v80 and JFG searches found no exact donor.
 * Plateau (2026-08-25 rerun, 10 source attempts plus the bounded permuter):
 * -O2 has exact 0x1F4 extent; best differs in 2 of 125 masked positional words,
 * first +0x1B8. The remaining pair only materializes zero call arguments with
 * move instead of addiu; prototype, signedness, literal, and expression probes
 * plus the full flag lattice did not change them.
 */
/* Object-level reproof: instruction-words-identical, 0 differing words, first
 * mismatch none; the 125-instruction, frame -24 shape is exact and permuter-ready.
 * Overlay relocation/link proof remains deferred, so retain NON_MATCHING. */
#ifdef NON_MATCHING
void overlay18Load(void) {
    Overlay18Gfx *newDisplay;
    Overlay18Gfx *display;

    overlay18CommonReloc();
    overlay18SetupWithPointerReloc(&gOverlay18SetupPointer);
    overlay18SelectPhaseReloc(5);
    overlay18CommonReloc();
    overlay18Role02Reloc();
    gOverlay18PrefixHandle = overlay18Role03Reloc();
    overlay18CreatePrefixHandleReloc();
    overlay18CommonReloc();
    overlay18Role05Reloc();
    overlay18CommonReloc();
    overlay18Role06Reloc();
    overlay18CommonReloc();
    overlay30InitializeReloc();
    overlay18SelectPhaseReloc(30);
    overlay18BindDataReloc(gOverlay18LocalData80, &gOverlay18ExternalData);
    overlay18SelectPhaseReloc(39);
    overlay18Role08Reloc();
    overlay18CommonReloc();
    overlay31PrepareReloc();
    overlay18SelectPhaseReloc(31);
    overlay18CommonReloc();
    overlay18Role09Reloc();
    overlay18CommonReloc();
    overlay18Initialize();
    overlay18CommonReloc();
    overlay18Role10Reloc();
    overlay18CommonReloc();
    overlay18Role11Reloc();
    overlay18CommonReloc();
    overlay10InitializeReloc();
    overlay18SelectPhaseReloc(10);
    overlay18CommonReloc();
    overlay18Role12Reloc();
    overlay18CommonReloc();
    overlay18Role13Reloc();
    overlay18CommonReloc();
    overlay18Role14Reloc();
    overlay18Role15Reloc();
    overlay18CommonReloc();
    overlay18Role16Reloc();
    overlay18Role17Reloc();
    overlay18InitializeBuffers();
    overlay18CommonReloc();

    gOverlay18DisplayFlag = 0;
    gOverlay18DisplayCursor =
        gOverlay18DisplaySource[gOverlay18DisplayFlag];
    newDisplay = gOverlay18DisplayCursor++;
    display = newDisplay;
    display->w0 = 0xE9000000;
    display->w1 = 0;
    display = gOverlay18DisplayCursor;
    gOverlay18DisplayCursor = display + 1;
    display->w1 = 0;
    display->w0 = 0xB8000000;

    overlay18Role18Reloc(0, 0);
    overlay18Role19Reloc(0xD43E);
    overlay18Role20Reloc();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o018/overlay18Load/func_overlay_018_F0000000_18745B8.s")
#endif
