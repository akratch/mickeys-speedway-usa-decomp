#include "PR/ultratypes.h"

/* Both local DKR v77/v80 trees are negative for this wrapper. */
extern void overlay42ResumeReloc(void);

void overlay42Resume(void) {
    overlay42ResumeReloc();
}
