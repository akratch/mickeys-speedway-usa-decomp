#include "overlays/overlay_007.h"

typedef struct Overlay7RuntimeState {
    u8 pad000[1];
    s8 kind;
    u8 pad002[0x1A6];
    u16 flags;
    u8 pad1AA[0x1DB];
    u8 level;
    u8 previousLevel;
    u8 pad387[0x2C];
    u8 cooldown;
    u8 pad3B4[0x10];
    f32 difference;
    f32 scale;
} Overlay7RuntimeState;

typedef struct Overlay7RuntimeObject {
    u8 pad00C[0xC];
    s32 x;
    s32 y;
    s32 z;
    u8 pad018[0x4C];
    Overlay7RuntimeState *state;
} Overlay7RuntimeObject;

extern s32 D_4;
extern Overlay7Entry *D_290;
extern u16 D_2AA;
extern Overlay7Pair D_844[];
extern void *overlay7RuntimeGateReloc;
extern void *overlay7RuntimeHandleReloc;
extern s32 overlay7RuntimeTimerReloc;
extern Overlay7RuntimeObject *overlay7RuntimeLastObjectReloc;
extern s32 overlay7RuntimeModeReloc;
extern u8 overlay7RuntimeLevelReloc;
extern u8 overlay7RuntimePreviousLevelReloc;
extern f32 overlay7RuntimeScaleReloc;
extern u8 overlay7RuntimeValuesReloc[];

extern Overlay7RuntimeObject **overlay7CollectRuntimeObjectsReloc(s32 *count);
extern void overlay7ContinueRuntimeObjectReloc(void *handle, s32 x, s32 y,
                                               s32 z);
extern f32 overlay7MeasureRuntimeObjectAReloc(Overlay7RuntimeObject *object);
extern f32 overlay7MeasureRuntimeObjectBReloc(Overlay7RuntimeObject *object);
extern void overlay7RecordRuntimeEventReloc(s32 event);
extern s32 overlay7GetRuntimeIndexReloc(s32 arg0, s32 arg1);
extern void overlay7StartRuntimeValueReloc(u8 value);
extern void overlay7ReleaseRuntimeOwnerReloc(void *owner);
extern s32 overlay7AdjustRuntimeValueReloc(s32 arg0, s32 value);
extern void *overlay7CreateRuntimeOwnerReloc(u16 value, s32 x, s32 y, s32 z,
                                             s32 kind, void *argument);
extern void overlay7ReleaseRuntimeHandleReloc(void *handle);
extern void overlay7ReleaseRuntimeEntryReloc(Overlay7Entry *entry);
extern s32 overlay7RuntimeChanceReloc(s32 minimum, s32 maximum);
extern void overlay7SetRuntimeModeReloc(Overlay7RuntimeObject *object,
                                        s32 mode);

/* Workbench: mixed structure/schedule/register, 348 words exact-size; 242 raw differ, first +0x0.
 * Levers 1 and 26 tried after the prior lattice: count aggregate and disjoint local reuses did not shrink the frame.
 * Remaining: identical save bytes but 0x80 vs 0x78 frame; private relocation scheduling drives the temp homes. */
