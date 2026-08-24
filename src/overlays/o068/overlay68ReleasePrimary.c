/* DKR v77/v80 has generic release wrappers only, with no exact donor. */
extern void *gOverlay68Primary;
extern void overlay68ReleaseReloc(void *resource);

void overlay68ReleasePrimary(void) {
    if (gOverlay68Primary != 0) {
        overlay68ReleaseReloc(gOverlay68Primary);
    }
    gOverlay68Primary = 0;
}
