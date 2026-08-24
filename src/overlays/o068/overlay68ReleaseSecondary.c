/* DKR v77/v80 has generic release wrappers only, with no exact donor. */
extern void *gOverlay68Secondary;
extern void overlay68ReleaseReloc(void *resource);

void overlay68ReleaseSecondary(void) {
    if (gOverlay68Secondary != 0) {
        overlay68ReleaseReloc(gOverlay68Secondary);
    }
    gOverlay68Secondary = 0;
}
