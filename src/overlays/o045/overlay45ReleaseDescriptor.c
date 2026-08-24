#include "overlays/o045/resource_descriptor.h"

extern Overlay45ResourceDescriptor *gOverlay45ResourceHead;
extern s32 overlay45FreeReloc(void *allocation);

/*
 * Overlay 45 +0x270. Fresh DKR v77/v80 searches for linked-list removal via
 * offsets +0x30/+0x2C, and for analogous descriptor release loops, were
 * negative.
 */
void overlay45ReleaseDescriptor(Overlay45ResourceDescriptor *descriptor) {
    Overlay45ResourceDescriptor *current;
    Overlay45ResourceDescriptor *previous;
    Overlay45ResourceDescriptor *target;

    current = gOverlay45ResourceHead;
    target = descriptor;
    previous = NULL;
    if (current != NULL && descriptor != NULL) {
        do {
            if (current == target) {
                if (previous == NULL) {
                    gOverlay45ResourceHead = target->next;
                } else {
                    previous->next = target->next;
                }
                overlay45FreeReloc(target->allocation);
                target = NULL;
            } else {
                previous = current;
                current = current->next;
            }
        } while (current != NULL && target != NULL);
    }
}
