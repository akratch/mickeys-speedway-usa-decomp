#include "PR/ultratypes.h"

extern void *o63PollReloc(s32 arg0);
extern void o63TransitionReloc(void);
extern void o63SetModeReloc(s32 mode);
extern void o63SetColorReloc(s32 red, s32 green, s32 blue, s32 alpha, s32 intensity);
extern void o63DrawReloc(void *resource, s32 x, s32 y, void *state, s32 size);

extern void **gO63Sequence;
extern u8 gO63PendingStart[];
extern void *gO63SequenceStart[];
extern void *gO63PendingState;
extern void *gO63ActiveState;
extern s32 gO63SequenceIndex;
extern s32 gO63Timer;
extern s32 gO63TransitionEnabled;
extern u32 gO63ToggleReloc;
extern u8 gO63GraphicReloc[];

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 63 as none. */
/*
 * Plateau (5 structural attempts): the flag lattice's closest row is MIPS I,
 * eight bytes long with 82 positional words differing and the first mismatch
 * at +0x18; canonical MIPS II is four bytes short with its first mismatch at
 * +0x14.  The target keeps the poll result separate from the sequence-pointer
 * snapshot, but return-type, element-type, expression-association, explicit
 * next-index/timer temporaries, and a local sequence snapshot did not recover
 * that allocation without register-order guessing.
 */
#ifdef NON_MATCHING
void overlay63UpdateSequence(s32 delta) {
    void *token;

    token = o63PollReloc(0);
    if (token != 0) {
        if (gO63Sequence == 0) {
            gO63PendingState = gO63PendingStart;
            gO63Sequence = gO63SequenceStart;
            gO63SequenceIndex = 1;
        } else if (token == gO63Sequence[gO63SequenceIndex]) {
            gO63SequenceIndex++;
            if (gO63Sequence[gO63SequenceIndex] == 0) {
                gO63ToggleReloc = (gO63ToggleReloc ^ 1) & 1;
                gO63Sequence = 0;
                gO63Timer = 0xB4;
                gO63ActiveState = gO63PendingState;
            }
        } else {
            gO63Sequence = 0;
        }
    }

    if (gO63Timer > 0) {
        gO63Timer -= delta;
        if (gO63Timer <= 0 && gO63TransitionEnabled != 0) {
            o63TransitionReloc();
        }
        o63SetModeReloc(2);
        o63SetColorReloc(0, 0, 0, 0xFF, 0xFF);
        o63DrawReloc(gO63GraphicReloc, 0xA0, 0x3C, gO63ActiveState, 0xC);
        o63SetColorReloc(0, 0xFF, 0xFF, 0xFF, 0xFF);
        o63DrawReloc(gO63GraphicReloc, 0xA1, 0x3D, gO63ActiveState, 0xC);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o063/overlay63UpdateSequence/func_overlay_063_F000077C_18C3304.s")
#endif
