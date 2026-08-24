/* Generic double-release wrapper only; no exact DKR donor. */
extern void *gOverlay65Resource208;
extern void *gOverlay65Resource2980;
extern void overlay65ReleaseReloc(void *resource);

void overlay65Release(void) {
    overlay65ReleaseReloc(gOverlay65Resource208);
    overlay65ReleaseReloc(gOverlay65Resource2980);
}
