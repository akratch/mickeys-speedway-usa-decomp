/* Generic submission wrapper only; no exact DKR donor. */
extern unsigned char gOverlay46SubmitBlock[];
extern void overlay46SubmitReloc(void *block);

void overlay46Submit(void) {
    overlay46SubmitReloc(gOverlay46SubmitBlock);
}
