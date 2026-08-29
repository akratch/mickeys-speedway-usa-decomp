#define OVERLAY_007_DEFINE_BSS
#include "overlays/overlay_007.h"

/* Overlay 7's local BSS through +0x2a0; the final 0x10 remains atlas-owned. */
static Overlay7Entry gOverlay7EntryStorage[32];
static s32 gOverlay7Reserved280;
static Overlay7Entry *gOverlay7ActiveHead;
static Overlay7Entry *gOverlay7FreeHead;
static Overlay7Entry *gOverlay7ActiveTail;
static Overlay7Entry *gOverlay7Selected;

typedef struct Overlay7SelectionRow {
    u16 values[3];
} Overlay7SelectionRow;

/* Overlay 7, ADR 0006 consolidation: C after the middle assembly island. */

/* Bounded reproof 2026-08-29: the identity-correct masked spelling is 128/131
 * raw and 129/131 after runtime relocation normalization, with exact 0x20C
 * size, 0x20 frame, and first substantive mismatch +0x44. The two residual
 * sites at +0x44/+0x64 are one t4-versus-target-t3 flags carrier; the raw
 * +0xA4 switch-table LO16 addend normalizes away. Runtime metadata proves all
 * 23 text plus seven table offsets, types, and identities, including mathRnd
 * at +0x124/+0x1BC. Clean unmasked V0 regressed to 121/131. All 119 flag rows
 * were nonexact; a proc-0 trace found every uopt pool assignment exact and the
 * temp FIFO diverging only at slot 4. Two natural scalar/scope forms regressed
 * to 112/131 and shifted a relocation. ORT 1471 and all six callers are
 * authenticated. The TU's +0x934..+0x950 rodata ownership clears module
 * growth; the function owns +0x894..+0xAA0 with no padding. The fallback
 * remains canonical; retry only after a new natural temp-FIFO phase/reuse
 * spelling, not more flags, explicit carriers, or a generic batch. */
#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay7DispatchModes
 * score: 129/131 words
 * frame: 0x20
 * relocations: 30
 * first-mismatch: +0x44
 * summary: One flags-carrier temp uses t4 instead of target t3 at two sites; 119 flags and two trace-selected scalar/scope forms are exhausted.
 */
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

    if ((s32)((gOverlay7DispatchFlagsReloc & 0x3FF) << 22) < 0) {
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
            if (mathRnd(1, 2) == 1) {
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
            if (mathRnd(1, 2) == 1) {
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

/*
 * Plateau: exact size and opcode schedule, but 81 words still differ from
 * +0x10. The remaining mismatch is a temp-FIFO/register-allocation phase;
 * failed and base now have the target stack homes, and the O2 flag lattice
 * does not change the result.
 */
#ifdef NON_MATCHING
void overlay7UpdateOwnerMode(Overlay7CheckOwner *owner, s32 previous) {
    Overlay7CheckState *state;
    s32 failed;
    Overlay7CheckEntry *entries;
    s32 reference;
    s32 index;
    Overlay7CheckEntry *base;
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
                i = 0;
                do {
                    if (state->limits404[i] < reference || reference == 0 ||
                        state->field400 < entries[i].value ||
                        entries[i].value == 0) {
                        failed = 1;
                    }
                    i++;
                } while (i != 3);
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

/* Bounded reproof 2026-08-29: current configured full-TU C is 55/60 raw and
 * 58/60 after runtime relocation normalization, with exact 0xF0 size, 0x20
 * frame, and first substantive mismatch +0x4. The two residual sites at
 * +0x4/+0x10 are one t7-versus-t6 unnamed flags/selection-offset temp web.
 * Runtime metadata proves all 13 emitted offsets, types, and identities,
 * including camGetModeReloc at +0xA4 and overlay59AppendValueReloc at +0xD8.
 * All 119 flag rows were nonexact; the proc-2 allocator trace found the named
 * webs already clean, and two trace-selected scalar/scope forms regressed to
 * 39/60. The fallback remains canonical. A later attempt needs a new UGEN
 * temp-coalescing mechanism, not more flags, explicit locals, or a generic
 * permutation batch. */
#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay7DispatchSelection
 * score: 58/60 words
 * frame: 0x20
 * relocations: 13
 * first-mismatch: +0x4
 * summary: Unnamed flags-load temp does not coalesce with the later selection-offset temp; 119 flags and two trace-selected scalar/scope forms are exhausted.
 */
void overlay7DispatchSelection(Overlay7DispatchOwner *owner, s32 selection) {
    Overlay7DispatchState *state;
    u16 *override;
    s8 mapped;

    state = owner->state;
    if ((s32)((gOverlay7DispatchFlagsReloc & 0x3FF) << 22) < 0) {
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
        if (camGetModeReloc() == 0) {
            mapped = gOverlay7DispatchMap[selection];
            if (mapped != -1) {
                overlay59AppendValueReloc(0, state->index, mapped,
                                          state->field45D);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay_007_tail/func_overlay_007_F0000CCC_185CB54.s")
#endif

/* Bounded reproof 2026-08-29: current configured full-TU C is 67/72 raw and
 * 69/72 after runtime relocation normalization, with exact 0x120 size, 0x30
 * frame, and first substantive mismatch +0xBC. The three residual sites at
 * +0xBC/+0xC4/+0xD4 are one post-mathRnd u16 conversion carrier. Runtime
 * metadata proves all 17 emitted offsets, types, and identities, including
 * the local JUMP at +0xF4 and four authenticated external calls. ORT 1345 and
 * all 17 resident/cross-overlay callers are authenticated. All 119 flag rows
 * were nonexact; the proc-3 trace places the candidate conversion in uopt pool
 * slot 20 while the target uses temp-FIFO slot 9. Explicit assignment and a
 * scoped u16 carrier both regressed to 68/72 normalized. The fallback remains
 * canonical; retry only after a new source route for this pool/temp decision.
 */
#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay7CommitSelection
 * score: 69/72 words
 * frame: 0x30
 * relocations: 17
 * first-mismatch: +0xBC
 * summary: Unnamed post-mathRnd u16 conversion remains in the uopt pool instead of the target temp FIFO; flags and two natural forms are exhausted.
 */
void overlay7CommitSelection(s32 selection) {
    u16 value;
    Overlay7Pair *pair;
    s32 remaining;

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
            value = ((Overlay7SelectionRow *)&gOverlay7DispatchData[0x754])
                        [selection]
                            .values[mathRnd(0, 2)];
            break;
        }
        pair = (Overlay7Pair *)&gOverlay7DispatchData[0x8F4];
        remaining = 11;
        do {
            if (pair->key == value) {
                value += mathRnd(0, pair->value);
                break;
            }
            pair++;
        } while (remaining--);
        if (value != 0) {
            if (gOverlay7DispatchObject != 0) {
                func_800031E8(gOverlay7DispatchObject);
                overlay7ReleaseEntry(gOverlay7Selected);
            }
            amSndPlay(value, &gOverlay7CommitArgument);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o007/overlay_007_tail/func_overlay_007_F0000DBC_185CC44.s")
#endif

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. Exact but
 * non-natural: the empty condition and dummy comma-expression operand are
 * semantically inert allocation aids preserving IDO's 11-word coloring.
 * Tracked in docs/cleanup-queue.md. */
s32 overlay7FillValues(s16 *value) {
    s32 remaining;

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
