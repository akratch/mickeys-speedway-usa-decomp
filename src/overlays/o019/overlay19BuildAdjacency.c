#include "overlays/overlay019.h"

/* Plateau (batch 17): exact 0x1EC size; 41 words differ, first at +0x7C.
 * Edge-loop locals, statement order, register hints, and signedness tie or regress.
 * The blocker is the temporary-register web; flags tie and permuter setup failed. */
#ifdef NON_MATCHING
void overlay19BuildAdjacency(
    O19Context *context,
    O19Group *group,
    O19Output *output) {
    struct A30Frame {
        s32 outputOffset;
        s32 spanOffset;
        s32 suppressed;
        u8 pad0C[0x18];
        s32 itemEnd;
        u8 pad28[0x0C];
        s32 spanIndex;
    } frame;
    s32 itemIndex;
    s32 edgeIndex;
    s32 nextEdgeIndex;
    s32 edgeOffset;
    s32 adjacentItem;
    s32 edgeLimit;
    u16 invalid;
    s16 vertexBase;
    s16 spanCount;
    s16 itemStart;
    O19Span *span;

    frame.spanIndex = 0;
    spanCount = group->spanCount;
    if (spanCount > 0) {
        frame.spanOffset = 0;
        edgeLimit = 6;
        invalid = 0xFFFF;
        do {
            span = (O19Span *)((u8 *)group->spans + frame.spanOffset);
            frame.itemEnd = (span + 1)->itemStart +
                (itemStart = span->itemStart,
                 vertexBase = span->vertexBase,
                 0);
            itemIndex = itemStart;
            adjacentItem = span->flags;
            if (itemStart < frame.itemEnd) {
                adjacentItem &= 0x1080;
                frame.suppressed = (s16)adjacentItem & 0x7FFF; frame.outputOffset = itemIndex << 3;
                do {
                    if (frame.suppressed != 0) {
                        *(u16 *)((u8 *)output->records + frame.outputOffset) = invalid;
                        edgeIndex = 0;
                        do {
                            O19AdjacencyRecord *invalidRecords;

                            invalidRecords = output->records;
                            *(u16 *)&invalidRecords[itemIndex]
                                .edgeNeighbor[edgeIndex] = invalid;
                            edgeIndex++;
                        } while (edgeIndex < 3);
                    } else {
                        *(s16 *)((u8 *)output->records + frame.outputOffset) =
                            (s16)itemIndex;
                        edgeIndex = 0;
                        do {
                            nextEdgeIndex = edgeIndex + 1;
                            if (nextEdgeIndex >= 3) {
                                nextEdgeIndex = 0;
                            }
                            adjacentItem = overlay19FindAdjacent(
                                context,
                                group,
                                itemIndex,
                                group->points[itemIndex].selectors[edgeIndex] +
                                    vertexBase,
                                group->points[itemIndex].selectors[nextEdgeIndex] +
                                    vertexBase);
                            if (adjacentItem == -1) {
                                *(u16 *)&output->records[itemIndex]
                                    .edgeNeighbor[edgeIndex] = 0xFFFE;
                            } else {
                                output->records[itemIndex]
                                    .edgeNeighbor[edgeIndex] =
                                    (s16)adjacentItem;
                            }
                            edgeIndex++;
                        } while (edgeIndex != 3);
                    }
                    itemIndex++;
                    frame.outputOffset += 8;
                } while (itemIndex < frame.itemEnd);
            }
            frame.spanOffset += sizeof(O19Span);
            frame.spanIndex++;
            spanCount = group->spanCount;
        } while (frame.spanIndex < spanCount);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o019/overlay19BuildAdjacency/func_overlay_019_F0000A30_1875C88.s")
#endif
