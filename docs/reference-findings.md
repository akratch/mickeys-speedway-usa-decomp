# Reference-decomp findings

The standing rule for this project is that it follows the same provenance and
matching practice as the published N64 decompilations it draws on: Diddy Kong
Racing (DKR), Jet Force Gemini (JFG), Banjo-Kazooie (BK), Perfect Dark (PD),
and (for file-format facts only) Dinosaur Planet (dp64). This note records what
those projects actually do on the questions that shaped this repository's
policies, and the cross-game measurements that guide where matching effort is
cheapest. The decisions that follow from it live in `docs/adr/`; this file is
the survey behind them, not the policy itself.

Numbers here are point-in-time measurements from the reference builds under
`~/Desktop/dev/decomp-refs/`, not tracked figures, and are not covered by
`gmake check-docs`. Recompute before relying on any of them.

## 1. What counts as matched

DKR's scoring is purely textual: it parses `src/**.c` and `libultra/src/**.c`,
weights every function by its size from the map file, and subtracts every
`GLOBAL_ASM`. Its `WIP_REGEX` rewrites any
`#ifdef NON_MATCHING … #else GLOBAL_ASM … #endif` block back to a bare
`GLOBAL_ASM` before counting, so **`NON_MATCHING` and `NON_EQUIVALENT`
functions count as unmatched**, exactly like extracted assembly. The generated
report shows five lines: decompiled, handwritten ASM, `GLOBAL_ASM` remaining,
`NON_MATCHING`, `NON_EQUIVALENT`. Project-level verification is separate: the
ROM SHA1 check, with compile-only CI jobs for the two escape hatches.

