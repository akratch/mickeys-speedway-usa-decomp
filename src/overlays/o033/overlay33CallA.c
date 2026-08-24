/* Pinned DKR scans found only generic zero-argument wrappers. */
extern void overlay33CallReloc(void);

void overlay33CallA(void) {
    overlay33CallReloc();
}
