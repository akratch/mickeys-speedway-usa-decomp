#include "PR/ultratypes.h"

typedef struct Overlay74UpdateState {
    u8 strength;
    u8 channel;
    s8 minimum;
    s8 maximum;
} Overlay74UpdateState;

typedef struct Overlay74UpdateObject {
    s16 angle;
    u8 pad02[4];
    s16 flags;
    u8 pad08[4];
    f32 x;
    f32 y;
    s32 z;
    u8 pad18[0x4C];
    Overlay74UpdateState *state;
} Overlay74UpdateObject;

extern u32 gOverlay74Flags;

/* Runtime identities: func_8005776C, func_800291B4, amSndPlay, func_8003A680.
 * Resident call surfaces authenticate these prototypes. */
s32 overlay74QueryReloc(f32 x, f32 y, s32 z, f32 strength, s32 enabled,
                        Overlay74UpdateObject **results);
void overlay74HitReloc(void);
void overlay74SoundReloc(u16 soundId, void **handle);
void overlay74RewardReloc(s32 count);

/* NON_MATCHING plateau (reproved 2026-08-29): policy-clean configured C is
 * exact-sized at 100 words with all eight required relocation offsets/types,
 * but differs in 39 relocation-masked words from +0x0 and uses a 0x70 frame
 * instead of the target's 0x60. The extra 16 non-save bytes shift the result
 * array and cascade through the integer pool/temp register assignments. All
 * 119 flag-lattice rows were attempted; the 53 supported configurations
 * compile, canonical -O2 -mips2 -32 is best, and the remaining rows are
 * rejected by the compiler/driver before code generation. Reversing the outer
 * OR at +0x124 is byte-identical to V0. The source keeps the authenticated
 * resident ABIs, natural 13-pointer hit array, and integer carriers. Its eight
 * offsets/types agree with the retained runtime records,
 * including both gOverlay74Flags pairs resolved through reserved selector
 * 0xFFF/addend 0x4D6E8 to D_800D3128; the assembled target retains only four
 * static call relocations. Exact pinned DKR v77/v80/JFG range scans found no
 * donor; no attributable near-match oracle survives.
 * Historical forced-color and permuter scores have no surviving attributable
 * variant objects and remain scheduling context only. The bounded clean-source
 * route is exhausted; linked equality proves the assembly fallback only. */
#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay74Update
 * score: 39 differing words
 * frame: 0x70
 * relocations: 8
 * first-mismatch: +0x0
 * summary: Target frame is 0x60; 16 extra non-save bytes shift the result array and cascade pool/temp assignments; flags and outer-OR order are exhausted.
 */
void overlay74Update(Overlay74UpdateObject *object, s32 amount) {
    Overlay74UpdateObject *results[13];
    Overlay74UpdateObject *hitObject;
    f32 delta;
    Overlay74UpdateState *state;
    s32 flagBits;
    s32 count;
    s32 mask;

    if (!(object->flags & 0x400)) {
        object->angle += amount << 8;
        state = object->state;
        if (overlay74QueryReloc(object->x, object->y, object->z,
                                (f32)state->strength, 1, results) > 0) {
            hitObject = results[0];
            delta = hitObject->y - object->y;
            if ((hitObject->state->strength == 0) &&
                ((f32)state->minimum < delta) &&
                (delta < (f32)state->maximum)) {
                object->flags |= 0x400;
                *(u16 *)&gOverlay74Flags =
                    (*(u16 *)&gOverlay74Flags & 0xF87F) |
                    ((((((gOverlay74Flags << 5) >> 28) |
                        (1 << state->channel)) << 1) << 6) & 0x780);
                overlay74HitReloc();
                overlay74SoundReloc(0x27C, 0);

                count = 5;
                flagBits = (gOverlay74Flags << 5) >> 28;
                mask = 8;
                do {
                    if (flagBits & mask) {
                        count++;
                    }
                    mask >>= 1;
                } while (mask != 0);
                if (count >= 6) {
                    overlay74RewardReloc(count);
                }
            }
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o074/overlay74Update/func_overlay_074_F00000B8_18CBD58.s")
#endif
