#include "PR/ultratypes.h"

typedef struct Overlay1Entry {
    u8 bytes[0x1C];
} Overlay1Entry;

extern Overlay1Entry *gOverlay1Entries;

/* The pinned DKR v77/v80 and JFG object scans contain no exact donor.
 *
 * This function's compiled object required normalize_elf_instructions.py
 * to reach byte identity (three register-field edits), which
 * docs/acceleration-survey.md sec.13.2's ruling disqualifies as "matched":
 * no gold-standard N64 decomp edits an instruction word after compilation.
 * It stays a queued NON_MATCHING/GLOBAL_ASM function, per DKR's convention,
 * until source restructuring makes IDO emit the retail bytes on its own. */
#ifdef NON_MATCHING
Overlay1Entry *overlay1GetEntry(s32 index) {
    Overlay1Entry *result;
    Overlay1Entry *entries;

    entries = gOverlay1Entries;
    result = NULL; if (entries != NULL) result = &entries[index];
    return result;
}
#else
/* splat's raw ROM scan never learns the friendly per-function name
 * TEXT_SUBSEGMENTS gives this range (every overlay shares one synthetic
 * VMA, so spimdisasm names functions by module+offset+ROM address instead);
 * the GLOBAL_ASM path has to name the .s file the way splat will actually
 * write it, func_overlay_MMM_FOOOOOOO_ROMADDR, and POSTPROCESS below
 * restores the linkable name with objcopy --redefine-sym (metadata-only,
 * tolerated scaffolding per docs/acceleration-survey.md sec.13.2). */
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1GetEntry/func_overlay_001_F0000050_184C430.s")
#endif
