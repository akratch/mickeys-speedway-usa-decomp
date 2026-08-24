/* DKR v77/v80 has generic release wrappers only, with no exact donor. */
extern void *gOverlay68Tertiary;
extern void overlay68ReleaseReloc(void *resource);

void overlay68ReleaseTertiary(void) {
    if (gOverlay68Tertiary != 0) {
        overlay68ReleaseReloc(gOverlay68Tertiary);
    }
    gOverlay68Tertiary = 0;
}
