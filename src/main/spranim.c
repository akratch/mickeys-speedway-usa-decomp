/*
 * Sprite/object animation controls -- ROM 0x1BEA0-0x1C790.
 *
 * PROVENANCE -- the TU and seven descriptive function names are borrowed
 * from Jet Force Gemini's public retail-derived src/spranim.c and its
 * nonmatching assembly names.  The attribution is supported at tier B by
 * object-control call roles and at tier D by function order and masked
 * instruction shape.  No JFG body is adapted by this scaffold.
 */

#include "PR/ultratypes.h"

typedef struct SpranimBAE4Target {
    u8 pad0[0x132];
    s16 state132;
} SpranimBAE4Target;

typedef struct SpranimBAE4Object {
    u8 pad0[0x58];
    SpranimBAE4Target *target58;
} SpranimBAE4Object;

#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/spranimInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/spranimControl.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/sprasjiInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/spranimOnceControl.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/effectboxControl.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/texscrollControl.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/func_8001B798.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/rangetriggerControl.s")
void func_8001BAE4(SpranimBAE4Object *arg0, void *arg1) {
    arg0->target58->state132 = 1;
}
void func_8001BAF8(void *arg0, void *arg1) {
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/func_8001BB04.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/func_8001BB10.s")
