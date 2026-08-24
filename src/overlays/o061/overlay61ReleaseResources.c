#include "PR/ultratypes.h"

extern u8 gOverlay61ResourceBaseReloc[];
extern void *gOverlay61Resource58Reloc;
extern void *gOverlay61Resource5CReloc;
extern void *gOverlay61Resource60Reloc;
extern void *gOverlay61Resource64Reloc;
extern void *gOverlay61Resource68Reloc;
extern void *gOverlay61Resource6CReloc;
extern void *gOverlay61Resource70Reloc;
extern void *gOverlay61Resource74Reloc;
extern void *gOverlay61Resource78Reloc;
extern void *gOverlay61Resource7CReloc;
extern void *gOverlay61Resource80Reloc;
extern void *gOverlay61Resource84Reloc;
extern void *gOverlay61Resource88Reloc;

extern void overlay61ReleaseBaseReloc(void *);
extern void overlay61ReleaseResourceReloc(void *);
extern void overlay61FinishReleaseReloc(void);
extern void overlay61SetReleaseModeReloc(s32);

void overlay61ReleaseResources(void) {
    overlay61ReleaseBaseReloc(gOverlay61ResourceBaseReloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource58Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource5CReloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource60Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource64Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource68Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource6CReloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource70Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource74Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource78Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource7CReloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource80Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource84Reloc);
    overlay61ReleaseResourceReloc(gOverlay61Resource88Reloc);
    overlay61FinishReleaseReloc();
    overlay61SetReleaseModeReloc(1);
}
