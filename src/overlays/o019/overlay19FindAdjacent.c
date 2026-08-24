#include "overlays/overlay019.h"

/*
 * Pinned DKR v77/v80 and JFG donor scans classify overlay 19 as none.
 * IDO appends one alignment word after this 0x15C-byte function; the build
 * rule trims that non-owned word because the following D78 body starts there.
 */
s32 overlay19FindAdjacent(
    O19Context *context,
    O19Group *group,
    s32 selfItem,
    s32 queryStartIndex,
    s32 queryEndIndex) {
    s32 spanIndex;
    u8 unusedStackShape[0x24];
    s32 spanOffset;
    s32 candidateStart;
    s32 candidateItem;
    s32 itemEnd;
    s32 edgeIndex;
    s32 nextEdgeIndex;
    s32 classification;
    s32 candidateStartIndex;
    s32 candidateEndIndex;
    s16 vertexBase;
    O19Span *span;

    spanIndex = 0;
    if (group->spanCount > 0) {
        spanOffset = 0;
        do {
            span = (O19Span *)((u8 *)group->spans + spanOffset);
            candidateStart = span->itemStart;
            vertexBase = span->vertexBase;
            itemEnd = (span + 1)->itemStart;
            if ((span->flags & 0x1080) != 0) candidateStart = itemEnd;
            if (candidateStart < itemEnd) {
                candidateItem = candidateStart;
                do {
                    if (candidateItem != selfItem) {
                        edgeIndex = 0;
                        do {
                            nextEdgeIndex = edgeIndex + 1;
                            if (nextEdgeIndex >= 3) nextEdgeIndex = 0;
                            candidateStartIndex = group->points[candidateItem].selectors[edgeIndex] + vertexBase;
                            candidateEndIndex = group->points[candidateItem].selectors[nextEdgeIndex] + vertexBase;
                            classification = overlay19ClassifyEdge(
                                context->vertices, queryStartIndex, queryEndIndex,
                                candidateStartIndex, candidateEndIndex);
                            if (classification != 0) return candidateItem;
                            edgeIndex++;
                        } while (edgeIndex < 3);
                    }
                    candidateItem++;
                } while (candidateItem != itemEnd);
            }
            spanOffset += (spanIndex++, sizeof(O19Span));
        } while (spanIndex < group->spanCount);
    }
    return -1;
}
