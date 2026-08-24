#include "PR/ultratypes.h"
#include "overlays/overlay057.h"

extern void *gOverlay57Range150Start[];
extern void *gOverlay57Range150End[];

extern void *gOverlay57Resource06C;
extern void *gOverlay57Resource074;
extern void *gOverlay57Resource070;
extern void *gOverlay57Resource078;
extern void *gOverlay57Resource000;
extern void *gOverlay57Resource0F8;
extern void *gOverlay57Resource0FC;

extern void *gOverlay57Range058Start[];
extern void *gOverlay57Range058End[];
extern void *gOverlay57Range008Start[];
extern void *gOverlay57Range008End[];
extern void *gOverlay57Range030Start[];
extern void *gOverlay57Range030End[];
extern void *gOverlay57Range080Start[];
extern void *gOverlay57Range080End[];
extern void *gOverlay57Range0E0Start[];
extern void *gOverlay57Range0E0End[];

extern u8 gOverlay57Object100[];

extern void overlay57ReleaseResidentReloc(void *resource);
extern void overlay57ReleaseDescriptorReloc(void *resource);
extern void overlay57FinalizeReloc(void *object);

void overlay57ReleaseAll(void) {
    void **cursor;
    void **end;

    cursor = gOverlay57Range150Start;
    end = gOverlay57Range150End;
    do {
        overlay57ReleaseResidentReloc(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    overlay57ReleaseDescriptorReloc(gOverlay57Resource06C);
    overlay57ReleaseDescriptorReloc(gOverlay57Resource074);
    overlay57ReleaseDescriptorReloc(gOverlay57Resource070);
    overlay57ReleaseDescriptorReloc(gOverlay57Resource078);
    overlay57ReleaseDescriptorReloc(gOverlay57Resource000);
    overlay57ReleaseDescriptorReloc(gOverlay57Resource0F8);
    overlay57ReleaseDescriptorReloc(gOverlay57Resource0FC);

    cursor = gOverlay57Range058Start;
    end = gOverlay57Range058End;
    do {
        overlay57ReleaseDescriptorReloc(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range008Start;
    end = gOverlay57Range008End;
    do {
        overlay57ReleaseDescriptorReloc(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range030Start;
    end = gOverlay57Range030End;
    do {
        overlay57ReleaseDescriptorReloc(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range080Start;
    end = gOverlay57Range080End;
    do {
        overlay57ReleaseDescriptorReloc(*cursor);
        cursor++;
    } while ((u32) cursor < (u32) end);

    cursor = gOverlay57Range0E0Start;
    end = gOverlay57Range0E0End;
    do {
        overlay57ReleaseDescriptorReloc(*cursor);
        cursor++;
    } while (cursor != end);

    overlay57FinalizeReloc(gOverlay57Object100);
}
