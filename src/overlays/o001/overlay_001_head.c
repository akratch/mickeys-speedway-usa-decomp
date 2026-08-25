#include "overlays/overlay_001.h"

/* ---- overlay1InitMotionScale ---- */

typedef struct O1Point2 { f32 x; f32 y; } O1Point2;
typedef struct O1Reference { u8 pad00[0xC]; f32 x; u8 pad10[4]; f32 y; } O1Reference;
typedef struct O1MotionWorld { u8 pad00[0x37C]; s16 angle; u8 pad37E[0x1A]; f32 scale; f32 heading; } O1MotionWorld;
extern O1Point2 *D_20C;
extern O1Point2 *D_210;
extern f32 overlay1SquareRoot(f32 value);
extern s32 overlay1AngleFromIndex(s16 value);
void overlay1InitMotionScale(void) {
    f32 dx;
    f32 dy;
    f32 firstDistance;
    f32 secondDistance;
    dx = D_20C->x - D_210->x;
    dy = D_20C->y - D_210->y;
    firstDistance = overlay1SquareRoot((dx * dx) + (dy * dy));
    dx = D_20C->x - ((O1Reference *)D_1D9C)->x;
    dy = D_20C->y - ((O1Reference *)D_1D9C)->y;
    secondDistance = overlay1SquareRoot((dx * dx) + (dy * dy));
    ((O1MotionWorld *)D_1DA0)->scale = secondDistance / firstDistance;
    ((O1MotionWorld *)D_1DA0)->heading =
        (f32)overlay1AngleFromIndex(((O1MotionWorld *)D_1DA0)->angle) +
        ((O1MotionWorld *)D_1DA0)->scale;
}

/* ---- overlay1InterpolatePath ---- */


typedef struct O1ControlPoint { f32 x; f32 z; u8 pad08[8]; } O1ControlPoint;
typedef struct O1ControlTable { u8 pad00[0x14]; O1ControlPoint points[1]; } O1ControlTable;
typedef struct O1PathOffsetOwner { u8 pad00[0x398]; f32 pathOffset; } O1PathOffsetOwner;

extern O1ControlTable *D_1D60;
extern O1ControlTable *D_1D68;
extern O1ControlTable *D_1D6C;
extern O1ControlTable *overlay1NextControlTable(O1ControlTable *table);
extern f32 overlay1CubicInterpolate(f32 a, f32 b, f32 c, f32 d, f32 t);

