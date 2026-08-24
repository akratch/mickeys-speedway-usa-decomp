#include "PR/ultratypes.h"

/* Overlay 105: negative DKR/JFG scans; both relocs name libultra PI locks. */
void overlay105PiGetAccessReloc(void);
void overlay105PiRelAccessReloc(void);

s32 overlay105CheckWord(u32 base) {
    struct {
        u32 unused;
        u32 status;
    } locals;
    volatile u32 *piStatus;

    overlay105PiGetAccessReloc();
    piStatus = (volatile u32 *)0xA4600010;
    locals.status = *piStatus;
    while ((locals.status & 3) != 0) {
        locals.status = *piStatus;
    }
    locals.status = *(u32 *)((base | 0xB0000A5C) | 0xA0000000);
    overlay105PiRelAccessReloc();
    if (locals.status != 0x1740FFF9) {
        return 0;
    }
    return 1;
}
