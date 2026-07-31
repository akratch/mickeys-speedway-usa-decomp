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
 * Field evidence -- the stride is proven: ResolveRelocAddress indexes the
 * table with `sll t9, v1, 5`, so sizeof is 0x20. Two fields are proven by
 * Mickey's own code: `vramBase` at 0x00 (read by ResolveRelocAddress and by
 * runlinkIsModuleLoaded) and `resumeFunction` at 0x1C (read twice by
 * runlinkCallResumeFunction, at ROM 0x32EB4 and 0x32F18, and compared against
 * -1). Both of those functions match Mickey's ROM byte for byte, so the two
 * offsets are measured rather than assumed.
 *
 * The other six fields are JFG's names at JFG's offsets and NOTHING IN THIS
 * PROJECT HAS TOUCHED THEM YET. They are present rather than commented out
 * because the struct has to be 0x20 bytes for the indexing to compile
 * correctly, but treat each one as unverified until a matched function reads
 * it. (An earlier version of this comment claimed they were "left commented",
 * which was simply wrong about its own code.)
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
            /* 0x07 */ u8 flags;           /* high nibble mode, low nibble operation */
        } b;
        /* 0x04 */ struct {
            /* 0x04 */ u8 pad[3];
            /* 0x07 */ u8 mode : 4; /* RELOC_TYPE_* */
            /* 0x07 */ u8 op : 4;   /* RELOC_OP_*, the same nibble the word test reads */
        } n;
    } u;
} RelocationEntry; /* sizeof == 0x8 */

/*
 * One MIPS instruction word, as the linker patches it.
 *
 * Field evidence -- PatchInstruction writes the whole word with `sw` at
 * offset 0 (ROM 0x325C0, 0x325E8) for the 32-bit and jump-target cases, and
 * writes a halfword with `sh` at offset 2 (ROM 0x325B0, 0x32600, 0x32604) for
 * the HI16/LO16 cases. Offset 2 is the low half of a big-endian word, i.e. an
 * I-type instruction's immediate field, which is exactly what HI16/LO16 patch.
 */
typedef union MipsInstruction {
    /* 0x00 */ u32 word;
    /* 0x00 */ struct {
        /* 0x00 */ u16 upper;     /* opcode and register fields */
        /* 0x02 */ u16 immediate; /* the I-type immediate HI16/LO16 patch */
    } i;
} MipsInstruction; /* sizeof == 0x4 */

/*
 * Relocation types -- the HIGH nibble of RelocationEntry::u.b.flags.
 *
 * One value space, used by two functions for two purposes: ResolveRelocAddress
 * tests it to decide how an unresolved symbol should be substituted, and
 * PatchInstruction switches on it to decide which bits of the instruction to
 * rewrite. Both read the same nibble of the same record -- ProcessRelocationEntry
 * loads it once into `mode` and passes that value straight to PatchInstruction
 * -- so what used to be two macro families here (PATCH_OP_* and RELOC_MODE_*)
 * was one family described twice, with two names for each of 2 and 4.
 *
 * The numbering is not arbitrary and not Rare's invention: 2, 4, 5 and 6 are
 * the standard ELF MIPS relocation type numbers for R_MIPS_32, R_MIPS_26,
 * R_MIPS_HI16 and R_MIPS_LO16. That is checkable against the published MIPS
 * ELF ABI, and it explains the gap at 3 and the meanings exactly: the runtime
 * linker consumes the same r_type values a static linker would.
 *
 * Do NOT confuse these with the RELOC_OP_* values further down. Those are the
 * LOW nibble of the same byte and are a different, Rare-specific space.
 */
#define RELOC_TYPE_32   2 /* R_MIPS_32:   store the resolved address as a whole word */
#define RELOC_TYPE_26   4 /* R_MIPS_26:   rewrite a j/jal target, preserving the opcode */
#define RELOC_TYPE_HI16 5 /* R_MIPS_HI16: upper half, carrying LO16's sign extension */
#define RELOC_TYPE_LO16 6 /* R_MIPS_LO16: lower half */

/*
 * One entry of the pending-overlay-load list. Eight bytes; only the second
 * word is read by anything decompiled so far.
 *
 * Field evidence -- runlinkCallResumeFunction walks the list at ROM
 * 0x32EDC-0x3230C with `lw t9, 0x4(v1)` and `addiu v1, v1, 0x8`, comparing the
 * loaded word against the overlay index. The list is exactly 16 entries long:
 * the loop counter is initialised to 15 (`addiu a0, zero, 0xF`) and runs down
 * to zero inclusive.
 */
typedef struct PendingOverlayLoad {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 overlayIndex;
} PendingOverlayLoad; /* sizeof == 0x8 */

#define PENDING_OVERLAY_LOADS 16

/*
 * One entry of the link-slot table -- a halfword split into a 10-bit and a
 * 6-bit field.
 *
 * Field evidence -- SetLinkSlot writes the upper field with
 * `sll t1, a1, 6` / `or` / `sh` over a halfword whose low 6 bits are preserved
 * (ROM 0x33C60-0x33C70), and the lower field with a byte write that preserves
 * the top two bits of the low byte (`andi t9, t8, 0xFFC0`, ROM 0x33C80). Two
 * fields, 10 bits over 6, big-endian order.
 *
 * The *names* are the weakest thing in this header and are marked as such:
 * all that is proven is that ReleaseUnusedLinkSlots treats "tag set, useCount
 * zero" as reclaimable, which is what a reference count looks like. Rename
 * once a third caller says otherwise.
 */
typedef struct LinkSlot {
    /* 0x00 */ u16 tag : 10;
    /* 0x00 */ u16 useCount : 6;
} LinkSlot; /* sizeof == 0x2 */

/* Linkage operations -- the low nibble of RelocationEntry::u.info. */
#define RELOC_OP_SYMBOL 0 /* absolute reference to a symbol in some overlay */
#define RELOC_OP_LOCAL  1 /* offset relative to this overlay's own base */
#define RELOC_OP_JUMP   2 /* R_MIPS_26, patch a j/jal target in place */
#define RELOC_OP_DATA   3 /* patch site is in the data section, not the text section */

/* Reserved overlay numbers -- section selectors rather than real overlays. */
#define RELOC_SECTION_DATA1 0xFFD
#define RELOC_SECTION_DATA2 0xFFE
#define RELOC_SECTION_BSS   0xFFF

/*
 * (The high nibble's values live with the RELOC_TYPE_* block above -- they are
 * one space. ResolveRelocAddress reads R_MIPS_32 there as "the word already in
 * place is an addend" and R_MIPS_26 as "route an unresolved symbol to the
 * dangling-jump trap", which is the same relocation kind seen from the
 * resolver's side rather than the patcher's.)
 */

#endif /* _GAME_RUNLINK_H_ */
