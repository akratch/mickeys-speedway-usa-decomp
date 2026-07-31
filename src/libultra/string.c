/*
 * libultra libc string routines -- ROM 0x72370-0x72410 (VRAM 0x80071770).
 *
 * First C translation unit in this project; it exists to prove the IDO 5.3 +
 * asm-processor build path end to end.  The TU was identified by exact byte
 * identity with the built libultra `string.c` object of the public DKR
 * decompilation, and its extent (0xA0 bytes, including a 12-byte tail pad) is
 * what fixed the split point in mickey.us.yaml.
 *
 * Build flags: -O2 -mips2 -32.  This deviates from the project default
 * (-mips1) and the deviation is forced by evidence: with -mips1 IDO will not
 * emit branch-likely instructions and the TU compiles to 0x90 bytes that do
 * not match the ROM; with -mips2 it emits the `bnel`/`beql` forms the ROM
 * actually contains and reproduces all 0xA0 bytes exactly.  See the per-file
 * MIPSISET override in the Makefile.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (DKR's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"
#include "string.h"

/*
 * The naive byte-at-a-time copy, not the word-at-a-time one -- libultra's
 * fast path for bulk copies is `bcopy`, which lives in hand-written assembly
 * elsewhere in this same libultra block (ROM 0x72420).  The compiled form
 * hoists the zero-length test out of the loop and writes through the return
 * value with a -1 displacement, which is what a plain post-increment loop
 * returning the original pointer gives you at -O2.
 */
void *memcpy(void *dst, const void *src, size_t size) {
    char *d = (char *)dst;
    const char *s = (const char *)src;

    while (size > 0) {
        *d = *s;
        d++;
        s++;
        size--;
    }
    return (void *)dst;
}

size_t strlen(const char *str) {
    const char *p = str;

    while (*p != 0) {
        p++;
    }
    return p - str;
}

/*
 * First matched C function in the project.
 *
 * Reads as the textbook strchr: walk the string comparing against the search
 * character truncated to `char`, stop at the terminator. The two details the
 * codegen pins down are (a) `ch` is narrowed once, up front, rather than
 * compared as an int each iteration, and (b) the terminator test happens
 * inside the loop after the match test, not as a separate leading condition --
 * which is why the compiled form enters the loop body with the first byte
 * already loaded.
 */
char *strchr(const char *str, int ch) {
    const char c = ch;

    while (*str != c) {
        if (*str == 0) {
            return NULL;
        }
        str++;
    }
    return (char *)str;
}
