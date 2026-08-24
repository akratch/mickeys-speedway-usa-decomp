#include "PR/ultratypes.h"

typedef struct {
    u8 pad00[0x28];
    f32 parameter;
} Overlay58Target;

typedef struct {
    u8 pad00[8];
    Overlay58Target *target;
    f32 value;
    u8 pad10[4];
    u8 active;
} Overlay58Node;

extern Overlay58Node *overlay58FindNodeReloc(s32 id);
extern void overlay58ConfigureNodeReloc(Overlay58Target *target, s32 argument,
                                        s32 mode, f32 parameter);

/* DKR v77/v80 and JFG have no exact donor for this node-update routine. */
void overlay58SetNodeValue(s32 id, s32 argument, f32 value) {
    Overlay58Node *node;

    node = overlay58FindNodeReloc(id & 0xFF);
    if ((node != NULL) && (node->target != NULL)) {
        if (value < 0.0f) {
            overlay58ConfigureNodeReloc(node->target, argument, -1,
                                        node->target->parameter);
        } else {
            overlay58ConfigureNodeReloc(node->target, argument, -1, 0.0f);
        }
        node->value = value;
        node->active = 0;
    }
}
