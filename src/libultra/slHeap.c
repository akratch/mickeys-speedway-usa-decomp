/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/reference-findings.md section 3.
 */

#include "n_audio/synthInternals.h"
#include "n_audio/libaudio.h"
#include "PR/os_internal.h"
#include "PR/ultraerror.h"

void alHeapInit(ALHeap *hp, u8 *base, s32 len)
{
    hp->base = base;
    hp->len  = len;
    hp->cur  = hp->base;
    hp->count = 0;
}

void *alHeapDBAlloc(u8 *file, s32 line, ALHeap *hp, s32 num, s32 size)
{
    s32 bytes;
    u8 *ptr = 0;

    bytes = (num*size + AL_CACHE_ALIGN) & ~AL_CACHE_ALIGN;

#ifdef _DEBUG
    hp->count++;
    bytes += sizeof(HeapInfo);
#endif

    if ((hp->cur + bytes) <= (hp->base + hp->len)) {

        ptr = hp->cur;
        hp->cur += bytes;

#ifdef _DEBUG
        ((HeapInfo *)ptr)->magic = AL_HEAP_MAGIC;
        ((HeapInfo *)ptr)->size  = bytes;
        ((HeapInfo *)ptr)->count = hp->count;
        if (file) {
            ((HeapInfo *)ptr)->file  = file;
            ((HeapInfo *)ptr)->line  = line;
        } else {
            ((HeapInfo *)ptr)->file  = (u8 *) "unknown";
            ((HeapInfo *)ptr)->line  = 0;
        }

        ptr += sizeof(HeapInfo);
#endif

    } else {
#ifdef _DEBUG
        __osError(ERR_ALHEAPNOFREE, 1, size);
#endif
    }

    return ptr;
}
