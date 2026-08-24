# 0003. Scoreboard

Status: Accepted
Date: 2026-08-24

## Context

The README's Progress block and `tools/progress.py` reported a single
matched-bytes figure that did not distinguish untouched compiler output from
objects reached via the instruction-editing steps ADR 0002 retires, and did
not separately track hand-written assembly, `NON_MATCHING`, or
`NON_EQUIVALENT` functions. `docs/acceleration-survey.md` §13.1 found DKR's
`score.py` reports five distinct lines, generated from `src/**.c` and the
map file, with `GLOBAL_ASM` (including `WIP_REGEX`-unwrapped
`NON_MATCHING`/`NON_EQUIVALENT` blocks) always subtracted from the
decompiled total. Project-level verification stays separate: the ROM SHA1
check, with dedicated CI jobs that compile-only under `NON_MATCHING=1` and
`NON_EQUIVALENT=1`.

## Decision

Adopt DKR's five-line scoreboard as Mickey's scoreboard, byte-weighted
(weighting each function by its size, as DKR does from the map file):

1. **Decompiled**: matched, per ADR 0001: untouched `tools/ido/cc` output,
   linked at the real offset, byte-identical, with no instruction-altering
   post-process step in its object's build path.
2. **Handwritten ASM**: `verified_asm.us.txt`, counted on its own line,
   counted toward the total the way DKR counts `src/hasm/*.s`.
3. **GLOBAL_ASM remaining**: extracted, unattempted or unresolved.
4. **NON_MATCHING**: has a C body believed semantically correct, compiles,
   does not byte-match; kept under `#ifdef NON_MATCHING` over a
   `#pragma GLOBAL_ASM` fallback per ADR 0002.
5. **NON_EQUIVALENT**: compiles, but is understood not to even be
   semantically the same function; a stricter, more honest sibling of
   NON_MATCHING.

A range counts toward line 1 **only if** the object it lives in has no
instruction-altering post-compile step anywhere in its build (ADR 0002).
This is a per-object gate, not a per-function guess: if one function in an
object was reached via `normalize_elf_instructions.py`, every function's
matched status in that object is suspect until the object is rebuilt clean
or the offending function is split out.

Every count is derived mechanically from the tree at the moment it's asked
for (`gmake scoreboard`, `tools/progress.py --verbose`), consistent with the
"Derived numbers are recomputed, never remembered" rule already in
`CLAUDE.md`.

## Consequences

- `tools/progress.py` needs to (a) read `objdiff`'s per-function match
  percentage or an equivalent bit-identity check per object, not just
  `TEXT_SUBSEGMENTS` membership, and (b) check that object's build recipe
  for any instruction-editing step before crediting a byte range to line 1.
  This is implementation work outside this ADR's scope, but is the concrete
  consequence the next engineering pass on `progress.py`/`scoreboard` must
  satisfy.
- `README.md`'s Progress block gains the five DKR-style rows in place of the
  current undifferentiated figure once that work lands; `gmake
  check-scoreboard` continues to fail on drift between the README block and
  what the tree produces.
- Immediate numeric effect, per ADR 0001: matched overlay C drops from
  276,600 bytes to at most 100,876 bytes pending per-object review of the
  274 affected functions.
