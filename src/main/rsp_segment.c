#include "PR/R4300.h"
#include "n_audio/mbi.h"

/*
 * PROVENANCE: adapted from Diddy Kong Racing's published
 * src/set_rsp_segment.c. Mickey's whole function and translation-unit text
 * are independently Tier-A exact against DKR's built object.
 */
void rsp_segment(Gfx **dList, s32 segment, s32 base) {
    gSPSegment((*dList)++, segment, base + K0BASE)
}