#ifdef NON_MATCHING
void overlay1InterpolatePath(f32 *outX, f32 *outZ, s32 path, f32 offset) {
    O1ControlTable *table3Base;
    O1ControlPoint *point0;
    O1ControlPoint *point1;
    O1ControlPoint *point2;
    O1ControlPoint *point3;
    f32 position;
    f32 fraction;
    s32 whole;
    s32 originalWhole;
    s32 remaining;

    position = ((O1PathOffsetOwner *)D_1DA0)->pathOffset + offset;
    point0 = &D_1D60->points[path];
    point1 = &((O1ControlTable *)D_1D64)->points[path];
    point2 = &D_1D68->points[path];
    table3Base = D_1D6C;
    point3 = &table3Base->points[path];
    whole = (s32) position;
    originalWhole = whole;
    remaining = whole - 1;

    if (whole != 0) {
        do {
            table3Base = overlay1NextControlTable(table3Base);
            point0 = point1;
            point1 = point2;
            point2 = point3;
            point3 = &table3Base->points[path];
            whole = remaining;
            remaining--;
        } while (whole != 0);
    }

    fraction = position - (f32) originalWhole;
    *outX = overlay1CubicInterpolate(point0->x, point1->x, point2->x,
                                     point3->x, fraction);
    *outZ = overlay1CubicInterpolate(point0->z, point1->z, point2->z,
                                     point3->z, fraction);
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_head/func_overlay_001_F0000CA8_184D088.s")
#endif

/* ---- overlay1ResolveMotionPoint ---- */

typedef struct O1PathOwner { s16 angle; u8 pad02[0xA]; f32 x; f32 y; f32 z; } O1PathOwner;
extern s32 D_0;
extern f32 D_B4;
extern f32 D_B8;
extern s32 overlay1HasPathData(void);
extern void overlay1InterpolatePath(f32 *x, f32 *z, s32 path, f32 offset);
extern f32 overlay1SinAngle(s16 angle);
extern f32 overlay1CosAngle(s16 angle);
extern f32 overlay1SquareRoot(f32 value);
#ifdef NON_MATCHING
void overlay1ResolveMotionPoint(O1PathOwner *owner, s32 path, f32 *outX,
                                f32 *outY, f32 *outZ) {
    f32 dx;
    f32 dz;
    f32 distance;
    f32 scale;
    if (overlay1HasPathData() == 0) {
        *outX = 0.0f;
        *outY = 0.0f;
        *outZ = 0.0f;
        return;
    }
    if (D_0 == 1) {
        overlay1InterpolatePath(outX, outZ, path, 1.0f);
        dx = *outX - owner->x;
        dz = *outZ - owner->z;
        distance = overlay1SquareRoot((dx * dx) + (dz * dz));
        if (distance > 0.0f) {
            scale = 1.0f / distance;
            dx *= scale;
            dz *= scale;
        }
        *outX = owner->x + (dx * 150.0f);
        *outY = owner->y + D_B4;
        *outZ = owner->z + (dz * 150.0f);
    } else {
        *outX = owner->x + (overlay1SinAngle(owner->angle) * 150.0f);
        *outY = owner->y + D_B8;
        *outZ = owner->z + (overlay1CosAngle(owner->angle) * 150.0f);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_head/func_overlay_001_F0000DF4_184D1D4.s")
#endif

/* ---- overlay1MeasureCurves ---- */

extern f32 overlay1EvaluateCurve(f32, f32, s32, s32, f32);
extern f32 overlay1SquareRoot(f32);
/* Plateau: exact 79-word size, frame 0x70, and five call-relocation sites,
 * but 27 masked words differ from +0xC. The flag lattice and 40-minute
 * permuter found no valid exact source; lower scores reordered calls/guards. */
#ifdef NON_MATCHING
f32 overlay1MeasureCurves(volatile f32 startX, volatile f32 startY,
                          volatile f32 endX, volatile f32 endY,
                          volatile s32 controlX1, volatile s32 controlY1,
                          volatile s32 controlX2, volatile s32 controlY2,
                          s32 segmentCount) {
    f32 t = 0.0f;
    volatile f64 unusedLocal;
    volatile f32 total = 0.0f;
    f32 previousX = overlay1EvaluateCurve(startX, endX, controlX1, controlX2, 0.0f);
    f32 previousY = overlay1EvaluateCurve(startY, endY, controlY1, controlY2, 0.0f);
    f32 step;
    f32 x;
    f32 y;
    f32 dx;
    f32 dy;
    s32 remaining = segmentCount - 1;
    if (segmentCount != 0) {
        step = 1.0f / (f32)segmentCount;
        do {
            t += step;
            x = overlay1EvaluateCurve(startX, endX, controlX1, controlX2, t);
            y = overlay1EvaluateCurve(startY, endY, controlY1, controlY2, t);
            dx = x - previousX;
            dy = y - previousY;
            total += overlay1SquareRoot((dx * dx) + (dy * dy));
            previousX = x;
            previousY = y;
        } while (remaining--);
    }
    return total;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_head/func_overlay_001_F0000F84_184D364.s")
#endif

/* ---- overlay1Noop ---- */


/* DKR v77/v80 and JFG have no overlay-1 donor; this is a generic no-op. */
void overlay1Noop(void) {
}

/* ---- overlay1LoadBuildRecords ---- */


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

/* Plateau (2026-08-24): all 119 flag combinations preserve the same best
 * -O2 -mips2 shape.  It is 0x5C bytes short, differs in 470 of 572 words,
 * and first diverges at +0x34.  The missing scheduling/frame structure is
 * broader than a bounded temporary-order permutation. */
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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_head/func_overlay_001_F00010C8_184D4A8.s")
#endif

/* ---- overlay1InitializeModeState ---- */


typedef struct Overlay1ModeInput {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay1ModeInput;

typedef struct Overlay1ModeSource {
    u8 pad00[0xE];
    u16 value;
} Overlay1ModeSource;

typedef struct Overlay1ModeState {
    u8 pad00[0x37C];
    s16 value;
    u8 mode;
    u8 previousMode;
    u8 phase;
    u8 pad381[0x17];
    s32 selector;
} Overlay1ModeState;

extern void func_overlay_001_F0000BD4_184CFB4(void);
extern u8 func_overlay_001_F0000614_184C9F4(
    f32 x, f32 y, Overlay1ModeSource *source, s32 selector);

void overlay1InitializeModeState(s32 value) {
    ((Overlay1ModeState *)D_1DA0)->value = value;
    ((Overlay1ModeState *)D_1DA0)->mode = 3;
    func_overlay_001_F0000BD4_184CFB4();
    ((Overlay1ModeState *)D_1DA0)->pad381[0x31] =
        (u8)((Overlay1ModeSource *)D_1D64)->value;
    ((Overlay1ModeState *)D_1DA0)->mode =
        func_overlay_001_F0000614_184C9F4(
            ((Overlay1ModeInput *)D_1D9C)->x,
            ((Overlay1ModeInput *)D_1D9C)->y,
            (Overlay1ModeSource *)D_1D64,
            ((Overlay1ModeState *)D_1DA0)->selector);
    ((Overlay1ModeState *)D_1DA0)->previousMode =
        ((Overlay1ModeState *)D_1DA0)->mode;
    ((Overlay1ModeState *)D_1DA0)->phase = 0;
}

/* ---- overlay1BuildObjectMappings ---- */


typedef struct Overlay1BuildData {
    s8 rank;
    s8 index;
    u8 pad02[0x1A6];
    u16 flags;
} Overlay1BuildData;

typedef struct Overlay1BuildObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
    u8 pad18[0x4C];
    Overlay1BuildData *data;
} Overlay1BuildObject;

typedef struct Overlay1BuildState {
    u8 pad00[0x381];
    s8 byte381;
    s8 byte382;
    s8 byte383;
    s8 byte384;
    u8 pad385[0x1B];
    f32 scale;
    u8 pad3A4[0x5C];
    s32 word400;
    s32 word404;
    s32 word408;
    s32 word40C;
    s32 word410;
} Overlay1BuildState;

extern Overlay1BuildObject **overlay1GetBuildObjectsReloc(s32 *count);
extern void overlay1MarkBuildObjectReloc(Overlay1BuildObject *object);
extern void func_overlay_001_F00004B4_184C894(Overlay1BuildObject *object);
extern void func_overlay_001_F00019B8_184DD98(s32 value);
extern s32 gOverlay1BuildGate;
extern u8 gOverlay1RankBase;
extern u8 gOverlay1RankLimit;
extern u8 D_8[];

/* Plateau (2026-08-24): the complete flag lattice ties at -O2 -mips2; the
 * candidate is 0x10 bytes short, differs in 114 of 148 words, and diverges
 * at +0x0.  The object/rank mapping loops need a structural rewrite before
 * register-order work can be meaningful. */
#ifdef NON_MATCHING
void overlay1BuildObjectMappings(volatile s32 unused) {
    s32 count;
    Overlay1BuildObject **base;
    Overlay1BuildObject **outerCursor;
    Overlay1BuildObject *object;
    Overlay1BuildData *data;
    s32 remaining;
    s32 inner;
    Overlay1BuildObject **innerCursor;
    Overlay1BuildObject *innerObject;
    u8 value;

    base = overlay1GetBuildObjectsReloc(&count);
    if (gOverlay1BuildGate != 0) {
        remaining = count - 1;
        if (count != 0) {
            outerCursor = base + remaining;
            do {
                object = *outerCursor;
                data = object->data;
                if (data->rank >= (gOverlay1RankBase - gOverlay1RankLimit)) {
                    data->flags |= 1;
                    overlay1MarkBuildObjectReloc(object);
                } else {
                    data->flags |= 0x20;
                }
                func_overlay_001_F00004B4_184C894(object);
                D_1DA0->scale = 1.0f;
                D_1DA0->byte381 = 0;
                D_1DA0->byte382 = 0;
                D_1DA0->byte383 = -1;
                D_1DA0->byte384 = 0;
                D_1DA0->word400 = 0;
                *(s16 *)((u8 *)D_1DA0 + 0x3BA) = 0xFF;
                *(f32 *)((u8 *)D_1DA0 + 0x3D0) = object->x;
                *(f32 *)((u8 *)D_1DA0 + 0x3D4) = object->y;
                *(f32 *)((u8 *)D_1DA0 + 0x3D8) = object->x;
                *(f32 *)((u8 *)D_1DA0 + 0x3DC) = object->y;
                if (gOverlay1BuildGate == 1) {
                    func_overlay_001_F00019B8_184DD98(0);
                    D_1DA0->word404 = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word404 = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word408 = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word40C = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word410 = 0;
                }
                inner = count - 1;
                if (count != 0) {
                    innerCursor = base + inner;
                    do {
                        innerObject = *innerCursor;
                        value = D_8[(((((*outerCursor)->data->index << 2) +
                                     (*outerCursor)->data->index)) << 1) +
                                    innerObject->data->index];
                        innerCursor--;
                        *((u8 *)D_1DA0 + 0x3A8 + inner) = value;
                    } while (inner--);
                }
                outerCursor--;
            } while (remaining--);
        }
    }
}

s32 overlay1BuildScheduleCarrier(s32 first, s32 second) {
    first += 1;
    first <<= 2;
    return first + second;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_head/func_overlay_001_F0001A54_184DE34.s")
#endif

/* ---- overlay1ReleaseRecords ---- */


typedef struct Overlay1ReleaseRecord {
    u8 pad00[0x14];
    void *resource;
    u8 pad18[4];
} Overlay1ReleaseRecord;

extern s32 gOverlay1ReleaseRecordCount;
extern Overlay1ReleaseRecord *gOverlay1ReleaseRecords;
extern void *gOverlay1ReleaseSecondary;
extern void *gOverlay1ReleaseFinal;
extern void overlay1ReleaseReloc(void *resource);

/* DKR v77/v80 and JFG have generic teardown loops, but no exact donor. */
void overlay1ReleaseRecords(void) {
    s32 remaining;
    Overlay1ReleaseRecord *record;

    if (gOverlay1ReleaseRecordCount != 0) {
        remaining = gOverlay1ReleaseRecordCount - 1;
        record = &gOverlay1ReleaseRecords[1];
        while (remaining-- > 0) {
            if (record->resource != NULL) {
                overlay1ReleaseReloc(record->resource);
            }
            record++;
        }
        overlay1ReleaseReloc(gOverlay1ReleaseRecords);
        overlay1ReleaseReloc(gOverlay1ReleaseSecondary);
        gOverlay1ReleaseRecords = NULL;
        gOverlay1ReleaseSecondary = NULL;
    }
    if (gOverlay1ReleaseFinal != NULL) {
        overlay1ReleaseReloc(gOverlay1ReleaseFinal);
    }
}

/* ---- overlay1CallReset ---- */


/* DKR v77/v80 and JFG have no overlay-1 donor for this wrapper. */
extern void overlay1ResetReloc(void);

void overlay1CallReset(void) {
    overlay1ResetReloc();
}
