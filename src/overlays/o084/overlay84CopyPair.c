#include "PR/ultratypes.h"

typedef struct Overlay84Pair {
    s16 first;
    s16 second;
} Overlay84Pair;

typedef struct Overlay84Entry {
    u8 pad0[0xC];
    s16 second;
    s16 first;
} Overlay84Entry;

void overlay84CopyPair(Overlay84Pair *dst, Overlay84Entry *src) {
    dst->first = src->first;
    dst->second = src->second;
}
