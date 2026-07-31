#ifndef _GAME_RUNLINK_H_
#define _GAME_RUNLINK_H_

#include "PR/ultratypes.h"

/*
 * The runtime overlay linker's data structures.
 *
 * Every field below is justified from Mickey's own disassembly (ROM
 * 0x323E0-0x33FA0); the *names* follow Jet Force Gemini's public decomp,
 * which documents the same Rare/DKR-lineage linker. The identification is
 * not a guess: the diagnostic strings at ROM 0x83018-0x83087 ("WARNING:
 * Unimplemented linkage operation %d\n", "ERROR:MIPS_HI16 without matching
 * MIPS_LO16\n", "REALLOC: %08x (%d)\n") are byte-identical to the ones
 * embedded in JFG's runLink.c, and the functions below have the same
 * argument lists, the same switch shapes and the same field offsets.
 */

/*
 * One entry of the overlay ROM table: which overlay a symbol lives in, and
 * how far into it.
 *
 * Field evidence -- ResolveRelocAddress loads the entry as one word and then
 * does `srl v1, t0, 20` (0x800318xx) and `and t5, t0, 0xFFFFF` (0x800318C4),
 * i.e. a 12-bit field above a 20-bit field, big-endian bitfield order.
 * Overlay numbers 0xFFD/0xFFE/0xFFF are reserved section selectors, so the
 * field really is 12 bits wide and not, say, 8.
 */
typedef struct RomTableEntry {
    /* 0x00 */ u32 overlayNumber : 12;
    /* 0x00 */ u32 functionOffset : 20;
} RomTableEntry; /* sizeof == 0x4 */

/*
 * One overlay's header.
 *
 * Field evidence -- only `vramBase` is proven by this file so far:
 * ResolveRelocAddress indexes the table with `sll t9, v1, 5` (stride 0x20)
 * and reads offset 0x00. The remaining names/offsets are JFG's and are left
 * commented until Mickey's own code is shown to touch them.
 */
typedef struct OverlayHeader {
    /* 0x00 */ s32 vramBase; /* 0 while the overlay is not resident */
    /* 0x04 */ s32 romAddress;
    /* 0x08 */ s32 textSize;
    /* 0x0C */ s32 dataSize;
    /* 0x10 */ s32 rodataSize;
    /* 0x14 */ u16 relocTableSize;
    /* 0x16 */ u16 relocTableSize2;
    /* 0x18 */ s32 initFunction;
    /* 0x1C */ s32 resumeFunction;
} OverlayHeader; /* sizeof == 0x20 */

/*
 * One relocation record.
 *
 * Field evidence -- ResolveRelocAddress reads the record with `lw t2, 4(a2)`
 * followed by `andi t8, t2, 0xF` (whole word, low nibble = the linkage
 * operation), reads `lw t4, 0(a2)` as the symbol index, and separately reads
 * `lbu t5, 7(a2)` followed by `srl 4` for the mode nibble. A whole-word read
 * *and* a byte read of the same word is what forces the union below: a plain
 * `u8 flags` field would have made the operation test a `lbu`, and a plain
 * bitfield would have made the mode test a shift of the word.
 */
typedef struct RelocationEntry {
    /* 0x00 */ u32 symbolIndex;
    /* 0x04 */ union {
        /* 0x04 */ u32 info;
        /* 0x04 */ struct {
            /* 0x04 */ u8 targetOffset[3]; /* big-endian 24-bit byte offset */
            /* 0x07 */ u8 mode : 4;        /* RELOC_MODE_* */
            /* 0x07 */ u8 op : 4;          /* RELOC_OP_*, same nibble the word test reads */
        } b;
    } u;
} RelocationEntry; /* sizeof == 0x8 */

/* Linkage operations -- the low nibble of RelocationEntry::u.info. */
#define RELOC_OP_SYMBOL 0 /* absolute reference to a symbol in some overlay */
#define RELOC_OP_LOCAL  1 /* offset relative to this overlay's own base */
#define RELOC_OP_JUMP   2 /* R_MIPS_26, patch a j/jal target in place */

/* Reserved overlay numbers -- section selectors rather than real overlays. */
#define RELOC_SECTION_DATA1 0xFFD
#define RELOC_SECTION_DATA2 0xFFE
#define RELOC_SECTION_BSS   0xFFF

/* High nibble of RelocationEntry::u.b.flags. */
#define RELOC_MODE_ADDEND 2 /* the word already in place is an addend */
#define RELOC_MODE_STUB   4 /* unresolved -> route to the dangling-jump trap */

#endif /* _GAME_RUNLINK_H_ */
