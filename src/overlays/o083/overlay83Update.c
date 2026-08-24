typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef signed long s32;
typedef float f32;

typedef struct Overlay83Record {
    s16 x;
    s16 y;
    s16 z;
    s8 life;
    s8 alpha;
    f32 halfLength;
    f32 worldX;
    f32 worldY;
    f32 worldZ;
} Overlay83Record;

typedef struct Overlay83Object {
    s16 x;
    s16 y;
    s16 z;
    s16 height;
    u8 pad08[4];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    s16 halfLength;
    s16 velocityX;
    s16 velocityY;
    s16 velocityZ;
    u8 pad20[3];
    u8 opacity;
    u8 pad24[4];
    void *linkedObject;
    u8 firstRecord;
    u8 lastRecord;
    u8 recordCount;
    u8 pad2F;
    Overlay83Record records[8];
} Overlay83Object;

typedef struct Overlay83SourceState {
    u8 pad00[2];
    s16 opacity;
} Overlay83SourceState;

typedef struct Overlay83Parent {
    u8 pad00[0x0C];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
    u8 pad18[0x4C];
    Overlay83SourceState *sourceState;
} Overlay83Parent;

typedef struct Overlay83LinkedObject {
    u8 pad00[0x18];
    f32 worldX;
    f32 worldY;
    f32 worldZ;
} Overlay83LinkedObject;

extern void overlay83TransformObjectReloc(s32 mode, Overlay83Object *object,
                                          f32 *source, f32 *destination);
extern void overlay83UpdateLinkedReloc(Overlay83LinkedObject *object,
                                       s32 opacity);

#ifdef NON_MATCHING
void overlay83Update(Overlay83Parent *parent,
                     Overlay83Object *object,
                     s32 updateRate) {
    s32 activeCount;
    s32 recordIndex;
    Overlay83Record *record;
    s16 sourceOpacity;
    Overlay83SourceState *sourceState;
    u8 oldLast;

    sourceState = parent->sourceState;
    activeCount = object->recordCount;
    if (activeCount != 0) {
        recordIndex = object->firstRecord;
        do {
            record = &object->records[recordIndex];
            recordIndex++;
            if (recordIndex >= 8) {
                recordIndex = 0;
            }
            record->life--;
            if (record->life < 0) {
                object->firstRecord = recordIndex;
                object->recordCount--;
            } else {
                record->alpha = (record->life * 0x24 * object->opacity) >> 8;
            }
        } while (recordIndex != object->lastRecord);
        activeCount = object->recordCount;
    }

    sourceOpacity = sourceState->opacity;
    object->opacity = sourceOpacity;
    if (activeCount < 8 && (sourceOpacity & 0xFF) != 0) {
        oldLast = object->lastRecord;
        object->recordCount = activeCount + 1;
        record = &object->records[oldLast];
        object->lastRecord = oldLast + 1;
        if (object->lastRecord >= 8) {
            object->lastRecord = 0;
        }
        record->x = object->x;
        record->y = object->y;
        record->z = object->z;
        record->life = 7;
        record->alpha = (record->life * 0x24 * object->opacity) >> 8;
        record->halfLength = object->halfLength;
        record->worldX = object->worldX;
        record->worldY = object->worldY;
        record->worldZ = object->worldZ;
    }

    object->x += object->velocityX * updateRate;
    object->y += object->velocityY * updateRate;
    object->worldY = object->height;
    object->worldX = 0.0f;
    object->worldZ = 0.0f;
    object->z += object->velocityZ * updateRate;
    overlay83TransformObjectReloc(1, object, &object->worldX, &object->worldX);
    object->worldX += parent->worldX;
    object->worldY += parent->worldY;
    object->worldZ += parent->worldZ;

    if (object->linkedObject != 0) {
        Overlay83LinkedObject *linkedObject = object->linkedObject;
        linkedObject->worldX = object->worldX;
        linkedObject->worldY = object->worldY;
        linkedObject->worldZ = object->worldZ;
        overlay83UpdateLinkedReloc(linkedObject, object->opacity);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o083/overlay83Update/func_overlay_083_F00002A0_18CFA60.s")
#endif
