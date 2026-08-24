#include "PR/ultratypes.h"

typedef struct Overlay88Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay88Vector;

typedef struct Overlay88Transform {
    u8 pad00[0x30];
    f32 x;
    u8 pad34[4];
    f32 z;
} Overlay88Transform;

typedef struct Overlay88Node {
    void *descriptor;
    u8 pad04[4];
    s16 selector;
    s16 transformIndex;
    Overlay88Transform *transforms[1];
} Overlay88Node;

typedef struct Overlay88Descriptor {
    u8 pad00[0x11];
    u8 needsRefresh;
} Overlay88Descriptor;

typedef struct Overlay88Anchor {
    u8 pad00[0x44];
    f32 x;
    f32 y;
    f32 z;
    s16 angle;
} Overlay88Anchor;

typedef struct Overlay88Object {
    u8 pad00[0x10];
    f32 y;
    u8 pad14[0x3C];
    void *context;
    u8 pad54[0x10];
    Overlay88Anchor *anchor;
    Overlay88Node **nodes;
    u8 pad6C[0x27];
    u8 nodeIndex;
} Overlay88Object;

extern void overlay88PrepareNodeReloc(Overlay88Node *node, void *descriptor,
                                      Overlay88Object *object);
extern void overlay88UpdateNodeReloc(Overlay88Object *object,
                                     Overlay88Node *node, void *context,
                                     Overlay88Transform *transform);
extern void overlay88RefreshNodeReloc(Overlay88Object *object,
                                      Overlay88Descriptor *descriptor,
                                      Overlay88Node *sameNode, s16 selector);
extern void overlay88ForwardVectorReloc(Overlay88Transform *transform,
                                       f32 x, f32 y, f32 z,
                                       f32 *outX, f32 *outY, f32 *outZ);
extern s16 overlay88AngleReloc(f32 x, f32 z);
extern f32 overlay88SinReloc(s16 angle);
extern f32 overlay88CosReloc(s32 angle);

/* Exact DKR v77/v80 and JFG scans are negative for this routine. */
void overlay88UpdateAnchor(Overlay88Object *object, s32 updateContext) {
    Overlay88Node *node;
    Overlay88Anchor *anchor;
    Overlay88Descriptor *descriptor;
    Overlay88Transform *transform;
    f32 forwardX;
    f32 forwardY;
    f32 forwardZ;
    s16 angle;

    node = object->nodes[object->nodeIndex];
    anchor = object->anchor;
    descriptor = node->descriptor;
    overlay88PrepareNodeReloc(node, descriptor, object);
    overlay88UpdateNodeReloc(object, node, object->context,
                             node->transforms[node->transformIndex]);
    if (descriptor->needsRefresh != 0) {
        overlay88RefreshNodeReloc(object, descriptor, node, node->selector);
    }

    node->selector = 0;
    transform = node->transforms[node->transformIndex];
    overlay88ForwardVectorReloc(transform, 0.0f, 0.0f, 1.0f,
                                &forwardX, &forwardY, &forwardZ);
    angle = overlay88AngleReloc(forwardX, forwardZ);
    anchor->x = transform->x - overlay88SinReloc(angle) * 18.0f;
    anchor->y = object->y;
    anchor->z = transform->z - overlay88CosReloc(angle) * 18.0f;
    anchor->angle = angle;
}
