#include "PR/ultratypes.h"

typedef struct Overlay49Status {
    u8 pad00[2];
    u8 player;
    u8 pad03[0x23];
    u16 value;
} Overlay49Status;

typedef struct Overlay49LookupResult {
    u8 pad00[8];
    void *value;
} Overlay49LookupResult;

extern s32 gOverlay49Timer;
extern s32 gOverlay49Finished;
extern s8 gOverlay49Modes[];
extern s32 gOverlay49Masks[];
extern s32 gOverlay49Shifts[];
extern void *gOverlay49Result;
extern s32 gOverlay49FastFinishEnabled;
extern u8 D_8007BF04;
extern u8 D_8007BF08;
extern u16 D_800D3128[];
extern Overlay49Status *func_80028F54(void);
extern void overlay65Initialize(void);
extern Overlay49LookupResult *func_800508B4(s32 id);
extern void func_8002917C(s32 mode);

/* Donor scan: DKR v77/v80 and JFG contain no exact initializer donor. */
void overlay49Initialize(void) {
    s32 unused0;
    s32 unused1;
    s32 shift;
    s32 mask;
    s32 desired;
    s32 current;
    Overlay49LookupResult *lookup;
    Overlay49Status *status;
    s32 i;
    s8 *modes;
    u16 *packed;

    status = func_80028F54();
    overlay65Initialize();
    if (D_8007BF08 != 0) {
        mask = 0xE00;
        shift = 9;
    } else {
        mask = gOverlay49Masks[D_8007BF04];
        shift = gOverlay49Shifts[D_8007BF04];
    }
    packed = &D_800D3128[status->player];
    gOverlay49Result = NULL;
    current = (packed[4] & mask) >> shift;
    if (status[0].value == 0x24) {
        lookup = func_800508B4(0x1E);
        if (lookup != NULL) {
            gOverlay49Result = lookup->value;
        }
        desired = 4;
    } else {
        desired = 0;
        modes = gOverlay49Modes;
        for (i = 1; i < 6; i++) {
            if (status[0].value < status[i].value) {
                desired++;
            }
        }
        desired = modes[desired];
    }
    if (current < desired) {
        D_800D3128[status->player + 4] &= ~mask;
        D_800D3128[status->player + 4] |= desired << shift;
        func_8002917C(desired);
    }
    if (current < 3 && current < desired) {
        goto slow;
    }
    gOverlay49FastFinishEnabled = 1;
    goto finish;
slow:
    gOverlay49FastFinishEnabled = 0;
finish:
    gOverlay49Timer = 0xA50;
    gOverlay49Finished = 0;
}
