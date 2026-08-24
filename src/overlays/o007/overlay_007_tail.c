#define OVERLAY_007_DEFINE_BSS
#include "overlays/overlay_007.h"

/* Overlay 7's local BSS through +0x2a0; the final 0x10 remains atlas-owned. */
static Overlay7Entry gOverlay7EntryStorage[32];
static s32 gOverlay7Reserved280;
static Overlay7Entry *gOverlay7ActiveHead;
static Overlay7Entry *gOverlay7FreeHead;
static Overlay7Entry *gOverlay7ActiveTail;
static Overlay7Entry *gOverlay7Selected;

/* Overlay 7, ADR 0006 consolidation: C after the middle assembly island. */

#ifdef NON_MATCHING
void overlay7DispatchModes(Overlay7ModeOwner *first, Overlay7ModeOwner *second) {
    Overlay7ModeState *firstState;
    Overlay7ModeState *secondState;
    Overlay7ModeRecord *record;
    Overlay7ModeRecord (*modes)[10];

    secondState = second->state;
    firstState = first->state;
    modes = secondState->alternate == 0 ? gOverlay7AlternateModes
                                        : gOverlay7PrimaryModes;
    record = &modes[firstState->index][secondState->index];

    if ((s32)(gOverlay7DispatchFlagsReloc << 22) < 0) {
        secondState->timer = 100;
        secondState->height += 5.0f;
        switch (record->mode) {
        case 1:
            overlay7CreateEntry(first, record->first, 3);
            break;
        case 2:
            overlay7CreateEntry(first, record->first, 3);
            overlay7AppendEntry(second, record->second, 3);
            break;
        case 3:
            overlay7CreateEntry(second, record->second, 3);
            overlay7AppendEntry(first, record->first, 3);
            break;
        case 4:
            if (overlay7LookupReloc(1, 2) == 1) {
                overlay7CreateEntry(first, record->first, 3);
                overlay7AppendEntry(second, record->second, 3);
            } else {
                overlay7CreateEntry(second, record->second, 3);
                overlay7AppendEntry(first, record->first, 3);
            }
            break;
        case 0:
        case 5:
            overlay7CreateEntry(first, record->first, 3);
            overlay7CreateEntry(second, record->second, 3);
            break;
        case 6:
            if (overlay7LookupReloc(1, 2) == 1) {
                overlay7CreateEntry(first, record->first, 3);
                break;
            }
            overlay7CreateEntry(second, record->second, 3);
            break;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay_007_tail/func_overlay_007_F0000894_185C71C.s")
#endif

#ifdef NON_MATCHING
void overlay7UpdateOwnerMode(Overlay7CheckOwner *owner, s32 previous) {
    Overlay7CheckState *state;
    Overlay7CheckEntry *base;
    Overlay7CheckEntry *entries;
    s32 reference;
    s32 index;
    s32 failed;
    s32 i;

    state = owner->state;
    if (*overlay7GetModeReloc() == 1) {
        failed = 0;
        base = (Overlay7CheckEntry *)overlay7GetCheckTableReloc();
        index = overlay7GetCheckIndexReloc(overlay7GetCurrentReloc());

        if (D_8 < state->field383 && !(state->flags1A8 & 1)) {
            if (D_8 == 2) {
                entries = base + index * 4;
                reference = entries[3].value;
                for (i = 0; i < 3; i++) {
                    if (state->limits404[i] < reference || reference == 0 ||
                        state->field400 < entries[i].value ||
                        entries[i].value == 0) {
                        failed = 1;
                    }
                }
                if (failed) {
                    overlay7RecordCheckReloc(5);
                } else {
                    overlay7RecordCheckReloc(D_8 + 2);
                }
            } else {
                overlay7RecordCheckReloc(D_8 + 2);
            }
            D_8++;
        }

        if (previous == state->field383) {
            if (failed) {
                overlay7SetOwnerModeReloc(owner, 6);
            } else {
                overlay7SetOwnerModeReloc(owner, 12);
            }
        } else {
            overlay7SetOwnerModeReloc(owner, state->field385);
        }
    } else {
        if (D_8 < state->field383 && !(state->flags1A8 & 1)) {
            overlay7RecordCheckReloc(D_8 + 2);
            D_8++;
        }
        if (previous == state->field383) {
            overlay7SetOwnerModeReloc(owner, state->field385 + 7);
        } else {
            overlay7SetOwnerModeReloc(owner, state->field385);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay_007_tail/func_overlay_007_F0000AA0_185C928.s")
#endif

#ifdef NON_MATCHING
void overlay7DispatchSelection(Overlay7DispatchOwner *owner, s32 selection) {
    Overlay7DispatchState *state;
    u16 *override;
    s8 mapped;

    state = owner->state;
    if ((s32)(gOverlay7DispatchFlagsReloc << 22) < 0) {
        if (selection >= 14 && selection < 17) {
            override = &gOverlay7DispatchOverride[state->index];
            if (*override == 0) {
                *override = 0x10E;
                goto create;
            }
        } else {
create:
            overlay7CreateEntry(owner,
                                gOverlay7DispatchValues[state->index][selection],
                                gOverlay7DispatchTypes[selection]);
            goto query;
        }
    } else {
query:
        if (overlay7QueryReloc() == 0) {
            mapped = gOverlay7DispatchMap[selection];
            if (mapped != -1) {
                overlay7ApplyReloc(0, state->index, mapped, state->field45D);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay_007_tail/func_overlay_007_F0000CCC_185CB54.s")
#endif

#ifdef NON_MATCHING
void overlay7CommitSelection(s32 selection) {
    Overlay7Pair *pair;
    s32 remaining;
    u16 value;

    if (gOverlay7DispatchModeReloc & 1) {
        switch (selection) {
        case 29:
            value = 0xCF;
            break;
        case 30:
            value = 0xF5;
            break;
        case 31:
            value = 0x116;
            break;
        default:
            value = *(u16 *)&gOverlay7DispatchData[
                0x754 + selection * 6 + overlay7LookupReloc(0, 2) * 2];
            break;
        }
        pair = (Overlay7Pair *)&gOverlay7DispatchData[0x8F4];
        remaining = 11;
        do {
            if (value == pair->key) {
                value = value + overlay7LookupReloc(0, pair->value);
                break;
            }
            pair++;
        } while (remaining--);
        if (value != 0) {
            if (gOverlay7DispatchObject != 0) {
                overlay7ObjectReloc(gOverlay7DispatchObject);
                func_overlay_007_F0000CCC_185CB54(gOverlay7Selected);
            }
            overlay7CommitReloc(value, &gOverlay7CommitArgument);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay_007_tail/func_overlay_007_F0000DBC_185CC44.s")
#endif

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
s32 overlay7FillValues(s16 *value) {
    s32 remaining;

    /* Preserves the original IDO register coloring without emitted code. */
    if (((!value) & 0xFFFFU) && (!value)) {
    }
    value = &gOverlay7ValuesEnd;
    remaining = 9;
    do {
        *((0, value)) = 0xF0;
        value--;
    } while (remaining--);
}

/* Naturally exact under the overlay's ordinary IDO -O2 -mips2 flags. */
void overlay7InitPool(void) {
    Overlay7Entry *entry;
    s32 i;

    entry = gOverlay7FreeHead = gOverlay7EntryStorage;
    for (i = 0; i < 31; i++) {
        entry->next = entry + 1;
        entry->active = 0;
        entry++;
    }
    entry->next = 0;
    gOverlay7ActiveHead = 0;
    gOverlay7ActiveTail = 0;
    gOverlay7Selected = 0;
}
