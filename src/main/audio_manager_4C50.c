/*
 * Resident audio oscillators -- ROM 0x45F0-0x4F40 (VRAM 0x800039F0).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_4C50.c,
 * which is a permitted source under docs/CLEANROOM.md. Adapted bodies carry
 * their own point-of-use disclosure. Mickey's boundaries remain authoritative.
 *
 * Both ends are measured: amVibratoInit and _depth2Cents are tier-A JFG byte
 * identities, all five functions preserve JFG's order, and _depth2Cents owns
 * 0x50 executable bytes followed by 0xC bytes of terminal alignment.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul, confirmed by the flag lattice.
 */

#include "PR/ultratypes.h"

typedef struct AudioOscillatorState {
    struct AudioOscillatorState *next;
    u8 type;
    u8 phase;
    u16 period;
    u16 counter;
    u16 padA;
    union {
        struct {
            u8 depth;
            u8 base;
            u8 alternate;
        } tremolo;
        struct {
            f32 depth;
            f32 alternate;
        } vibrato;
        struct {
            s32 base;
            s32 depth;
        } cents;
    } data;
} AudioOscillatorState;

extern f32 __sinf(f32 angle);
extern f32 alCents2Ratio(s32 cents);
f32 _depth2Cents(u8 depth);
extern f32 D_80080BA0;
extern f32 D_80080BA4;
extern f32 D_80080BA8;
extern AudioOscillatorState *D_800C9300;
extern AudioOscillatorState D_800C9308[16];

/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_4C50.c
 * amVibratoInit; free-list body adapted from BK src/core1/code_1D00.c
 * audioManager_setupSeqp. Mickey's 16-state pool remains authoritative.
 */
void amVibratoInit(void) {
    AudioOscillatorState *state;
    s32 i;

    D_800C9300 = &D_800C9308[0];
    state = &D_800C9308[0];
    for (i = 0; i < 15; i++) {
        state->next = &D_800C9308[i + 1];
        state = state->next;
    }
    state->next = NULL;
}
/*
 * PROVENANCE: body adapted from Perfect Dark src/lib/naudio/osc.c osc_init
 * and compared with JFG src/audio_manager_4C50.c amInitOsc; Mickey's state
 * layout and oscillator case set remain authoritative.
 */
s32 amInitOsc(void **oscillatorState, f32 *initValue, u8 oscillatorType,
              u8 rate, u8 depth, u8 delay, u8 timeIndex) {
    AudioOscillatorState *state;
    s32 interval = 0;
    s32 cents;

    if (D_800C9300 != NULL) {
        state = D_800C9300;
        D_800C9300 = state->next;
        state->type = oscillatorType;
        *oscillatorState = state;
        interval = delay << 14;

        switch (oscillatorType) {
        case 1:
            state->counter = 0;
            state->period = 0x103 - rate;
            state->data.tremolo.depth = depth >> 1;
            state->data.tremolo.base = 0x7F - state->data.tremolo.depth;
            *initValue = state->data.tremolo.base;
            break;
        case 2:
            state->period = 0x100 - rate;
            state->counter = state->period;
            state->phase = 0;
            state->data.tremolo.alternate = 0x7F - depth;
            state->data.tremolo.base = 0x7F;
            state->data.tremolo.depth = 0x7F;
            *initValue = 127.0f;
            break;
        case 3:
            state->period = 0x100 - rate;
            state->counter = 0;
            state->data.tremolo.depth = 0x7F;
            state->data.tremolo.base = depth;
            *initValue = state->data.tremolo.depth;
            break;
        case 4:
            state->period = 0x100 - rate;
            state->counter = 0;
            state->data.tremolo.base = depth;
            state->data.tremolo.depth = 0x7F - depth;
            *initValue = state->data.tremolo.depth;
            break;
        case 0x80:
            state->data.vibrato.depth = _depth2Cents(depth);
            state->counter = 0;
            state->period = 0x103 - rate;
            *initValue = 1.0f;
            break;
        case 0x81:
            state->period = 0x100 - rate;
            state->counter = state->period;
            state->phase = 0;
            cents = (s32)_depth2Cents(depth);
            state->data.vibrato.depth = alCents2Ratio(-cents);
            state->data.vibrato.alternate = alCents2Ratio(cents);
            *initValue = state->data.vibrato.alternate;
            break;
        case 0x82:
            state->period = 0x100 - rate;
            state->counter = state->period;
            cents = (s32)_depth2Cents(depth);
            state->data.cents.depth = cents * 2;
            state->data.cents.base = cents;
            *initValue = alCents2Ratio(cents);
            break;
        case 0x83:
            state->period = 0x100 - rate;
            state->counter = state->period;
            cents = (s32)_depth2Cents(depth);
            state->data.cents.base = -cents;
            state->data.cents.depth = cents * 2;
            *initValue = alCents2Ratio(state->data.cents.base);
            break;
        }
    }
    return interval;
}
/*
 * PROVENANCE: body adapted from Perfect Dark src/lib/naudio/osc.c osc_update
 * and compared with JFG src/audio_manager_4C50.c amUpdateOsc; Mickey's state
 * layout and oscillator case set remain authoritative.
 */
