#include "PR/ultratypes.h"

typedef struct Overlay53Config {
    u8 bytes[0xA0];
} Overlay53Config;

extern u8 gOverlay53Resource0[];
extern u8 gOverlay53Resource14[];
extern u8 gOverlay53PatchTableA[];
extern u8 gOverlay53PatchTableB[];
extern Overlay53Config gOverlay53ConfigA[];
extern Overlay53Config gOverlay53ConfigB[];
extern s32 gOverlay53Value280;
extern s32 gOverlay53Handles288[2];
extern f32 gOverlay53Height290;
extern u16 gOverlay53GlobalHandleReloc;

extern void overlay53LoadResourceReloc(void *resource);
extern void overlay53LoadTableReloc(void *table);
extern void overlay53SetModeReloc(s32 mode);
extern void overlay53SetTypeReloc(s32 type);
extern void overlay53FinalizeReloc(void);
extern s32 overlay53AcquireHandleReloc(void);
extern void overlay53PatchIndices(void *table);
extern void overlay53InitializeConfig(void *table, Overlay53Config *config,
                                     s32 index, s32 mode);

/* Pinned DKR v77/v80 and JFG scans classify Overlay 53 as no donor. */
void overlay53Initialize(void) {
    Overlay53Config *configA;
    Overlay53Config *configB;
    s32 i;

    overlay53LoadResourceReloc(gOverlay53Resource0);
    overlay53LoadTableReloc(gOverlay53Resource14);
    overlay53SetModeReloc(4);
    gOverlay53Value280 = 0x104;
    overlay53SetTypeReloc(11);
    overlay53PatchIndices(gOverlay53PatchTableA);
    overlay53PatchIndices(gOverlay53PatchTableB);

    configA = gOverlay53ConfigA;
    configB = gOverlay53ConfigB;
    i = 0;
    do {
        overlay53InitializeConfig(gOverlay53PatchTableA, configA, i, 0);
        overlay53InitializeConfig(gOverlay53PatchTableB, configB, i, 1);
        i++;
        configA++;
        configB++;
    } while (i != 2);

    gOverlay53Height290 = -80.0f;
    overlay53FinalizeReloc();
    gOverlay53Handles288[0] = -1;
    gOverlay53Handles288[1] = -1;
    gOverlay53GlobalHandleReloc = overlay53AcquireHandleReloc();
}
