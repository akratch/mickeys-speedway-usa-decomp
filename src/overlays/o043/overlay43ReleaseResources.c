#include "PR/ultratypes.h"

typedef struct Overlay43ResourceOwner {
    u8 pad00[0x24];
    void *resource24;
    void *resource28;
    void *resource2C;
    u8 pad30[0x04];
    void *resource34;
    void *children[12];
    void *resource68;
    u8 pad6C[0x4C];
    u8 childCount;
} Overlay43ResourceOwner;

extern void mmFree(void *resource);
extern void modFreeModel(void *resource);

void func_overlay_043_F0000280_188A250(Overlay43ResourceOwner *owner) {
    s32 index;

    mmFree(owner->resource24);
    mmFree(owner->resource28);
    mmFree(owner->resource2C);
    if (owner->resource34 != 0) {
        mmFree(owner->resource34);
    }
    if (owner->resource68 != 0) {
        modFreeModel(owner->resource68);
    }
    for (index = 0; index < owner->childCount; index++) {
        mmFree(owner->children[index]);
    }
}
