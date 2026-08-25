#ifndef GAME_TRACK_H
#define GAME_TRACK_H

#include "PR/ultratypes.h"

/*
 * PROVENANCE: field grouping and the FogData starting point come from Jet
 * Force Gemini's public `src/track.c`. Offsets and the 0x40-byte stride are
 * re-derived from Mickey's fog functions; Mickey's layout wins where they
 * differ.
 */
typedef struct TrackFogValues {
    s32 r;
    s32 g;
    s32 b;
    s32 near;
    s32 far;
} TrackFogValues;

typedef struct TrackFogCompact {
    u8 r;
    u8 g;
    u8 b;
    s8 state;
    s16 near;
    s16 far;
} TrackFogCompact;

typedef struct TrackFog {
    TrackFogValues fog;
    s32 initialNear;
    s32 targetNear;
    TrackFogValues addFog;
    TrackFogCompact intendedFog;
    s32 switchTimer;
    void *fogChanger;
} TrackFog;

extern TrackFog D_800C99C0[4];
extern s32 D_800C9558;
extern void *D_800C9550;

void trackSkySet(s32 skyDome);
void *trackGetTrack(void);
void trackSetFog(s32 fogIndex, s16 near, s16 far, s16 targetNear,
                 u8 red, u8 green, u8 blue, s8 state);
void trackGetFog(s32 playerID, s16 *near, s16 *far, s16 *targetNear,
                 u8 *red, u8 *green, u8 *blue, s8 *state);

#endif
