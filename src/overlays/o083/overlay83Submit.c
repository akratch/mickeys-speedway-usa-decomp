#include "PR/ultratypes.h"

extern void overlay83PrepareReloc(void *object);
extern void overlay83CommitReloc(void *request);

/* DKR v77/v80 and JFG checks found no exact donor for this two-stage wrapper. */
void overlay83Submit(void *object, void *request) {
    overlay83PrepareReloc(object);
    overlay83CommitReloc(request);
}
