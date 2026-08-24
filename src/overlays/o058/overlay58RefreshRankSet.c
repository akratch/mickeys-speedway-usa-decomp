#include "PR/ultratypes.h"
#include "overlays/overlay058.h"

typedef struct Overlay58RankEntry {
    s32 value;
    u8 reserved4;
    u8 reserved5;
    u8 reserved6;
    u8 tag;
} Overlay58RankEntry;

typedef struct Overlay58RankSet {
    Overlay58RankEntry entries[3];
    s32 floorValue;
    u8 reserved1C;
    u8 reserved1D;
    u8 reserved1E;
    u8 floorTag;
} Overlay58RankSet;

typedef struct Overlay58PrioritySource {
    u8 reserved0[4];
    u8 tag;
    u8 reserved5[3];
    s32 insertValue;
    s32 floorCandidates[3];
} Overlay58PrioritySource;

extern s16 gOverlay58RankSetIdReloc;
extern s32 gOverlay58FloorChoice;
extern s32 gOverlay58InsertChoice;

extern Overlay58RankSet *overlay58RankSetsReloc(void);
extern s32 overlay58RankSetIndexReloc(s32 id);
extern Overlay58PrioritySource *overlay58PrioritySourceReloc(void);

/* Pinned DKR v77/v80 and JFG scans found no matching donor. */
void overlay58RefreshRankSet(void) {
    Overlay58RankSet *set;
    Overlay58RankSet *sets;
    Overlay58PrioritySource *source;
    s32 setIndex;
    s32 i;
    s32 j;
    s32 candidateFloor;
    s32 existingFloor;
    u8 *sourceMeta;
    s32 *floorCursor;

    sets = overlay58RankSetsReloc();
    setIndex = overlay58RankSetIndexReloc(gOverlay58RankSetIdReloc);
    set = setIndex + sets;
    source = overlay58PrioritySourceReloc();
    sourceMeta = &source->tag;
    gOverlay58FloorChoice = -1;
    floorCursor = (s32 *)sourceMeta;

    for (i = 0; i < 3; i++) {
        candidateFloor = floorCursor[2];
        existingFloor = set->floorValue;
        if (((candidateFloor / 3) < (existingFloor / 3)) ||
            (existingFloor == 0)) {
            set->floorValue = candidateFloor;
            set->floorTag = *sourceMeta;
            gOverlay58FloorChoice = i;
        }
        floorCursor++;
    }

    gOverlay58InsertChoice = -1;
    for (i = 0; i < 3; i++) {
        if (((*(s32 *)(sourceMeta + 4) / 3) <
             (set->entries[i].value / 3)) ||
            (set->entries[i].value == 0)) {
            gOverlay58InsertChoice = i;

            for (j = 2; j > i; j--) {
                set->entries[j].reserved4 = set->entries[j - 1].reserved4;
                set->entries[j].reserved5 = set->entries[j - 1].reserved5;
                set->entries[j].reserved6 = set->entries[j - 1].reserved6;
                set->entries[j].value = set->entries[j - 1].value;
                set->entries[j].tag = set->entries[j - 1].tag;
            }

            set->entries[i].value = *(s32 *)(sourceMeta + 4);
            set->entries[i].tag = *sourceMeta;

            /* Keep the ordinary increment/backedge while forcing loop exit. */
            i = 3;
        }
    }
}
