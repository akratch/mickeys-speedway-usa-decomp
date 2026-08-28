/*
 * PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/textures.c:texEnableModes. Mickey's global, linked bytes, and
 * relocations remain authoritative.
 */

#include "PR/ultratypes.h"

extern s32 D_8007BD90;

void texEnableModes(s32 flags) {
    D_8007BD90 &= ~flags;
}
