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

extern void func_8002B768(void *resource);
extern void func_80020184(void *resource);

void func_overlay_043_F0000280_188A250(Overlay43ResourceOwner *owner) {
    s32 index;

    func_8002B768(owner->resource24);
    func_8002B768(owner->resource28);
    func_8002B768(owner->resource2C);
    if (owner->resource34 != 0) {
        func_8002B768(owner->resource34);
    }
    if (owner->resource68 != 0) {
        func_80020184(owner->resource68);
    }
    for (index = 0; index < owner->childCount; index++) {
        func_8002B768(owner->children[index]);
    }
}
