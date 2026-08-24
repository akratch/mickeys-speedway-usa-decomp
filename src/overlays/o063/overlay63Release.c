extern void *gOverlay63Resource;
extern unsigned char gOverlay63Block[];
extern void overlay63ReleaseReloc(void *resource);

void overlay63Release(void) {
    overlay63ReleaseReloc(gOverlay63Resource);
    overlay63ReleaseReloc(gOverlay63Block);
}
