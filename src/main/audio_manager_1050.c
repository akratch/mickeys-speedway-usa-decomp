/*
 * Resident audio front end -- ROM 0x1050-0x2340 (VRAM 0x80000450).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_1050.c,
 * which is a permitted source under docs/CLEANROOM.md. Adapted bodies carry
 * their own point-of-use disclosure. Mickey's boundaries remain authoritative.
 *
 * The end is the 16-byte-aligned start of Mickey's audiomgr-shaped run:
 * func_80001740 has the allocator, message-queue, and audio-thread setup shape
 * of JFG's first audiomgr routine. The two preceding Mickey-only routines stay
 * with this TU; no unaligned boundary is asserted from JFG's layout.
 *
 * Flags: -O2 -mips2 -32, from the measured src/main/ flag group.
 */

#include "PR/ultratypes.h"

typedef struct AudioSequencePlayer {
    u8 pad0[0x2C];
    s32 state;
} AudioSequencePlayer;

typedef struct AudioSoundData {
    u16 soundBite;
    u8 volume;
    u8 minVolume;
    u8 pitch;
    u8 unk5;
    u16 range;
    u8 priority;
    u8 unk9;
} AudioSoundData;

typedef struct AudioEnvelope {
    s32 attackTime;
    s32 decayTime;
} AudioEnvelope;

typedef struct AudioSound {
    AudioEnvelope *envelope;
} AudioSound;

typedef struct AudioInstrument {
    u8 pad0[0xE];
    s16 soundCount;
    AudioSound *soundArray[1];
} AudioInstrument;

typedef struct AudioBank {
    u8 pad0[0xC];
    AudioInstrument *instArray[1];
} AudioBank;

typedef struct AudioBankFile {
    s32 revision;
    AudioBank *bankArray[1];
} AudioBankFile;

extern s32 D_80078D7C;
extern s32 D_80078D80;
extern s32 D_80078D78;
extern s32 D_80078D8C;
extern void *D_80078D60;
extern void *D_80078D64;
extern u8 D_80078D68;
extern u8 D_80078D6C;
extern u8 D_80078D74;
extern u8 D_80078D88;
extern u8 D_80078DB0;
extern u8 D_800BF794;
extern u8 D_800BF795;
extern u32 *D_800BF798;
extern AudioBankFile *D_800BF79C;
extern AudioSoundData *D_800BF7A0;
extern s32 D_800BF7A8;
extern s32 D_800BF7B0;
extern u8 *D_800BF7A4;
extern void gsSndpSetParam(void *sound, s16 type, u32 value);
extern u32 gsSndpGetGlobalVolume(void);
extern void gsSndpStop(void *sound);
extern void n_alCSPSetChlVol(void *player, u8 channel, u8 volume);
extern void n_alCSPSetVol(void *player, s16 volume);
extern void n_alCSPNew(void *player, void *config);
extern void n_alCSPSetMessageQ(void *player, void *queue);
extern void func_800005CC(f32 fade, s32 volume);
extern s32 amDittyPlaying(void);
extern void func_80001308(u8 value, void *player);
extern void func_80001568(void *player);
extern u16 func_800016C8(s32 volume);
extern u16 amGetSfxCount(void);
extern void *ad_sndp_play(void *bank, s16 soundBite, u16 volume, u8 pan,
                          f32 pitch, u8 arg5, void **handle);

#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000450.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000510.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000594.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800005CC.s")
#ifdef NON_MATCHING
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_1050.c.
 *
 * Plateau: the best stock-flag candidate has the exact instruction count,
 * opcode schedule, frame, and relocation surface. Seven temporary-register
 * sites differ, beginning at function offset 0x1C. Ten source-shape attempts
 * and the flag lattice did not close IDO's temporary-FIFO phase difference.
 */
