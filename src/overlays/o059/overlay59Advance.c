#include "PR/ultratypes.h"

typedef struct Overlay59Value {
    u8 first;
    s8 second;
} Overlay59Value;

typedef struct Overlay59Entry {
    f32 x;
    f32 y;
    Overlay59Value values[8];
    s16 count;
    s16 position;
    s16 timer;
    u16 state;
    void *owner;
    void *handles[8];
} Overlay59Entry;

typedef struct Overlay59TableInfo {
    s32 count;
    s32 unused;
} Overlay59TableInfo;

extern Overlay59Entry gOverlay59Entries[];
extern Overlay59Entry gOverlay59EntriesOneEnd[];
extern Overlay59TableInfo gOverlay59TableInfo[];
extern f32 gOverlay59ApproachFactor;
extern s32 overlay59PrepareEntry(Overlay59Entry *entry, s32 tableIndex,
                                 s32 itemIndex);
extern void overlay59Release(Overlay59Entry *entry);

/*
 * Overlay 59 text +0x36C..+0x784. Natural source reproduces the exact body,
 * frame, CFG, loops, calls, integer/FP allocation, and effects. A bounded
 * prologue permutation plus asserted local relocation addends selects the
 * retail schedule while the original overlay relocation assets stay retained.
 */
#ifdef NON_MATCHING
void overlay59Advance(s32 steps) {
    Overlay59Entry *entry;
    Overlay59Entry *current;
    Overlay59Value *cursor;
    f32 factor;
    f32 x;
    f32 y;
    s32 remaining;
    s32 repeat;
    s32 i;
    s32 count;
    s32 target;
    s32 stateOne;
    s32 stateTwo;
    s32 stateFour;
    s32 stateFive;

    entry = gOverlay59Entries;
    factor = gOverlay59ApproachFactor;
    stateOne = 1;
    stateTwo = 2;
    stateFour = 4;
    stateFive = 5;
    do {
        current = entry;
        remaining = steps;
        do {
            repeat = 0;
            switch (entry->state) {
                case 0:
                    if (entry->count != 0) {
                        if (overlay59PrepareEntry(entry, entry->values[0].first, 0)) {
                            entry->position = 0;
                            entry->timer = 0;
                            entry->state = stateOne;
                            repeat = stateOne;
                        }
                    }
                    break;

                case 1:
                    if (remaining != 0 && entry->timer < 30) {
                        do {
                            x = entry->x;
                            y = entry->y;
                            entry->x += (1.0f - x) * factor;
                            entry->y += (0.0f - y) * factor;
                            remaining--;
                            entry->timer++;
                        } while (remaining != 0 && current->timer < 30);
                    }
                    if (entry->timer >= 30) {
                        entry->x = 1.0f;
                        entry->y = 0.0f;
                        entry->timer = 0;
                        entry->state = stateTwo;
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    break;

                case 2:
                    entry->timer += remaining;
                    if (entry->timer >= 10) {
                        remaining = entry->timer - 10;
                        entry->timer = 0;
                        entry->state = 3;
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    break;

                case 3:
                    target = (gOverlay59TableInfo[entry->values[0].first].count - 1) << 8;
                    if (remaining != 0 && entry->position < target) {
                        do {
                            remaining--;
                            entry->position += 0x40;
                        } while (remaining != 0 && current->position < target);
                    }
                    if (entry->position >= target) {
                        entry->position = target;
                        entry->timer = 0;
                        entry->state = stateFour;
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    overlay59PrepareEntry(entry, entry->values[0].first,
                                          entry->position >> 8);
                    break;

                case 4:
                    entry->timer += steps;
                    if (entry->timer >= 60) {
                        remaining = entry->timer - 60;
                        entry->count--;
                        count = entry->count;
                        if (count != 0) {
                            i = 0;
                            if (count > 0) {
                                cursor = (Overlay59Value *) entry;
                                do {
                                    cursor[4].first = cursor[5].first;
                                    cursor[4].second = cursor[5].second;
                                    i++;
                                    cursor++;
                                } while (i < entry->count);
                            }
                            if (overlay59PrepareEntry(entry, entry->values[0].first, 0)) {
                                entry->position = 0;
                                entry->timer = 0;
                                entry->state = stateTwo;
                            }
                        }
                        if ((s16) entry->state == stateFour) {
                            entry->count = 0;
                            entry->timer = 0;
                            entry->state = stateFive;
                        }
                        if (remaining != 0) {
                            repeat = stateOne;
                        }
                    }
                    break;

                case 5:
                    if (entry->count != 0) {
                        if (overlay59PrepareEntry(entry, entry->values[0].first, 0)) {
                            entry->position = 0;
                            entry->timer = 0;
                            entry->state = stateOne;
                            repeat = stateOne;
                            break;
                        }
                        entry->count = 0;
                        entry->x = 0.0f;
                        entry->y = 1.0f;
                        entry->state = 0;
                        break;
                    }
                    if (remaining != 0 && entry->timer < 30) {
                        do {
                            x = entry->x;
                            y = entry->y;
                            entry->x += (0.0f - x) * factor;
                            entry->y += (1.0f - y) * factor;
                            remaining--;
                            entry->timer++;
                        } while (remaining != 0 && current->timer < 30);
                    }
                    if (entry->timer >= 30) {
                        overlay59Release(entry);
                        entry->x = 0.0f;
                        entry->y = 1.0f;
                        entry->state = 0;
                    }
                    break;
            }
        } while (repeat != 0);
        entry++;
    } while (entry != gOverlay59EntriesOneEnd);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o059/overlay59Advance/func_overlay_059_F000036C_18B8ABC.s")
#endif
