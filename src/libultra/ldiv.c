/*
 * Source provenance: reconstructed from the matching libultra implementation
 * in the permitted Jet Force Gemini reference tree; see docs/CLEANROOM.md.
 * Its SDK object is an all-phase -O3 build.
 */

#include "libc/stdlib.h"

lldiv_t lldiv(long long num, long long denom) {
    lldiv_t ret;

    ret.quot = num / denom;
    ret.rem = num - denom * ret.quot;

    if (ret.quot < 0 && ret.rem > 0) {
        ret.quot += 1;
        ret.rem -= denom;
    }

    return ret;
}

ldiv_t ldiv(long num, long denom) {
    ldiv_t ret;

    ret.quot = num / denom;
    ret.rem = num - denom * ret.quot;

    if (ret.quot < 0 && ret.rem > 0) {
        ret.quot += 1;
        ret.rem -= denom;
    }

    return ret;
}
