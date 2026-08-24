#include "overlay31BuildLookupTables.h"

#ifndef OVERLAY31_MAX_LEVEL
#define OVERLAY31_MAX_LEVEL 9
#endif

#define INIT_SHORTS(record) \
    (record)->b4 = 0;         \
    (record)->b6 = 0;         \
    (record)->b8 = 0x200;     \
    (record)->bA = 0;         \
    (record)->bC = 0x200;     \
    (record)->bE = 0x200;     \
    (record)->d14 = 0;        \
    (record)->d16 = 0;        \
    (record)->d18 = 0x200;    \
    (record)->d1A = 0x200;    \
    (record)->d1C = 0;        \
    (record)->d1E = 0x200

#ifdef NON_MATCHING
void func_overlay_031_F0000000_187F520(void) {
    Overlay31IndexRecord *first;
    Overlay31IndexRecord *second;
    Overlay31FloatPair *pairs;
    s32 level;
    s32 index;
    s32 sum;

    first = overlay31AllocateReloc(0x880, 0x8C);
    gOverlay31IndexRows[0][0] = first;
    second = first + 34;
    level = 2;
    do {
        s32 limit;

        limit = level - 1;
        gOverlay31IndexRows[level - 2][0] = first;
        gOverlay31IndexRows[level - 2][1] = second;
        index = 0;
        if (limit > 0) {
            s32 next;
            s32 firstNeighbor;
            s32 secondNeighbor;

            firstNeighbor = level;
            secondNeighbor = level + 1;
            do {
                first->a1 = index;
                next = index + 1;
                first->a3 = secondNeighbor;
                first->c11 = index;
                first->c12 = secondNeighbor;
                first->c13 = firstNeighbor;
                first->a0 = 0x40;
                first->a2 = next;
                INIT_SHORTS(first);
                first->c10 = 0x40;

                second->a1 = firstNeighbor;
                second->a2 = secondNeighbor;
                second->c11 = firstNeighbor;
                second->c13 = index;
                index = next;
                firstNeighbor++;
                secondNeighbor++;
                first++;
                second->a0 = 0x40;
                second->a3 = next;
                INIT_SHORTS(second);
                second->c10 = 0x40;
                second->c12 = next;
                second++;
            } while (next < limit);
        }
        sum = index + level;
        if (level >= 3) {
            first->a0 = 0x40;
            first->a1 = index;
            first->a2 = 0;
            first->a3 = level;
            INIT_SHORTS(first);
            first->c10 = 0x40;
            first->c11 = index;
            first->c12 = level;
            first->c13 = sum;
            first++;

            second->a0 = 0x40;
            second->a1 = sum;
            second->a2 = level;
            second->a3 = 0;
            INIT_SHORTS(second);
            second->c10 = 0x40;
            second->c11 = sum;
            second->c12 = 0;
            second->c13 = index;
            second++;
        }
        level++;
    } while (level < OVERLAY31_MAX_LEVEL);

    pairs = overlay31AllocateReloc(0x118, 0x8C);
    {
        s32 level2;

        level2 = 2;
        do {
            s16 angle;
            s32 count;
            s16 step;

            gOverlay31FloatRows[level2 - 2] = pairs;
            angle = 0;
            count = 0;
            if (level2 > 0) {
                step = (s16)(0xFFFF / level2);
                do {
                f32 firstValue;
                f32 secondValue;
                f32 doubledFirst;
                f32 doubledSecond;

                firstValue = func_8002A8BC(angle);
                secondValue = func_8002A8C0(angle);
                doubledFirst = firstValue + firstValue;
                doubledSecond = secondValue + secondValue;
                angle += step;
                count++;
                pairs->first = doubledFirst + doubledSecond;
                pairs->second = doubledFirst - doubledSecond;
                    pairs++;
                } while (count != level2);
            }
            level2++;
        } while (level2 != OVERLAY31_MAX_LEVEL);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o031/overlay31BuildLookupTables/func_overlay_031_F0000000_187F520.s")
#endif
