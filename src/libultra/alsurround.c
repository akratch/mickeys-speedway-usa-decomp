/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), whose source credits the
 * Perfect Dark decompilation; both are permitted sources under
 * docs/CLEANROOM.md. See docs/reference-findings.md section 3.
 */

#include <n_libaudio.h>
#include <ultra64.h>

#define SPEAKERMODE_MONO 1
#define SPEAKERMODE_STEREO 2
#define SPEAKERMODE_HEADPHONE 3
#define SPEAKERMODE_SURROUND 4

SpeakerMode D_800D7DC0;
u8 D_800D7DC4[2];
u8 D_800D7DC6[2];
u8 D_800D7DC8[4];

void alSurround_ReverbSetup(s32 index, s32 arg1);

void alSurround_OutputType(u8 mode) {
    s32 i;

    D_800D7DC0.surround = 0;
    D_800D7DC0.mono = 0;
    D_800D7DC0.headphone = 0;

    switch (mode) {
        case SPEAKERMODE_MONO:
            D_800D7DC0.mono = 1;
            break;
        case SPEAKERMODE_HEADPHONE:
            D_800D7DC0.headphone = 1;
            break;
        case SPEAKERMODE_SURROUND:
            D_800D7DC0.surround = 1;
            break;
    }

    for (i = 0; i < 2; i++) {
        alSurround_ReverbSetup(i, 0);
    }
}

void alSurround_ReverbSetup(s32 index, s32 arg1) {
    if (arg1 == 0) {
        arg1 = D_800D7DC8[index];
    }

    D_800D7DC4[index] = 0;
    D_800D7DC6[index] = 0;

    switch (arg1) {
        case 2:
            if (D_800D7DC0.surround) {
                D_800D7DC6[index] = 1;
            }
            break;
        case 3:
            if (D_800D7DC0.surround) {
                D_800D7DC4[index] = 1;
            }
            break;
        case 4:
            if (!D_800D7DC0.mono) {
                D_800D7DC4[index] = 1;
            }
            break;
        case 5:
            if (!D_800D7DC0.mono) {
                D_800D7DC4[index] = 1;
                D_800D7DC6[index] = 1;
            }
            break;
    }

    D_800D7DC8[index] = arg1;
}
