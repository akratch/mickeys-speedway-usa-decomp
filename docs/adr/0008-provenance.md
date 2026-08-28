# 0008. Provenance

Status: Accepted
Date: 2026-08-24

## Context

The standing rule (`CLAUDE.md`, `docs/CLEANROOM.md`) is that this project
follows the same provenance practice as the existing published N64 matching
decomps. `docs/acceleration-survey.md` §13.3 and §13.4 checked what that
practice actually is, across DKR, JFG, BK, PD, and dp64.

**SDK library source.** DKR ships 108 files carrying the "Copyright …
Silicon Graphics" legend (76 with the "unpublished proprietary" paragraph)
under `libultra/src/{audio,gu,io,libc,os,sc}`, in a CC0 repository, with no
provenance statement anywhere near them. JFG has 64 such files, BK 210 (via
a submodule plus `include/n_audio/`), PD 74 under MIT, dp64 30. This is
universal practice across every gold-standard project, and
`docs/CLEANROOM.md` already permits "library source as distributed in
existing public decomp projects"; this finding confirms the existing
policy rather than changing it.

**Naming source.** None of the five projects documents a formal naming
process; DKR hand-curates `symbol_addrs.*.txt` with inline "Official Name:"
comments; JFG tracks two full symbol dumps. None of the five carries a
written leak policy, and the community norm in practice treats leaked
*source code* as off-limits while a dumped *build* is treated as an
ordinary ROM. dp64's own README describes its baserom neutrally: "Dinosaur
Planet … as released by Forest of Illusion on Feb. 20, 2021," not as a
retail release.

`docs/CLEANROOM.md`'s dp64 exception currently justifies itself by saying
dp64 is "built from a retail ROM, not the leaked build." That premise is
factually wrong: there is no retail Dinosaur Planet release. dp64 is built
from the 2000-12-01 development-cartridge dump, the same dump Forest of
Illusion released on 2021-02-20. The exception's *scope*, binary
file-format facts only (reloc-table layout, field offsets, table
structure), never names, code, or comments, does not depend on that
premise and is unaffected by correcting it.

Separately, under the "same provenance as the existing projects" standing
rule, a public decomp of a *publicly dumped* build reads, in every respect
that matters to the decomp community, as a published retail-derived decomp:
debug-build symbols from a dumped build are exactly how OoT, Majora's
Mask, and Paper Mario were named elsewhere in the community. That would
make dp64's 2,291 names, and the `sfadebug` (Star Fox Adventures debug
build) symbol set, admissible on the same terms as DKR/JFG/BK/PD. But
`docs/CLEANROOM.md` currently prohibits both by name, and §13.4 is explicit
that changing that is a policy choice for the project owner to make in
writing, not something decided mid-campaign by an agent reading the survey.

## Decision

1. Adopt the standard of the five named published decomps for SDK library
   source: files carrying the SGI legend, as distributed in DKR, JFG, BK,
   or PD, are permitted, with a `PROVENANCE` note at the point of use
   (unblocking `n_audio`, ADR 0005 item 2).
2. `PROVENANCE` notes remain required at the point of use for any adapted
   name or function body, exactly as `docs/CLEANROOM.md` already states.
3. decomp.me is acceptable to use per the project owner, but unnecessary
   in an agent-driven loop, since the permuter, coddog, and objdiff all run
   locally against `tools/ido/cc` (ADR 0007).
4. `docs/CLEANROOM.md`'s dp64 exception is corrected on its **factual
   premise only**: dp64 is built from the 2000-12-01 development-cartridge
   dump released by Forest of Illusion on 2021-02-20, not from a retail
   ROM. The exception's **scope is unchanged**: binary file-format facts
   only, never names, code, or comments.
5. **dp64 names and `sfadebug` symbols remain prohibited** until the
   project owner rewrites `docs/CLEANROOM.md` to say otherwise, in writing.
   This ADR records the open question and the argument on both sides; it
   does not resolve it. No agent may treat this ADR, or the survey it
   summarizes, as authorization to adopt a dp64 or `sfadebug` name.

## Consequences

- `docs/CLEANROOM.md`'s dp64 paragraph is edited to state the dump's actual
  provenance (development-cartridge dump, 2000-12-01, released by Forest of
  Illusion 2021-02-20) in place of "built from a retail ROM." What is
  permitted and prohibited in that paragraph does not change.
- `n_audio` (45 TUs, 50 KB, matched donor C in PD and BK) is unblocked and
  is ADR 0005's top-priority resident target.
- dp64 and `sfadebug` stay off-limits for names/code/comments pending a
  future, explicit CLEANROOM.md rewrite by the project owner; dp64's
  tooling (`elf2dll.py`, `dll_split.py`, `dllsyms2ld.py`, `dllimports.py`,
  `dlldiff.py`) may be consulted for the structural/format facts they
  encode; their code is not adopted into this tree.
