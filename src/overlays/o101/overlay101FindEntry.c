#include "PR/ultratypes.h"

typedef struct Overlay101Node {
    s32 value;
    struct Overlay101Node *next;
} Overlay101Node;

Overlay101Node *overlay101FindEntry(Overlay101Node *head, s32 value,
                                    s32 occurrence) {
    s32 current;
    s32 found;
    Overlay101Node *node;

    found = 0;
    current = head->value;
    node = head->next;
    if ((node != NULL) && (current != 0)) {
        do {
            if (current == value) {
                if (found == occurrence) {
                    return node;
                }
                found++;
            }
            current = node->value;
            node = node->next;
        } while ((node != NULL) && (current != 0));
    }
    return NULL;
}