Hand-written original assembly is a first-class, permanently-asm category
(DKR's `src/hasm/*.s`, `hasm_in_src_path: True` in the splat yaml), counted
toward 100% and reported on its own line. Mickey's `verified_asm.us.txt` ledger
is the same idea. This is the basis for
[`docs/adr/0003-scoreboard.md`](adr/0003-scoreboard.md).

## 2. Post-compile object editing

Every object-level build step in DKR, JFG, BK, PD, and dp64 was enumerated.
None of them edits an instruction word after compilation to reach a match. The
steps they do apply touch metadata only:

- DKR/PD/dp64 patch one byte of the ELF header `e_flags` so a `-mips3` object
  links (`patchmips3.py`).
- BK adjusts ELF flags and the symbol table (`set_o32abi_bit.py`,
  `objcopy --prefix-symbols`, `strip`).
- DKR and PD recompute post-link data words the original build also computed:
  the game's own anti-tamper checksums and the header CRC.
- dp64's `elf2dll.py` rewrites relocation metadata and GOT bindings for DLLs
  that still contain `GLOBAL_ASM`, explicitly marked as decomp-only hacks; its
  instruction-level patcher touches only the deliberately non-matching
  static-recomp output.

So the standard is: metadata edits (ELF flags, symbol renames, section
trimming, relocation filter/rebind) are permitted; editing an instruction word
to force a match is not. DKR's own endgame case studies are the proof the hard
cases are solvable the honest way — a global-allocator tie between two float
registers, a temp-register FIFO recovered by splitting one expression, an
instruction-scheduling decision hinging on one missing aliasing fact — all
solved by source restructuring, none by touching the object. This is the basis
for [`docs/adr/0002-no-post-compile-instruction-editing.md`](adr/0002-no-post-compile-instruction-editing.md);
a function reachable only through an instruction-altering step is written as
`#ifdef NON_MATCHING` C over a `#pragma GLOBAL_ASM` fallback and counted as
`NON_MATCHING`, not as matched.

## 3. SDK library source and naming provenance

**SDK library source.** DKR ships 108 files carrying the "Copyright … Silicon
Graphics" legend (76 with the "unpublished proprietary" paragraph) under
`libultra/src/{audio,gu,io,libc,os,sc}`, in a CC0 repository, with no
provenance statement near them. JFG has 64 such files, BK 210, PD 74 under MIT,
dp64 30. This is universal practice across the gold-standard projects, and
`docs/CLEANROOM.md` already permits library source as distributed in existing
public decomp projects. This confirms the existing policy rather than changing
it, and is what unblocks the `n_audio` cohort.

**Naming source.** None of the five projects documents a formal naming process;
DKR hand-curates `symbol_addrs.*.txt` with inline "Official Name:" comments
where one was known, and JFG tracks two full symbol dumps. None carries a
written leak policy; the community norm treats leaked *source code* as
off-limits while a *dumped build* is treated as an ordinary ROM. dp64's own
README describes its baserom as the development-cartridge dump released by
Forest of Illusion on 2021-02-20, not as a retail release — which is why
`docs/CLEANROOM.md`'s dp64 exception is scoped to binary file-format facts only.
This is the basis for [`docs/adr/0008-provenance.md`](adr/0008-provenance.md);
dp64 names and `sfadebug` symbols remain prohibited pending an explicit
`docs/CLEANROOM.md` rewrite.

## 4. Source organization

DKR is one `.c` per original translation unit, with libultra TUs interleaved at
their true ROM offsets in the splat yaml, `migrate_rodata_to_functions: True`,
and asm-processor as the driver for any whole file that still contains
`GLOBAL_ASM`. There is no per-function file anywhere. JFG's overlays are one
`src/overlays/oN/overlay_N.c` each over shared headers. This is the target
layout for Mickey's overlays too: one TU per overlay with shared headers, with
the atlas's `(overlay, offset)` ownership becoming the ordinary splat
subsegment list (see [`docs/adr/0006-overlay-source-layout.md`](adr/0006-overlay-source-layout.md)).

## 5. Cross-game donor measurement

A masked-skeleton fingerprint (opcode plus funct/fmt/regimm field kept;
registers, immediates, and jump targets masked) makes two functions equal when
they are the same source compiled by the same compiler, regardless of register
colouring, addresses, or constants. `tools/skeleton_scan.py` computes it over
the reference builds and searches Mickey's ROM for matches; see
[`docs/skeleton-scan.md`](skeleton-scan.md).

The measurement that motivates the tool: of Mickey's resident game code, about
a fifth has an instruction-skeleton twin in the reference builds, and dozens of
those donors cover functions not yet named. The libultra corridor is far denser
still (roughly four-fifths skeleton-covered). The overlays, by contrast, sit at
the level of an unrelated Rare game — Mickey's overlay code is largely new,
while its resident segment is closest to JFG's engine. An 8-word n-gram kinship
measure confirms the ordering: the resident segment is closer to JFG than DKR
is to JFG, the resident tail is next, and the overlays are last. The practical
consequence is a donor-availability-first ranking of the resident queue: the
functions with a skeleton twin in a reference build are the cheapest bytes in
the ROM to match.

JFG additionally ships module symbol dumps with in-module offsets and a
cross-overlay call graph. Because its overlay code is compiled from a different
revision, bytes will not match, but structure can be cross-referenced: an
overlay's import list, export offsets, entry-function shape, and cross-overlay
edges. Matching Mickey's atlas graph to JFG's names the modules Mickey's
kart-racing rewrite kept from JFG's object, AI, path, particle, and audio-line
systems (`tools/overlay_graph_match.py`, [`docs/overlay-graph.md`](overlay-graph.md)).

## 6. Toolchain notes

The reference decomps use a small, stable toolset: asm-differ, asm-processor,
m2c, decomp-permuter (run from a separate checkout), objdiff, and per-project
scoring scripts. Two toolchain facts are worth carrying forward:

- **IDO 7.1 as a per-TU basin.** PD compiles its naudio/mp3 TUs with IDO 7.1
  `-g`; dp64 keeps both 5.3 and 7.1 available. A 7.1 flag group is a legitimate
  per-TU option to test on functions that resist 5.3, alongside the existing
  per-file flag overrides.
- **A near-match oracle is worth having.** DKR never needed one because it had
  no sibling to mine; Mickey does. The masked-skeleton scan (§5) and objdiff's
  per-function match percentage are the two that fit this tree, the latter
  being the number `tools/progress.py` reads per object rather than a bare
  membership check.
