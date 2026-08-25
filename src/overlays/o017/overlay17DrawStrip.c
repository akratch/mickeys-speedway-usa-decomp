#include "PR/ultratypes.h"
typedef struct G { u32 w0, w1; } G;
typedef struct V { s16 x,y,z; u8 r,g,b,a; } V;
typedef struct Pair { V first, second; } Pair;
typedef struct Strip { s16 count; u8 buffer,pad03; void *material; u8 pad08[0x24]; Pair *buffers[2]; void *triangles; } Strip;
extern void overlay17PrepareStripReloc(G **, void *, s32, s32);
#define S(v,s,w) (((u32)(v)&((1U<<(w))-1U))<<(s))
#define PRIM(p) { G *m=(G *)(p); m->w0=0xFA000000; m->w1=0xFFFFFFFF; }
#define VTX(p,a,n) { G *m=(G *)(p); m->w0=S(4,24,8)|S(((n)<<3)|((u32)(a)&6),16,8)|S(((n)<<3)+((n)<<1)+8,0,16); m->w1=(u32)(a); }
#define STRIP(p,a,n,t) { G *m=(G *)(p); m->w0=S(5,24,8)|S((((n)-1)<<4)|(t),16,8)|S((n)<<4,0,16); m->w1=(u32)(a); }
#define SYNC(p) { volatile G *m=(G *)(p); m->w1=0; m->w0=0xE7000000; }

/*
 * Plateau (2026-08-25): canonical -O2 -mips2 is exactly 0x1DC bytes but
 * first diverges at +0x0 with 20 differing words. The candidate matches
 * +0x4 through +0x64; the remaining blocker is the target's 0x38-byte frame
 * and pre-loop count/remaining/previous live-range allocation. The flag
 * lattice was neutral, and a bounded permuter run reached score 325.
 */
#ifdef NON_MATCHING
void overlay17DrawStrip(G **commands, Strip *strip) {
    Pair *pair, *previous, *start;
    s32 vertices, flush, textured;
    u32 remaining, segment;
    if (strip == 0) return;
    segment=0x80000000U;
    textured = strip->material ? 1 : 0;
    textured |= (overlay17PrepareStripReloc(commands, strip->material, 0x1F, 0), 0);
    PRIM((*commands)++);
    vertices=strip->count;
    remaining=vertices;
    previous=0;
    pair=strip->buffers[strip->buffer];
    start=0; vertices=0; flush=0;
    if (remaining != 0) {
        remaining--;
        do {
            vertices += 2;
            if (remaining == 0) {
                flush=1;
                goto check_flush;
            }
            if (pair->first.a == 0) {
                flush=1;
                goto check_flush;
            }
            if (start == 0) {
                if (previous != 0) { start=previous; vertices=4; }
                else { start=pair; vertices=2; }
            }
            if (vertices >= 17) flush=1;
check_flush:
            if (flush) {
                flush=0;
                if (start != 0) {
                    VTX((*commands)++, (void *)((u32)start+segment), vertices);
                    STRIP((*commands)++, (void *)((u32)strip->triangles+segment), vertices-2,
                          textured);
                    start=0;
                }
                vertices=0;
            }
            previous=pair++;
        } while (remaining--);
    }
    SYNC((*commands)++);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o017/overlay17DrawStrip/func_overlay_017_F00008B4_187426C.s")
#endif
