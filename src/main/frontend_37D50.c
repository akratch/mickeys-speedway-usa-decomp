#include "PR/ultratypes.h"
#include "game/memory.h"

extern s32 D_8007BEA8;
extern s32 D_8007BE90;
extern s32 D_8007BE94;
extern s32 D_8007BE98;
extern s32 D_8007BEB8;
extern s32 D_8007BE80;
extern s32 D_8007BEAC;
extern s32 D_8007BE9C;
extern s32 D_8007BEA0;
extern s32 D_8007BEA4;
extern s32 D_8007BEE0;
extern s32 D_800D2FA8;
typedef struct FrontendBufferPointers {
    void *unk0;
    void *unk4;
    void *unk8;
} FrontendBufferPointers;
extern FrontendBufferPointers D_8007BE88;
typedef struct FrontendVertex {
    s16 x;
    s16 y;
    s16 unk4;
    s8 r;
    s8 g;
    s8 b;
    s8 a;
} FrontendVertex;
extern s32 D_8007BE84;
extern s32 D_8007BEB0;
extern s32 D_8007BEB4;
extern f32 D_800826A0;
extern void func_800378A4(f32 arg0, s32 arg1);
extern f32 func_8002A8C0(s32 angle);
extern f32 sqrtf(f32 value);
extern void func_80037AEC(f32 arg0, s32 arg1);

typedef struct FrontendGfxWords {
    u32 w0;
    u32 w1;
} FrontendGfxWords;
typedef struct Gfx {
    FrontendGfxWords words;
} Gfx;
typedef struct Mtx Mtx;
typedef struct MainVertex MainVertex;

void func_80037150(void) {
    D_8007BEA8 = 0;
    if (D_8007BE88.unk0 != NULL) {
        mmFree(D_8007BE88.unk0);
        D_8007BE88.unk0 = NULL;
    }
    if (D_8007BE88.unk4 != NULL) {
        mmFree(D_8007BE88.unk4);
        D_8007BE88.unk4 = NULL;
    }
    D_8007BE80 = 0;
}
extern s32 viGetVideoMode(void);
extern void *func_8002B280(s32, s32);
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_800371BC.s")
extern void TrapDanglingJump();
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_80037414.s")
void func_80037658(void) {
    D_8007BEA8 = 0;
}
s32 func_80037664(void) {
    if ((D_8007BEA8 == 0) && (D_8007BEB8 == 0)) {
        return 0;
    }
    if ((D_8007BEA8 != 2) || ((D_8007BE90 & 1) != 0) ||
        (D_8007BEB8 != 0)) {
        return 1;
    }
    return 2;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_800376CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_800378A4.s")
void func_80037A78(void) {
    D_8007BEB4 = 0x8000 - (D_8007BEB0 << 8);
    if (D_8007BEB0 < 0x200) {
        func_800378A4((f32) ((s32) D_8007BEB0 >> 3), 0x100);
        return;
    }
    func_800378A4(64.0f, (s32) (0x400 - D_8007BEB0) >> 1);
}
void func_80037AEC(f32 arg0, s32 arg1)
{
  FrontendVertex *vertex;
  s32 row;
  s32 phase;
  vertex = ((FrontendVertex **) (&D_8007BE88))[D_8007BE84];
  if (vertex != ((void *) 0))
  {
    s32 contrast = 0xFF - (((s32) arg0) * 2);
    phase = D_8007BEB4;
    if (1)
    {
      row = 0;
      do
      {
        s32 column = 0;
        s32 angle = phase;
        do
        {
          f32 value = func_8002A8C0(angle) * arg0;
          s32 integerValue;
          s32 intensity;
          column += 1;
          angle += 0x2000;
          vertex += 1;
          integerValue = (s32) value;
          vertex[-1].unk4 = (s16) (integerValue + 5);
          intensity = ((s32) (((integerValue * 2) + contrast) * arg1)) >> 8;
          vertex[-1].r = (s8) intensity;
          vertex[-1].g = (s8) intensity;
          vertex[-1].b = (s8) (intensity & 0xFFFFFFFFFFFFFFFF);
        }
        while (column != 0x11);
        row += 1;
        phase += 0x800;
      }
      while (row != 0x11);
    }
  }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_80037BF4.s")
extern u8 D_7BE40[];
extern s32 D_800D2FAC;
extern void camStandardPersp(Gfx **, Mtx **);
extern void func_80034920(Gfx **);
extern u8 D_8007BEC0[];
extern void viGetCurrentSize(s32 *, s32 *);
extern void func_80037A78(void);
extern void func_80037BF4(void);

#define FRONTEND_EMIT(pkt, opcode, data) \
    do { \
        Gfx *_cmd = *(pkt); \
        _cmd->words.w0 = (u32) (opcode); \
        _cmd->words.w1 = (u32) (data); \
        *(pkt) = _cmd + 1; \
    } while (0)

#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_80037C74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/frontend_37D50/func_80038190.s")
#undef FRONTEND_EMIT
