#include "PR/ultratypes.h"

typedef struct Overlay98Vertex {
    u8 reserved0[2];
    s16 y;
    u8 reserved4[6];
} Overlay98Vertex;

typedef struct Overlay98PointRef {
    u8 reserved0;
    u8 vertexIndex;
    u8 reserved2[14];
} Overlay98PointRef;

typedef struct Overlay98Span {
    u8 reserved0[6];
    s16 vertexBase;
    s16 pointRefIndex;
    u8 reservedA[2];
    u32 flags;
} Overlay98Span;

typedef struct Overlay98Block {
    Overlay98Vertex *vertices;
    Overlay98PointRef *pointRefs;
    u8 reserved08[4];
    Overlay98Span *spans;
    u8 reserved10[0x14];
    s16 spanCount;
    u8 reserved26[0x1A];
} Overlay98Block;

typedef struct Overlay98Group {
    u8 reserved00[4];
    Overlay98Block *blocks;
    u8 reserved08[0x12];
    s16 blockCount;
} Overlay98Group;

extern s32 overlay98UniqueCountReloc;
extern s16 overlay98UniqueYReloc[15];

/* Exact DKR v77/v80 and JFG scans are negative for this routine. */
void overlay98CollectUniqueY(Overlay98Group *group) {
    Overlay98Block *block;
    Overlay98Span *span;
    s16 *unique;
    s16 *uniqueEnd;
    s16 value;
    s32 blockIndex;
    s32 spanIndex;
    s32 oldUniqueCount;
    s32 nextUniqueCount;
    s32 isNew;
    s32 pointRefIndex;
    register s32 vertexBase;

    overlay98UniqueCountReloc = 0;
    for (blockIndex = 0; blockIndex < group->blockCount; blockIndex++) {
        block = &group->blocks[blockIndex];
        for (spanIndex = 0; spanIndex < block->spanCount; spanIndex++) {
            span = &block->spans[spanIndex];
            if (span->flags & 0x8000) {
                pointRefIndex = span->pointRefIndex;
                vertexBase = span->vertexBase;
                value = *(s16 *)((u8 *)block->vertices +
                                  ((vertexBase +
                                    block->pointRefs[pointRefIndex]
                                        .vertexIndex) *
                                       10) +
                                   2);

                oldUniqueCount = overlay98UniqueCountReloc;
                nextUniqueCount = oldUniqueCount + 1;
                unique = overlay98UniqueYReloc;
                uniqueEnd = &overlay98UniqueYReloc[oldUniqueCount];
                isNew = 1;
                if (oldUniqueCount > 0) {
                    do {
                        if (*unique == value) {
                            isNew = 0;
                        }
                        unique++;
                    } while (unique < uniqueEnd);
                }

                if (isNew) {
                    overlay98UniqueYReloc[oldUniqueCount] = value;
                    overlay98UniqueCountReloc = nextUniqueCount;
                    if (nextUniqueCount >= 15) {
                        spanIndex = block->spanCount;
                        blockIndex = group->blockCount;
                    }
                }
            }
        }
    }
}
