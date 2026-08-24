#include "PR/ultratypes.h"
typedef struct Overlay12Effect {
 f32 x0,y0,z0; f32 x1,y1,z1; f32 x2,y2,z2; f32 zero; f32 value;
 u8 pad2C[0x10]; s16 lifetime; u8 pad3E[0xC];
 u8 active; u8 kind1; s16 scaleX; s16 scaleY; u8 type; u8 kind2; u8 pad52[2];
} Overlay12Effect;
extern s32 gOverlay12Ready, gOverlay12EffectCount;
extern Overlay12Effect gOverlay12Effects[64];
extern void overlay12Initialize(void); extern s32 overlay12Lookup(s32,s32);
#ifdef NON_MATCHING
void func_overlay_012_F00001B4_186D434(f32 x,f32 y,f32 z,f32 x2,f32 y2,f32 z2,s32 scale,s32 type,f32 value) {
 s32 i; Overlay12Effect *effect;
 if(gOverlay12Ready==0) overlay12Initialize();
 if(gOverlay12EffectCount<64) {
  effect=gOverlay12Effects;
  for(i=0;i<64;i++,effect++) if(effect->active==0) break;
  if(i<64) {
   effect->x2=x2; effect->y2=y2; effect->z2=z2;
   effect->active=1; effect->lifetime=300;
   effect->x1=x; effect->y1=y; effect->zero=0.0f; effect->value=value;
   effect->z1=z; effect->x0=x; effect->y0=y; effect->z0=z;
   effect->kind1=overlay12Lookup(0,1);
   effect->scaleX=scale<<5; effect->scaleY=scale<<5; effect->type=type;
   effect->kind2=overlay12Lookup(0,2);
   gOverlay12EffectCount++;
  }
 }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o012/overlay12SpawnEffect/func_overlay_012_F00001B4_186D434.s")
#endif
