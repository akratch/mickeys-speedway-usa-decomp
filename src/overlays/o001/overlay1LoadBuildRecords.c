#include "PR/ultratypes.h"

/* Canonical typed owner of Overlay 1 text +0x10C8..+0x19B8. */

typedef struct Overlay1PackedRecord {
    s16 type;
    u8 size;
    u8 pad03;
    s16 value4;
    u8 pad06[2];
    s16 value8;
    u8 group;
    u8 slot;
    u8 link;
} Overlay1PackedRecord;

typedef struct Overlay1Point {
    s16 first;
    s16 second;
} Overlay1Point;

typedef struct Overlay1Group {
    Overlay1Point *points;
    s32 count;
    struct Overlay1Group *previous;
    struct Overlay1Group *next;
    s32 selector;
    s32 field14;
    s32 field18;
} Overlay1Group;

typedef struct Overlay1Metric {
    f32 x;
    f32 y;
    f32 score;
    s8 rank;
    u8 pad0D[3];
} Overlay1Metric;

typedef struct Overlay1LargeRecord {
    u8 pad00[0x14];
    Overlay1Metric metrics[8];
} Overlay1LargeRecord;

extern void overlay1LoadPackedRecordsReloc(
    Overlay1PackedRecord **records, s32 *size, s32 resource);
extern s32 D_1D7C;
extern s32 D_1D80;
extern s32 D_1D8C;
extern s32 D_0;
extern s32 D_1D90;
extern s32 D_1DBC;
extern s32 D_1D98;
extern s32 D_1D98Read;
extern u8 gOverlay1RankBase;
extern u8 gOverlay1RankLimit;
extern s32 gOverlay1RankDelta;
extern s32 gOverlay1ModeConstant;
extern u8 gOverlay1ConfigMode;
extern f32 D_BC;
extern f32 D_C0;
extern f32 D_C4;
extern f32 D_C8;
extern f32 D_CC;
extern f32 D_D0;
extern f32 D_D4;
extern f32 D_1DAC;
extern f32 D_1DB0;
extern f32 D_1DB4;
extern f32 D_1DB8;
extern u8 D_0_Clear[];
extern Overlay1Group *D_1BA0;
extern Overlay1Point *D_1D70;
extern void *D_1BA4;
extern void overlay1ReleaseBuildMemoryReloc(void *memory);
extern void *overlay1AllocateBuildMemoryReloc(s32 size, s32 tag);
extern void overlay1RejectBuildCycleReloc(Overlay1Group *group);
extern void overlay1FinalizeBuildGroupReloc(Overlay1Group *group);
extern void *overlay1SubmitBuildReloc(s32 value);
extern Overlay1LargeRecord *D_1D58;
extern Overlay1LargeRecord *D_1D58Read;
extern Overlay1LargeRecord *D_1D5C;
extern u8 *gOverlay1MissingLargeContext;
extern f32 D_1DA8;
extern u8 D_1DCC[];
extern void overlay1ClearLargeRecordsReloc(void *records, s32 size);
extern void overlay1DecodeLargeRecordReloc(
    Overlay1LargeRecord *record, Overlay1PackedRecord *packed, s32 mode);
extern void overlay1ReportMissingLargeReloc(s32 value, s32 type, s32 severity);
extern Overlay1LargeRecord *overlay1GetMetricSourceAReloc(
    Overlay1LargeRecord *record);
extern Overlay1LargeRecord *overlay1GetMetricSourceBReloc(
    Overlay1LargeRecord *record);
extern Overlay1LargeRecord *overlay1GetMetricSourceCReloc(
    Overlay1LargeRecord *record);
extern f32 func_overlay_001_F0000F84_184D364(
    f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3,
    s32 scale);

