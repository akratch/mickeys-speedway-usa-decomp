#include "PR/ultratypes.h"

/*
 * Overlay 76 function map: +0x000, +0x038, +0x0D0; text ends at +0x120.
 * Fresh exact scans against DKR v77/v80 and JFG are negative. Two SYMBOL
 * relocations prove the resident calls (sound dispatch and mathRnd); twelve
 * LOCAL records prove one 16-byte data range and the 0x20-byte status BSS.
 */
typedef struct {
    s32 index;
} Overlay76Record;

typedef struct {
    u8 pad0[0x0C];
    s32 x;
    s32 y;
    s32 z;
    u8 pad18[0x4C];
    Overlay76Record *record;
} Overlay76Object;

extern s32 gOverlay76NextIndex;
extern s32 gOverlay76Status[8];

void overlay76SoundReloc(u16 soundId, s32 x, s32 y, s32 z, s32 arg4, s32 arg5);
s32 overlay76RandomReloc(s32 min, s32 max);

void overlay76Register(Overlay76Object *object, volatile f32 unused) {
    s32 *next = &gOverlay76NextIndex;
    Overlay76Record *record = object->record;

    record->index = *next;
    gOverlay76Status[*next] = 0;
    (*next)++;
}

void overlay76Update(Overlay76Object *object, f32 unused) {
    Overlay76Record *record = object->record;
    s32 index = record->index;
    s32 soundId;

    if (gOverlay76Status[index] == 1) {
        soundId = (index & 1) ? 0x205 : 0x206;
        overlay76SoundReloc(soundId, object->x, object->y, object->z, 4, 0);
        gOverlay76Status[record->index] = 0;
    }
}

void overlay76TriggerRandom(void) {
    s32 count = gOverlay76NextIndex;

    if (count > 0) {
        gOverlay76Status[overlay76RandomReloc(0, count - 1)] = 1;
    }
}