s32 amUpdateOsc(void *oscillatorState, f32 *updateValue) {
    f32 phase;
    AudioOscillatorState *state = oscillatorState;
    s32 interval = 16000;

    switch (state->type) {
    case 1:
        state->counter++;
        if (state->counter >= state->period) {
            state->counter = 0;
        }
        phase = (f32)state->counter / (f32)state->period;
        phase = __sinf(phase * D_80080BA0);
        phase = state->data.tremolo.depth * phase;
        *updateValue = state->data.tremolo.base + phase;
        break;
    case 2:
        if (state->phase == 0) {
            *updateValue = state->data.tremolo.alternate;
            state->phase = 1;
        } else {
            *updateValue = state->data.tremolo.base;
            state->phase = 0;
        }
        interval = state->period * 16000;
        break;
    case 3:
        state->counter++;
        if (state->counter > state->period) {
            state->counter = 0;
        }
        phase = (f32)state->counter / (f32)state->period;
        phase *= state->data.tremolo.base;
        *updateValue = state->data.tremolo.depth - phase;
        break;
    case 4:
        state->counter++;
        if (state->counter > state->period) {
            state->counter = 0;
        }
        phase = (f32)state->counter / (f32)state->period;
        phase *= state->data.tremolo.base;
        *updateValue = state->data.tremolo.depth + phase;
        break;
    case 0x80:
        state->counter++;
        if (state->counter >= state->period) {
            state->counter = 0;
        }
        phase = (f32)state->counter / (f32)state->period;
        phase = __sinf(phase * D_80080BA4);
        phase *= state->data.vibrato.depth;
        *updateValue = alCents2Ratio((s32)phase);
        break;
    case 0x81:
        if (state->phase == 0) {
            state->phase = 1;
            *updateValue = state->data.vibrato.depth;
        } else {
            state->phase = 0;
            *updateValue = state->data.vibrato.alternate;
        }
        interval = state->period * 16000;
        break;
    case 0x82:
        state->counter++;
        if (state->counter > state->period) {
            state->counter = 0;
        }
        phase = (f32)state->counter / (f32)state->period;
        phase *= (f32)state->data.cents.depth;
        phase = (f32)state->data.cents.base - phase;
        *updateValue = alCents2Ratio((s32)phase);
        break;
    case 0x83:
        state->counter++;
        if (state->counter > state->period) {
            state->counter = 0;
        }
        phase = (f32)state->counter / (f32)state->period;
        phase *= (f32)state->data.cents.depth;
        phase += (f32)state->data.cents.base;
        *updateValue = alCents2Ratio((s32)phase);
        break;
    }
    return interval;
}
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_4C50.c amStopOsc;
 * body uses Mickey-only evidence.
 */
void amStopOsc(AudioOscillatorState *state) {
    state->next = D_800C9300;
    D_800C9300 = state;
}

/* PROVENANCE: body adapted from JFG src/audio_manager_4C50.c and BK src/core1/code_1D00.c. */
f32 _depth2Cents(u8 depth) {
    f32 ratio;
    f32 result;

    ratio = D_80080BA8;
    result = 1.0f;
    while (depth) {
        if (depth & 1) {
            result *= ratio;
        }
        ratio *= ratio;
        depth >>= 1;
    }
    return result;
}