#ifdef NON_MATCHING
void overlay1LoadBuildRecords(void) {
    Overlay1PackedRecord *records;
    Overlay1PackedRecord *record;
    s32 size;
    s32 offset;
    s32 clearIndex;
    s32 value;
    s32 index;
    s32 loopValue;
    s32 pointIndex;
    Overlay1Group *group;
    Overlay1Group *link;
    Overlay1Point *point;

    overlay1LoadPackedRecordsReloc(&records, &size, 1);
    D_1D7C = 0;
    D_1D80 = 0;
    D_1D8C = 0;
    D_0 = 0;
    D_1D90 = 0;
    D_1DBC = 0;
    D_1D98 = 0;
    gOverlay1RankDelta = gOverlay1RankBase - gOverlay1RankLimit;
    gOverlay1ModeConstant = 3;

    if (gOverlay1ConfigMode != 0) {
        if (gOverlay1ConfigMode != 1) {
            if (gOverlay1ConfigMode == 2) {
                D_1DAC = 1.0f;
                D_1DB0 = 1.0f;
                D_1DB4 = 1.25f;
                D_1DB8 = 0.25f;
            }
        } else {
            D_1DAC = D_BC;
            D_1DB0 = D_C0;
            D_1DB4 = D_C4;
            D_1DB8 = 0.5f;
        }
    } else {
        D_1DAC = D_C8;
        D_1DB0 = D_CC;
        D_1DB4 = D_D0;
        D_1DB8 = 0.75f;
    }

    clearIndex = 5;
    do {
        D_0_Clear[clearIndex] = 0xFF;
    } while (clearIndex-- != 0);

    offset = 0;
    record = records;
    if (size > 0) {
        do {
            if (record->type == 0xC8) {
                D_1D7C++;
                value = record->group + 1;
                if (D_1D80 < value) {
                    D_1D80 = value;
                }
            }
            offset += record->size;
            record = (Overlay1PackedRecord *)((u8 *)record + record->size);
        } while (offset < size);
    }

    if (D_1D80 != 0) {
        if (D_1BA0 != NULL) {
            overlay1ReleaseBuildMemoryReloc(D_1BA0);
        }
        if (D_1D70 != NULL) {
            overlay1ReleaseBuildMemoryReloc(D_1D70);
        }
        D_1BA0 = overlay1AllocateBuildMemoryReloc(D_1D80 * 0x1C, 0x85);
        D_1D70 = overlay1AllocateBuildMemoryReloc(D_1D7C * 4, 0x85);

        index = D_1D80 - 1;
        if (D_1D80 != 0) {
            group = &D_1BA0[index];
            do {
                group->points = NULL;
                group->count = 0;
                group->previous = NULL;
                group->next = NULL;
                group->selector = 0;
                group->field14 = 0;
                group->field18 = 0;
                group--;
            } while (index-- != 0);
        }

        offset = 0;
        record = records;
        if (size > 0) {
            do {
                if (record->type == 0xC8) {
                    D_1BA0[record->group].count++;
                }
                offset += record->size;
                record = (Overlay1PackedRecord *)((u8 *)record + record->size);
            } while (offset < size);
        }

        pointIndex = 0;
        index = 0;
        if (D_1D80 > 0) {
            group = D_1BA0;
            do {
                group->points = &D_1D70[pointIndex];
                pointIndex += group->count;
                group++;
                index++;
            } while (index < D_1D80);
        }

        offset = 0;
        record = records;
        if (size > 0) {
            do {
                if (record->type == 0xC8) {
                    point = &D_1BA0[record->group].points[record->slot];
                    point->first = record->value4;
                    point->second = record->value8;
                }
                offset += record->size;
                record = (Overlay1PackedRecord *)((u8 *)record + record->size);
            } while (offset < size);
        }

        offset = 0;
        record = records;
        if (size > 0) {
            do {
                if (record->type == 0xC9) {
                    group = &D_1BA0[record->group];
                    group->selector = record->slot;
                    if (record->link != 0) {
                        link = &D_1BA0[record->link];
                        while (link->next != NULL) {
                            link = link->next;
                        }
                        if (link == group) {
                            overlay1RejectBuildCycleReloc(link);
                        } else {
                            link->next = group;
                            group->previous = link;
                        }
                    }
                }
                offset += record->size;
                record = (Overlay1PackedRecord *)((u8 *)record + record->size);
            } while (offset < size);
        }

        index = D_1D80 - 2;
        group = &D_1BA0[1];
        if ((D_1D80 - 1) > 0) {
            do {
                if (group->previous == NULL) {
                    overlay1FinalizeBuildGroupReloc(group);
                }
                group++;
            } while (index-- > 0);
        }
        D_1BA4 = overlay1SubmitBuildReloc(1);
    } else {
        offset = 0;
        record = records;
        if (size > 0) {
            do {
                if (record->type == 0xCA) {
                    value = record->link + 1;
                    if (D_1D8C < value) {
                        D_1D8C = value;
                    }
                }
                offset += record->size;
                record = (Overlay1PackedRecord *)((u8 *)record + record->size);
            } while (offset < size);
        }

        value = D_1D8C * 0x94;
        if (value != 0) {
            D_1D58 = overlay1AllocateBuildMemoryReloc(value, 0x85);
            overlay1ClearLargeRecordsReloc(D_1D58, value);
            offset = 0;
            record = records;
            if (size > 0) {
                do {
                    if (record->type == 0xCA) {
                        overlay1DecodeLargeRecordReloc(
                            &D_1D58[record->link], record, 0);
                    }
                    offset += record->size;
                    record = (Overlay1PackedRecord *)((u8 *)record + record->size);
                } while (offset < size);
            }
        } else {
            D_1D58 = NULL;
        }

        if (D_1D58 == NULL) {
            gOverlay1ModeConstant = 0;
            overlay1ReportMissingLargeReloc(
                (s32)(gOverlay1MissingLargeContext + D_1D98Read), 2, 2);
            return;
        }
        {
            f32 total;
            f32 maximum;
            f32 minimum;
            f32 score;
            f32 scale;
            Overlay1LargeRecord *sourceA;
            Overlay1LargeRecord *sourceB;
            Overlay1LargeRecord *sourceC;
            Overlay1LargeRecord *large;
            Overlay1Metric *metric;
            Overlay1Metric *metricA;
            Overlay1Metric *metricB;
            Overlay1Metric *metricC;

            gOverlay1ModeConstant = 1;
            D_1D5C = &D_1D58[D_1D8C - 1];

            offset = 0;
            record = records;
            if (size > 0) {
                do {
                    if (record->type == 0xCA) {
                        large = &D_1D58[record->link];
                        maximum = 0.0f;
                        minimum = D_D4;
                        sourceA = overlay1GetMetricSourceAReloc(large);
                        sourceB = overlay1GetMetricSourceBReloc(sourceA);
                        sourceC = overlay1GetMetricSourceCReloc(large);
                        index = 7;
                        metric = &large->metrics[7];
                        metricA = &sourceA->metrics[7];
                        metricB = &sourceB->metrics[7];
                        metricC = &sourceC->metrics[7];
                        do {
                            score = func_overlay_001_F0000F84_184D364(
                                metricC->x, metricC->y,
                                metric->x, metric->y,
                                metricA->x, metricA->y,
                                metricB->x, metricB->y, 0x10);
                            metric->score = score;
                            if (metric->rank != 0) {
                                if (maximum < score) {
                                    maximum = score;
                                }
                                if (score < minimum) {
                                    minimum = score;
                                }
                            }
                            loopValue = index;
                            metric--;
                            metricA--;
                            metricB--;
                            metricC--;
                            index--;
                        } while (loopValue != 0);
                        if (maximum != minimum) {
                            scale = 51.0f / (maximum - minimum);
                            index = 7;
                            metric = &large->metrics[7];
                            do {
                                if (metric->rank != 0) {
                                    metric->rank = (s8)(s32)(
                                        (f32)metric->rank +
                                        ((maximum - metric->score) * scale));
                                    if (index == 3) {
                                        metric->rank += 5;
                                    }
                                }
                                loopValue = index;
                                metric--;
                                index--;
                            } while (loopValue != 0);
                        }
                    }
                    value = record->size;
                    offset += value;
                    record = (Overlay1PackedRecord *)((u8 *)record + value);
                } while (offset < size);
            }

            total = 0.0f;
            index = D_1D8C - 1;
            if (D_1D8C != 0) {
                do {
                total += D_1D58Read->metrics[3].score;
                } while (index-- != 0);
            }
            D_1DA8 = total / (f32)D_1D8C;
            index = 4;
            do {
                D_1DCC[index] = 0;
            } while (index-- != 0);
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1LoadBuildRecords/func_overlay_001_F00010C8_184D4A8.s")
#endif
