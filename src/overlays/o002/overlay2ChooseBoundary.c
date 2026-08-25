#include "PR/ultratypes.h"

typedef struct Overlay2Line {
    f32 x1;
    f32 y1;
    f32 x2;
    f32 y2;
    u16 value1;
    u16 value2;
} Overlay2Line;

typedef struct Overlay2Region {
    s32 boundaryAxis;
    f32 boundaryValue;
    u8 pad8[8];
    u16 start;
    u16 count;
} Overlay2Region;

typedef struct Overlay2BoundaryCandidate {
    s32 lineIndex;
    u16 side0Count;
    u16 side1Count;
    u16 crossingCount;
    u16 axis;
    f32 value;
} Overlay2BoundaryCandidate;

extern Overlay2Line *gOverlay2Lines;
extern s32 gOverlay2LineCount;
extern s32 gOverlay2BoundaryAxis;
extern f32 gOverlay2BoundaryValue;
extern s32 gOverlay2TemporaryMode;
extern Overlay2Region gOverlay2TemporaryRange;
extern Overlay2BoundaryCandidate *gOverlay2BoundaryCandidates;
extern s32 gOverlay2BoundaryCandidateCount[];
extern s32 gOverlay2SelectedBoundary;

extern s32 func_overlay_002_F0000000_1856DF8(Overlay2Region *range);
extern s32 overlay2ClassifyBoundary(f32 x1, f32 y1, f32 x2, f32 y2,
                                    s32 *side1, s32 *side2);
extern void overlay2ClipLines(Overlay2Region *input, Overlay2Region *output,
                              s32 wantedSide);

#define CANDIDATE_COUNT gOverlay2BoundaryCandidateCount[0]

#ifdef NON_MATCHING
void overlay2ChooseBoundary(Overlay2Region *region) {
    Overlay2Line *candidateLine;
    Overlay2Line *line;
    Overlay2BoundaryCandidate *candidate;
    s32 lineRemaining;
    s32 totalLines;
    s32 scanRemaining;
    s32 endpoint;
    s32 axis;
    s32 side1;
    s32 side2;
    s32 side0Count;
    s32 side1Count;
    s32 crossingCount;
    s32 rangeResults;
    s32 score;
    s32 bestScore;
    s32 savedLineCount;

    totalLines = region->count;
    scanRemaining = totalLines;
    candidateLine = &gOverlay2Lines[region->start];
    bestScore = 0x7FFFFFFF;
    CANDIDATE_COUNT = 0;
    scanRemaining--;
    if (totalLines != 0) {
        do {
            axis = 1;
            do {
                endpoint = 1;
                do {
                    side0Count = 0;
                    side1Count = 0;
                    crossingCount = 0;
                    rangeResults = 0;
                    gOverlay2BoundaryAxis = axis;
                    if (axis == 0) {
                        if (endpoint != 0) {
                            gOverlay2BoundaryValue = candidateLine->y1;
                        } else {
                            gOverlay2BoundaryValue = candidateLine->y2;
                        }
                    } else if (endpoint != 0) {
                        gOverlay2BoundaryValue = candidateLine->x1;
                    } else {
                        gOverlay2BoundaryValue = candidateLine->x2;
                    }

                    lineRemaining = region->count;
                    line = &gOverlay2Lines[region->start];
                    lineRemaining--;
                    if (region->count != 0) {
                        do {
                            overlay2ClassifyBoundary(line->x1, line->y1,
                                                     line->x2, line->y2,
                                                     &side1, &side2);
                            line++;
                            if (side1 != side2) {
                                crossingCount++;
                            } else if (side1 != 0) {
                                side1Count++;
                            } else {
                                side0Count++;
                            }
                        } while (lineRemaining--);
                    }

                    score = side1Count - side0Count;
                    if (score < 0) {
                        score = -score;
                    }
                    score += crossingCount >> 1;

                    savedLineCount = gOverlay2LineCount;
                    gOverlay2TemporaryMode = 0;
                    overlay2ClipLines(region, &gOverlay2TemporaryRange, 1);
                    if (func_overlay_002_F0000000_1856DF8(
                            &gOverlay2TemporaryRange) != 0) {
                        rangeResults = 1;
                    }
                    overlay2ClipLines(region, &gOverlay2TemporaryRange, 0);
                    if (func_overlay_002_F0000000_1856DF8(
                            &gOverlay2TemporaryRange) != 0) {
                        rangeResults++;
                    }
                    gOverlay2LineCount = savedLineCount;
                    gOverlay2TemporaryMode = 1;

                    if ((crossingCount == 0) &&
                        ((side1Count == 0) || (side0Count == 0))) {
                        score += 1000;
                    }
                    if (rangeResults == 2) {
                        score = -100;
                    } else if (rangeResults == 1) {
                        score -= 5;
                    }
                    if (score < bestScore) {
                        CANDIDATE_COUNT = 0;
                        bestScore = score;
                    }
                    if (score <= bestScore) {
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].lineIndex =
                            candidateLine - gOverlay2Lines;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].side0Count =
                            side0Count;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].side1Count =
                            side1Count;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT]
                            .crossingCount = crossingCount;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].axis =
                            gOverlay2BoundaryAxis;
                        gOverlay2BoundaryCandidates[CANDIDATE_COUNT].value =
                            gOverlay2BoundaryValue;
                        if (CANDIDATE_COUNT < 0x20) {
                            CANDIDATE_COUNT++;
                        }
                    }
                } while (endpoint--);
            } while (axis--);
            candidateLine++;
        } while (scanRemaining--);
    }

    scanRemaining = CANDIDATE_COUNT;
    scanRemaining--;
    if (CANDIDATE_COUNT != 0) {
        candidate = &gOverlay2BoundaryCandidates[scanRemaining];
        do {
            if (candidate->crossingCount < 0x0FFFFFFF) {
                gOverlay2SelectedBoundary = scanRemaining;
            }
            candidate--;
        } while (scanRemaining--);
    }

    gOverlay2BoundaryAxis =
        gOverlay2BoundaryCandidates[gOverlay2SelectedBoundary].axis;
    region->boundaryAxis = gOverlay2BoundaryAxis;
    gOverlay2BoundaryValue =
        gOverlay2BoundaryCandidates[gOverlay2SelectedBoundary].value;
    region->boundaryValue = gOverlay2BoundaryValue;
    gOverlay2SelectedBoundary =
        gOverlay2BoundaryCandidates[gOverlay2SelectedBoundary].lineIndex;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/overlay2ChooseBoundary/func_overlay_002_F00006E0_18574D8.s")
#endif
