#include "PR/ultratypes.h"

typedef struct Overlay46Resource Overlay46Resource;
typedef struct Overlay46Group Overlay46Group;

typedef struct Overlay46Data {
    s16 resourceIds[0x26];
    Overlay46Resource *resource4C;
    Overlay46Resource *resource50;
    Overlay46Group *group54;
    u8 pad58[0x12C];
    Overlay46Resource *resource184;
} Overlay46Data;

extern Overlay46Data gOverlay46Data;
extern void overlay46ReleaseIdsReloc(s16 *ids);
extern void overlay46ClearGroupReloc(Overlay46Group *group);
extern void overlay46SyncResourceReloc(Overlay46Resource *resource);
extern void overlay46RegisterResourceReloc(Overlay46Resource *resource);
extern u8 overlay46GetModeReloc(void);
extern void overlay46SetModeReloc(s32 mode);

void overlay46ReleaseState(void) {
    Overlay46Resource *resource;

    overlay46ReleaseIdsReloc(gOverlay46Data.resourceIds);
    overlay46ClearGroupReloc(gOverlay46Data.group54);

    resource = gOverlay46Data.resource50;
    if (resource != NULL) {
        overlay46SyncResourceReloc(resource);
    }
    resource = gOverlay46Data.resource4C;
    if (resource != NULL) {
        overlay46SyncResourceReloc(resource);
    }
    if (gOverlay46Data.resource184 != NULL) {
        overlay46RegisterResourceReloc(gOverlay46Data.resource184);
    }

    overlay46SetModeReloc(overlay46GetModeReloc());
}