void amTuneSetFadeScaled(f32 fade, u8 volume) {
    s32 scaled;

    if (volume > 0x7F) {
        volume = 0x7F;
    }
    scaled =
        (s32)((u32)*(D_800BF7A4 + (D_800BF794 * 3)) * volume) / 0x7F;
    func_800005CC(fade, scaled & 0xFF);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/amTuneSetFadeScaled.s")
#endif
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneResetFade(void) {
    D_80078D7C = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_8000073C.s")
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amAmbientResetFade(void) {
    D_80078D80 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000838.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000ABC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000B3C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000B48.s")
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amTuneMuteChl(s32 channel) {
}
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amTuneUnmuteChl(s32 channel) {
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetChlVolume(u8 channel, u8 volume) {
    if (channel < 16) {
        n_alCSPSetChlVol(D_80078D60, channel, volume);
    }
}
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amTuneResetChls(void) {
    s32 channel;
    u8 maskedChannel;

    if (D_80078D78 == 0) {
        channel = 0;
        do {
            maskedChannel = channel;
            amTuneUnmuteChl(maskedChannel & 0xFF);
            amTuneSetChlVolume(maskedChannel, 0x7F);
            channel++;
        } while (channel != 16);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c
 * music_jingle_play_safe; JFG src/audio_manager_1050.c supplies the official
 * amAmbientPlay name. Mickey's globals and call target remain authoritative.
 */
void amAmbientPlay(u8 value) {
    if (amDittyPlaying() == 0) {
        func_80001308(D_800BF795 = value, D_80078D64);
        D_80078D88 = 1;
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_stop; JFG
 * src/audio_manager_1050.c supplies the official amTuneStop name.
 */
void amTuneStop(void) {
    if (D_80078D78 == 0) {
        func_80001568(D_80078D60);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_jingle_stop; JFG
 * src/audio_manager_1050.c supplies the official amAmbientStop name.
 */
void amAmbientStop(void) {
    if (amDittyPlaying() == 0) {
        D_800BF795 = 0;
        func_80001568(D_80078D64);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_current_sequence;
 * JFG src/audio_manager_1050.c supplies the official amTuneGetSeqNo name.
 */
u8 amTuneGetSeqNo(void) {
    if (D_800BF794 != 0 &&
        ((AudioSequencePlayer *)D_80078D60)->state == 1) {
        return D_800BF794;
    }
    return 0;
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_jingle_current;
 * JFG src/audio_manager_1050.c supplies the official amAmbientGetSeqNo name.
 */
u8 amAmbientGetSeqNo(void) {
    return D_800BF795;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetVolume(u8 volume) {
    s32 scaledVolume;

    if (volume > 0x7F) {
        volume = 0x7F;
    }
    D_80078D68 = volume;
    scaledVolume = D_80078D8C * D_80078D68;
    n_alCSPSetVol(D_80078D60, scaledVolume);
    D_80078DB0 = 1;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetGlobalVolume(u32 volume) {
    s32 scaledVolume;

    if (volume > 0x100) {
        volume = 0x100;
    }
    D_80078D8C = volume;
    scaledVolume = D_80078D8C * D_80078D68;
    n_alCSPSetVol(D_80078D60, scaledVolume);
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_volume; JFG
 * src/audio_manager_1050.c supplies the official amTuneGetVolume name.
 */
u8 amTuneGetVolume(void) {
    return D_80078D68;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amAmbientSetVolume(u8 volume) {
    D_80078D6C = volume;
    n_alCSPSetVol(D_80078D64,
                  (s16)(gsSndpGetGlobalVolume() * D_80078D6C));
}
/*
 * PROVENANCE: body shape and official name adapted from JFG
 * asm/nonmatchings/audio_manager_1050/amDittyPlay.s; Mickey's operands and
 * boundaries remain authoritative.
 */
void amDittyPlay(u8 sequenceId) {
    if (D_800BF798[sequenceId] <= 0x1000) {
        D_80078D74 = 1;
        func_80001308(D_800BF795 = sequenceId, D_80078D64);
    }
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c music_jingle_playing;
 * JFG src/audio_manager_1050.c supplies the official amDittyPlaying name.
 */
s32 amDittyPlaying(void) {
    if (D_800BF795 != 0 && D_80078D74 != 0 &&
        ((AudioSequencePlayer *)D_80078D64)->state == 1) {
        return D_800BF795;
    }
    D_80078D74 = 0;
    return 0;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amSndStop(void *sound) {
    gsSndpStop(sound);
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c sound_play; JFG's
 * src/audio.h and asm/nonmatchings/audio_manager_1050/amSndPlay.s supply the
 * official name, SoundData layout, exact boundary, and direct-player call.
 */
void amSndPlay(u16 soundId, void **handle) {
    f32 pitch;
    s32 soundBite;

    if (soundId > D_800BF7A8) {
        if (handle != NULL) {
            *handle = NULL;
        }
        return;
    }
    soundBite = D_800BF7A0[soundId].soundBite;
    if (soundBite == 0) {
        if (handle != NULL) {
            *handle = NULL;
        }
        return;
    }
    pitch = (f32)D_800BF7A0[soundId].pitch / 100.0f;
    ad_sndp_play(D_800BF79C->bankArray[0], soundBite,
                 func_800016C8(D_800BF7A0[soundId].volume << 8), 0x40, pitch,
                 0, handle);
}
/*
 * PROVENANCE: official name, parameter role, and body shape adapted from JFG
 * asm/nonmatchings/audio_manager_1050/amSndPlayDirect.s; Mickey's boundary,
 * branch form, globals, and calls remain authoritative.
 */
void amSndPlayDirect(u16 soundBite, u8 volume, u8 pan, f32 pitch, u8 arg4,
                     void **handle) {
    if (soundBite <= 0 || amGetSfxCount() < soundBite) {
        if (handle != NULL) {
            *handle = NULL;
        }
        return;
    }
    ad_sndp_play(D_800BF79C->bankArray[0], soundBite,
                 func_800016C8(volume << 8), pan, pitch, arg4, handle);
}
/*
 * PROVENANCE: body shape adapted from DKR src/audio.c
 * sound_volume_set_relative; JFG supplies the official amSndSetVol name.
 */
void amSndSetVol(u16 soundId, void *sound, u8 volume) {
    s32 scaledVolume;

    scaledVolume =
        (s32)(D_800BF7A0[soundId].volume * (volume / 127.0f)) * 256;
    if (sound != NULL) {
        gsSndpSetParam(sound, 8, func_800016C8(scaledVolume));
    }
}
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amSndSetPan(void *sound, u32 pan) {
    if (sound != NULL) {
        gsSndpSetParam(sound, 4, pan);
    }
}
/*
 * PROVENANCE: body and official name adapted from DKR src/audio.c
 * sound_pitch_set and JFG src/audio_manager_1050.c.
 */
void amSndSetPitchDirect(void *sound, u32 pitch) {
    u32 *pitchAddress = &pitch;

    if (sound != NULL) {
        gsSndpSetParam(sound, 16, *pitchAddress);
    }
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
u16 amGetSfxCount(void) {
    return D_800BF79C->bankArray[0]->instArray[0]->soundCount;
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amGetSfxSettings(AudioSoundData **table, s32 *size, s32 *count) {
    if (table != NULL) {
        *table = D_800BF7A0;
    }
    if (size != NULL) {
        *size = D_800BF7B0;
    }
    if (count != NULL) {
        *count = D_800BF7A8;
    }
}
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
u8 amSoundIsLooped(u16 soundId) {
    if (soundId <= 0 ||
        D_800BF79C->bankArray[0]->instArray[0]->soundCount < soundId) {
        return 0;
    }
    return ((u32)(1 + D_800BF79C->bankArray[0]->instArray[0]
                          ->soundArray[soundId - 1]
                          ->envelope->decayTime) == 0);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001308.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_8000137C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001568.s")
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneSetReverbOnOff(s32 enabled) {
}
void func_800015F8(void) {
    D_80078D78 = 1;
}
void func_80001608(void) {
    D_80078D78 = 0;
}
s32 func_80001614(void) {
    return D_80078D78;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001620.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001668.s")
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void forcelink(void) {
    n_alCSPNew(NULL, NULL);
    n_alCSPSetMessageQ(NULL, NULL);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800016C8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800016EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001708.s")
