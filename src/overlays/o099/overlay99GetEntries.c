#include "PR/ultratypes.h"

extern u8 gOverlay99Entries[];

/* DKR v77/v80 and JFG checks found no exact donor for this base accessor. */
void *overlay99GetEntries(void) {
    return gOverlay99Entries;
}
