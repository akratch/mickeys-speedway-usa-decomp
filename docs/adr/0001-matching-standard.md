# 0001. Matching standard

Status: Accepted
Date: 2026-08-24

## Context

The tree had no written definition of "matched." In its absence, functions
whose object was edited after compilation to force byte-identity were being
counted the same as functions the compiler produced byte-identically on its
own. `docs/acceleration-survey.md` §1 measured the effect: 63.5% of the
"overlay C" byte total came from 274 objects that had instruction words
added, reordered, dropped, or commuted by `normalize_elf_instructions.py`
and related tools after `tools/ido/cc` ran (13,889 instruction-level
operations, of them 13,332 field edits, 250 reorders, 31 deletions). Only
10.3% of that total was untouched compiler output.

§13.1 surveyed how the five reference decomps define "matched." DKR's
`tools/python/score.py` is textual: it parses `src/**.c`, weights each
function by its map-file size, and subtracts every `GLOBAL_ASM`. Its
`WIP_REGEX` specifically rewrites `#ifdef NON_MATCHING … GLOBAL_ASM … #endif`
back to a bare `GLOBAL_ASM` before counting, so a function that compiles but
isn't byte-identical is not credited just because a C body exists for it.
This is also, independently, the definition `objdiff` and decomp.me use:
100% match means the compiler's output for the given C, linked at the
function's real address, is byte-identical to the ROM.

## Decision

**"Matched" means: the untouched output of `tools/ido/cc` compiling the C
source in the tree, linked at the function's real ROM offset, is
byte-identical to the ROM at that offset.** No step between compilation and
comparison may change an instruction word. This is the objdiff/DKR
definition, not a Mickey-specific one.

A function's C source may be written under an `#ifdef NON_MATCHING` guard,
compiling to a bytes-plausible but non-identical object, with the original
`#pragma GLOBAL_ASM` fallback preserved under `#else`. Both `NON_MATCHING`
and its stricter cousin `NON_EQUIVALENT` (compiles, but the semantic
decompilation itself is understood to be wrong, not just mis-scheduled) are
**counted as unmatched**, identically to a function still in
`GLOBAL_ASM`, exactly DKR's `WIP_REGEX` semantics.

Hand-written original assembly (`verified_asm.us.txt`) is its own counted
category, not folded into either matched or unmatched: it is code nobody is
trying to recompile from C, and DKR counts its equivalent
(`src/hasm/*.s`, `hasm_in_src_path: True`) the same way.

## Consequences

- The scoreboard (ADR 0003) is redefined on this standard; the immediate
  effect is that matched overlay C drops from 276,600 bytes to at most
  100,876 bytes (28,452 untouched-compiler bytes plus 72,424 bytes pending
  per-object review for metadata-only post-processing), pending the
  per-object review ADR 0002 requires.
- `tools/progress.py` and `gmake scoreboard` must stop counting an object as
  matched solely because its bytes compare equal; they must also confirm no
  instruction-altering post-process step ran on that object (ADR 0002,
  ADR 0003).
- This does not change what counts as *progress worth reporting*: a
  NON_MATCHING function with a semantically faithful C body and a known,
  described mismatch is real, useful work. It changes what counts as *done*.
