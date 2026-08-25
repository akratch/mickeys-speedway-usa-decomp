#include "PR/ultratypes.h"
#include "n_audio/mbi.h"

extern Gfx D_0[];
extern Gfx D_50[];
extern Gfx D_88[];

extern s32 viGetVideoMode(void);
extern void func_80034920(Gfx **displayList);
extern void rsp_segment(Gfx **displayList, s32 segment, void *base);
extern void viGetCurrentSize(u32 *width, u32 *height);

/*
 * PROVENANCE: JFG asm/nonmatchings/fx/fxScreenEffect.s was used as the
 * nearest structural oracle for this display-list command family. Mickey's
 * own function supplies the body, resident bindings, and framebuffer loop.
 */
/*
 * Plateau (2026-08-25, 10 attempts): -O2 -g3 -mips2 emits 195 instructions
 * versus the 204-instruction target and differs in 182 positional words. The
 * first mismatch is +0x0: a 0x78-byte frame versus 0xA8. The remaining gap is
 * callee-save allocation and the loop-entry/texture-command schedule. The full
 * flag lattice found no closer codegen group; the bounded permuter was blocked
 * by the checkout's unrelated diCpuThreadStack splat-symbol overlap.
 */
#ifdef NON_MATCHING
void func_overlay_066_F00004E0_18C6948(register Gfx **displayList,
                                       register u16 *framebuffer,
                                       register void *segmentBase) {
    register Gfx *textureCommands;
    u32 width;
    u32 height;
    s32 y;
    s32 previousY;
    s32 limit;
    s32 videoMode;

    videoMode = viGetVideoMode();
    if (videoMode != 2) {
        if (videoMode != 3) {
            textureCommands = D_50;
        } else {
            textureCommands = D_88;
        }
    } else {
        textureCommands = D_88;
    }

    gDPPipeSync((*displayList)++);
    func_80034920(displayList);
    rsp_segment(displayList, 0, 0);
    rsp_segment(displayList, 1, segmentBase);
    rsp_segment(displayList, 2, 0);
    rsp_segment(displayList, 4, (u8 *)segmentBase - 0x500);
    gDPSetColorImage((*displayList)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320,
                     (void *)0x01000000);
    viGetCurrentSize(&width, &height);
    gDPSetScissor((*displayList)++, G_SC_NON_INTERLACE, 0, 0,
                  width - 1, height - 1);
    gSPDisplayList((*displayList)++, D_0);
    gDPSetPrimColor((*displayList)++, 0, 0, 255, 255, 255, 255);

    limit = 0x3C0;
    y = 0;
    if (y < limit) {
        do {
            previousY = y;
            y += 0x10;

            (*displayList)->words.w0 = textureCommands->words.w0;
            (*displayList)->words.w1 = (u32)framebuffer;
            framebuffer += 0x500;
            (*displayList)++;

            gDma1p((*displayList)++, 7,
                   (u32)textureCommands + 0x80000008, 0x30, 6);
            if (y > limit) {
                y = limit;
            }
            gSPTextureRectangle((*displayList)++, 0, previousY, 0x500, y,
                                G_TX_RENDERTILE, 0, 0, 0x400, 0x400);
        } while (y < limit);
    }

    gDPPipeSync((*displayList)++);
    func_80034920(displayList);
    rsp_segment(displayList, 1, (void *)D_0->words.w0);
    gDPSetPrimColor((*displayList)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*displayList)++, 255, 255, 255, 255);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o066/func_overlay_066_F00004E0_18C6948/func_overlay_066_F00004E0_18C6948.s")
#endif
