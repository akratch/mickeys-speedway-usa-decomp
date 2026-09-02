#include "PR/ultratypes.h"

/*
 * PROVENANCE: this texture-TU sequence was compared with Jet Force Gemini's
 * public src/textures.c. The two state setters are JFG-shaped starting
 * points; Mickey's globals and final byte/relocation results decide adoption.
 */

extern u8 D_8007BD94;
extern s32 D_8007BD80;

typedef struct TextureCacheEntry {
    s32 id;
    void *texture;
} TextureCacheEntry;

typedef struct TextureHeader {
    u16 numberOfInstances;
    u8 pad02[0x1E];
} TextureHeader;

extern s32 D_800D2FE0;
extern TextureCacheEntry *D_800D2FD8;
extern void mmFree(void *ptr);
void func_80034424(u8 enabled) {
    D_8007BD94 = enabled;
}
void func_80034434(s32 value)
{
  s32 *new_var;
  D_8007BD80 = (*(new_var = &value)) & 0xFF;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_35024/func_80034448.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_35024/func_800347A0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_35024/func_8003484C.s")