#ifdef NON_MATCHING
void func_overlay_007_F0000324_185C1AC(s32 arg0, s32 elapsed) {
    s32 count;
    Overlay7RuntimeObject **objects;
    Overlay7RuntimeObject **cursor;
    u16 *timer;
    s32 remaining;
    Overlay7RuntimeObject *object;
    Overlay7RuntimeObject *handleObject;
    Overlay7Entry *entry;
    Overlay7RuntimeState *state;
    s32 index;
    f32 difference;
    s32 found;
    u16 value;

    objects = overlay7CollectRuntimeObjectsReloc(&count);
    timer = &D_2AA;
    remaining = 9;
    do {
        if (*timer != 0) {
            if (*timer >= elapsed) {
                *timer -= elapsed;
            } else {
                *timer = 0;
            }
        }
        timer--;
    } while (remaining--);

    if (overlay7RuntimeGateReloc == NULL) {
        entry = gOverlay7ActiveHead;
        while (entry != NULL) {
            if (entry->active != 0) {
                entry->field04 += elapsed;
            }
            entry = entry->next;
        }

        if (overlay7RuntimeHandleReloc != NULL) {
            handleObject = overlay7RuntimeLastObjectReloc;
            overlay7ContinueRuntimeObjectReloc(
                overlay7RuntimeHandleReloc, handleObject->x, handleObject->y,
                handleObject->z);
        }

        if (elapsed < overlay7RuntimeTimerReloc) {
            overlay7RuntimeTimerReloc -= elapsed;
            remaining = count - 1;
        } else {
            if (overlay7RuntimeModeReloc == 1 && count != 0) {
                remaining = count - 1;
                cursor = objects + remaining;
                do {
                    object = *cursor;
                    state = object->state;
                    if (!(state->flags & 1) && state->kind < 6 &&
                        state->cooldown == 0) {
                        difference = overlay7MeasureRuntimeObjectAReloc(object) -
                                     overlay7MeasureRuntimeObjectBReloc(object);
                        if (difference - state->difference > 4.0f) {
                            overlay7RecordRuntimeEventReloc(state->kind + 0x15);
                        }
                        state->difference = difference;
                    }
                    cursor--;
                } while (remaining--);
            }

            remaining = count - 1;
            if (overlay7RuntimeModeReloc == 1 &&
                ((s32)overlay7RuntimePreviousLevelReloc -
                     (s32)overlay7RuntimeLevelReloc ==
                 1) &&
                overlay7RuntimeLevelReloc > 0 && count != 0) {
                cursor = objects + remaining;
                do {
                    object = *cursor;
                    state = object->state;
                    if (!(state->flags & 1) && state->level != 0 &&
                        state->scale + (f32)state->level > 9.0f) {
                        index = overlay7GetRuntimeIndexReloc(0, 4);
                        overlay7StartRuntimeValueReloc(
                            overlay7RuntimeValuesReloc[index + 0x924]);
                    }
                    cursor--;
                } while (remaining--);
                remaining = count - 1;
            }
            overlay7RuntimeTimerReloc = 0x12C;
        }

        if (count != 0) {
            f32 scale = overlay7RuntimeScaleReloc;

            cursor = objects + remaining;
            do {
                object = *cursor;
                state = object->state;
                if (state->cooldown != 0) {
                    state->cooldown--;
                }
                state->scale *= scale;
                cursor--;
            } while (remaining--);
        }

        if (overlay7RuntimeModeReloc == 0 && D_4 == 0) {
            found = 0;
            if (D_290 != NULL) {
                entry = D_290->nested;
                if (entry != NULL) {
                    entry->active = 1;
                    found = 1;
                }
                overlay7ReleaseRuntimeEntryReloc(D_290);
            }
            if (!found) {
                entry = gOverlay7ActiveHead;
                while (entry != NULL) {
                    if (entry->active == 1) {
                        found = 1;
                        break;
                    }
                    entry = entry->next;
                }
            }
            if (found) {
                value = entry->value;
                for (index = 0; index < 0x2B; index++) {
                    if (value == D_844[index].key) {
                        value += overlay7AdjustRuntimeValueReloc(
                            0, D_844[index].value);
                        break;
                    }
                }
                if ((u32)entry->field04 < 0xB4) {
                    overlay7CreateRuntimeOwnerReloc(
                        value, object->x, object->y, object->z, 4, NULL);
                    overlay7RuntimeLastObjectReloc = object;
                    D_290 = entry;
                } else {
                    if (entry->nested != NULL) {
                        overlay7ReleaseRuntimeHandleReloc(entry->nested);
                    }
                    overlay7ReleaseRuntimeEntryReloc(entry);
                }
            }
        }

        remaining = count - 1;
        if (count != 0) {
            cursor = objects + remaining;
            do {
                object = *cursor;
                state = object->state;
                if (state->level < state->previousLevel) {
                    if (state->level == 0) {
                        overlay7SetRuntimeModeReloc(object, 0x10);
                    } else if (!(state->flags & 1) ||
                               overlay7RuntimeChanceReloc(1, 0x64) < 0x24) {
                        overlay7SetRuntimeModeReloc(object, 0xE);
                    }
                } else if (state->previousLevel < state->level &&
                           (!(state->flags & 1) ||
                            overlay7RuntimeChanceReloc(1, 0x64) < 0x24)) {
                    overlay7SetRuntimeModeReloc(object, 0xF);
                }
                state->previousLevel = state->level;
                cursor--;
            } while (remaining--);
        }

        entry = gOverlay7ActiveHead;
        while (entry != NULL) {
            entry = entry->next;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/func_overlay_007_F0000324_185C1AC/func_overlay_007_F0000324_185C1AC.s")
#endif
