/*
 * The runtime overlay linker -- ROM 0x323E0-0x33FA0 (VRAM 0x800317E0).
 *
 * Identified from its diagnostic strings (ROM 0x83018-0x83087), which are
 * byte-identical to the ones in Jet Force Gemini's public decomp of the same
 * Rare/DKR-lineage linker. Names follow JFG's where the routine is
 * demonstrably the same function; see include/game/runlink.h for the field
 * evidence behind the structs.
 *
 * Flags: -O2 -mips2 -32. The -O2 is the project default; the -mips2 is a
 * measured deviation and the first evidence about how GAME code (as opposed to
 * libultra) was built. ResolveRelocAddress's `lw t7,0(a3)` at ROM 0x32534 is
 * followed immediately by `addu v1,v1,t7`, using the loaded register in the
 * next instruction. At -mips1 IDO's assembler pads every such pair with a
 * load-delay nop; five of them appeared across this file before the Makefile
 * gained its src/main/ MIPSISET override.
 */

#include "PR/ultratypes.h"
#include "PR/os.h"
#include "game/runlink.h"

extern RomTableEntry *D_800D2D98;  /* the overlay ROM table */
extern OverlayHeader *D_800D2D90;  /* the overlay table */
extern u32 D_800D2DC4;             /* placeholder returned for unresolved symbols */
extern void func_800333A0(void);   /* the dangling-jump trap */

/* Linker-ish section anchors, referenced only to form differences. */
extern u8 D_80078D60[]; /* start of .data  */
extern u8 D_80085A40[]; /* start of .bss   */
extern void func_80000450(void); /* start of .text */

#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_800317E0.s")

/*
 * Turn one relocation record into the address it should resolve to.
 *
 * JFG calls this ResolveRelocAddress; same four arguments, same three-way
 * switch on the linkage operation, same 0xFFD/0xFFE/0xFFF section selectors.
 */
void *ResolveRelocAddress(s32 ortIndex, s32 otIndex, RelocationEntry *relocEntry, u32 *patchLocation) {
    s32 address;
    s32 addressBase;
    s32 addressOffset;
    u32 overlayNumber;
    RomTableEntry *romTableEntry;

    romTableEntry = &D_800D2D98[ortIndex];
    overlayNumber = romTableEntry->overlayNumber;
    addressOffset = 0;

    switch (relocEntry->u.info & 0xF) {
        case RELOC_OP_SYMBOL:
            switch (overlayNumber) {
                case RELOC_SECTION_DATA1:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80078D60 - (s32) func_80000450;
                    break;
                case RELOC_SECTION_DATA2:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80078D60 - (s32) func_80000450;
                    break;
                case RELOC_SECTION_BSS:
                    overlayNumber = 0;
                    addressOffset = (s32) D_80085A40 - (s32) func_80000450;
                    break;
            }
            addressBase = D_800D2D90[overlayNumber].vramBase;
            if (addressBase == 0) {
                if (relocEntry->u.b.mode == RELOC_MODE_STUB ||
                    relocEntry->u.b.mode == RELOC_MODE_ADDEND) {
                    return (void *) func_800333A0;
                }
                return &D_800D2DC4;
            }
            return (void *) (addressBase + romTableEntry->functionOffset + addressOffset);

        case RELOC_OP_LOCAL:
            address = D_800D2D90[otIndex].vramBase + relocEntry->symbolIndex;
            if (relocEntry->u.b.mode == RELOC_MODE_ADDEND) {
                address += *patchLocation;
            }
            return (void *) address;

        case RELOC_OP_JUMP:
            return (void *) (((*patchLocation & 0x3FFFFFF) << 2) + D_800D2D90[otIndex].vramBase);

        default:
            return NULL;
    }
}
/*
 * Write a resolved address into the instruction that referenced it, then make
 * the change visible to the CPU's instruction fetch.
 *
 * JFG calls this PatchInstruction; same three arguments, same four patch
 * operations with the same numbering, same pair of cache calls at the end.
 */
void PatchInstruction(MipsInstruction *instr, u32 address, u8 patchOp) {
    u32 word;
    u32 patched;

    switch (patchOp) {
        case PATCH_OP_WORD:
            instr->word = address;
            break;

        case PATCH_OP_JUMP:
            word = instr->word;
            patched = ((address >> 2) & 0x3FFFFFF) ^ word;
            instr->word = ((patched << 6) >> 6) ^ word;
            break;

        case PATCH_OP_HI16:
            patched = address >> 16;
            if (address & 0x8000) {
                patched = (address >> 16) + 1;
            }
            instr->i.immediate = patched;
            break;

        case PATCH_OP_LO16:
            instr->i.immediate = address;
            break;
    }
    osWritebackDCache(instr, sizeof(MipsInstruction));
    osInvalICache(instr, sizeof(MipsInstruction));
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80031A30.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80031C78.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_800320F0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80032284.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_8003229C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80032338.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80032618.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80032820.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_800328CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80032B14.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80032BF8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80032FE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80033040.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_80033090.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_8003314C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/runlink/func_800331E4.s")
