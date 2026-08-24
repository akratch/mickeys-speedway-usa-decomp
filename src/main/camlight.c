/*
 * Disabled camera-light interface -- ROM 0x1BE50-0x1BEA0.
 *
 * PROVENANCE -- the TU and function names are borrowed from Jet Force
 * Gemini's public retail-derived src/camlight.c and its nonmatching assembly
 * names.  Mickey's same-order entry points are deliberately stubbed; no JFG
 * body is adapted here.  Evidence is tier B/D, not tier-A byte identity.
 */

void camlightInit(void) {
}
void camlightFlush(void) {
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camlight/camlightAdd.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camlight/camlightDelete.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camlight/camlightUpdateAll.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camlight/camlightUpdate.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camlight/camlightVisibilityCheck.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camlight/camlightDraw.s")
