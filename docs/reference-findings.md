# Practices observed in reference projects

This note records the established decompilation practices that informed the
project's ADRs. It is background, not policy. The accepted decisions under
`docs/adr/` are authoritative.

## Matching

Diddy Kong Racing reports five code states: matched C, original hand-written
assembly, extracted assembly, `NON_MATCHING`, and `NON_EQUIVALENT`. Its scoring
counts the two conditional C states as unmatched because their assembly
fallback still supplies the ROM bytes.

asm-differ, objdiff, and the surveyed projects use the same practical rule: a
match is compiler output that is identical at the correct linked location.
Project-level ROM verification is separate. This supports
[ADR 0001](adr/0001-matching-standard.md) and
[ADR 0003](adr/0003-scoreboard.md).

## Post-compile operations

The surveyed builds modify ELF flags, symbol tables, relocation metadata,
section padding, and checksums. They do not edit compiled instruction words to
turn a non-match into a match. This supports
[ADR 0002](adr/0002-no-post-compile-instruction-editing.md).

## Provenance

The permitted retail-ROM projects publish libultra and other SDK-derived
source as part of their decompilations. This project may use those materials
with the disclosure required by the [clean-room policy](CLEANROOM.md).

The public *Dinosaur Planet* project is based on a development-cartridge dump.
Only its documentation of the overlay file format may be consulted here. Its
names and code are outside the permitted source set. See
[ADR 0008](adr/0008-provenance.md).

## Source layout

Diddy Kong Racing groups functions by original translation unit. Jet Force
Gemini groups each overlay under its own directory and shared headers. Neither
uses one permanent C file per function. This supports the consolidation rule
in [ADR 0006](adr/0006-overlay-source-layout.md).

## Reference search

Exact object matching works well for libultra and unchanged shared engine
code. A masked instruction skeleton can also find related functions after
register allocation, constants, or linked addresses change. In the measured
reference set, resident code has far more useful structural candidates than
overlay code. This supports the ordering in
[ADR 0005](adr/0005-work-prioritisation.md).

Jet Force Gemini also supplies overlay imports, exports, and function offsets.
Those records can help compare module roles even when code bytes differ. Such
comparisons cover object management, computer-controlled behavior, paths,
particles, and audio, but remain structural evidence until Mickey's own call
graph confirms a name.

## Tools

The common tools are asm-processor, asm-differ, m2c, decomp-permuter, objdiff,
and project-specific progress scripts. The useful division is:

- exact-object and skeleton search for candidate discovery;
- compiler-flag comparison before source mutation;
- asm-differ or objdiff for byte and relocation comparison; and
- bounded permutation only after a readable C candidate exists.

[ADR 0007](adr/0007-matching-tools.md) records the project's tool policy.
