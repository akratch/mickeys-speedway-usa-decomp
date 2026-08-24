# 0007. Matching tools

Status: Accepted
Date: 2026-08-24

## Context

`docs/acceleration-survey.md` §3 and §4 found the tree had accumulated a
parallel, hand-rolled toolchain in place of standard decomp tooling:
per-function brute-forcers
(`nonmatchings/*/search_{shapes,loops,stack,locals,layouts,body_lines}.py`,
each an `itertools.product` over hand-listed C spellings) and the
object-editing machinery ADR 0002 retires, alongside about 700 `.tmp-o*`
objects at the repo root. Meanwhile `decomp-permuter` was checked out on
disk but not installed into `.venv` and referenced by nothing;
`tools/permuter_settings.toml` pointed at a compiler path that doesn't
exist. `tools/find_known_objects.py`, the one exact-match tool in use,
masks relocated words and anchors on the longest fixed run — sound for the
libultra corridor (it found 190 names there) but structurally blind to any
function whose constants, register coloring, or call targets shifted, which
is every function in a differently-revised source tree; against the
overlays it returned "none" for 104 of 107 against DKR and 96 of 107
against JFG.

§3 identified three tools built for exactly this gap, none in use: coddog
(exact/opcode+operand/opcode-only hashing, bounded Levenshtein over opcode
sequences, a cross-project index), objdiff 3.8.0 (per-object match
percentage, "find similar functions," a JSON one-shot CLI), and the
survey's own `scratchpad/fingerprint.py`, a 60-line masked-skeleton scanner
that already produced the ADR 0005 donor numbers.

§4 recorded a caution from a comparable project (Snowboard Kids 2, 100% in
2026-05): the permuter was removed from the *agent* reasoning loop because
agents mistook permuter artifacts (`do{}while(0)`, nested assignments) for
genuine signal. The pattern that worked there: agent proposes a typed,
structurally plausible candidate; the permuter runs as a bounded batch job
against it; a human-readable diff of the winning mutation comes back to the
agent, which rewrites it idiomatically rather than keeping the mutation
verbatim.

## Decision

- **decomp-permuter**, installed into `.venv` (not left as an unreferenced
  checkout), run only as a **bounded batch job** — never inside an agent's
  own reasoning loop. An agent may hand it one candidate and a time/attempt
  budget and read back a diff; it may not run it as a tool it iterates on
  turn-by-turn.
- **objdiff-cli** is the per-object oracle: the source of match percentage
  and first-mismatch information that `tools/progress.py` and per-function
  work both read, replacing ad hoc scoring.
- **coddog** and (until it's indexed) `skeleton_scan.py` (the survey's
  `fingerprint.py`, promoted with `--emit-symbols`) are the near-match
  oracle: what finds a donor whose bytes changed but whose structure
  didn't, which `find_known_objects.py` cannot do.
- A **flag-lattice sweep** (compile one natural candidate under the full
  set of known flag groups, rank by objdiff score) runs before any hand
  permutation is attempted on a function — permutation is for closing a
  near-miss the flag lattice couldn't, not a first move.
- The hand-rolled `search_*.py` brute-forcers and the ELF-surgery tools
  ADR 0002 already retires are retired here too, as tools, not just as
  permitted build steps.

## Consequences

- `tools/permuter_settings.toml` is repointed at `tools/ido/cc`.
- The 274 functions ADR 0001/0002 move to `NON_MATCHING` are the permuter's
  first batch: `-j` sized to the machine, `--stop-on-zero`, a bounded
  per-function time cap. Every exact result deletes an `.ops`-style
  artifact and moves bytes from "normalized" to genuinely matched.
- Indexing the reference farm (DKR, JFG, PD, BK, Conker, dp64) into coddog
  is follow-up engineering work, not something this ADR performs.
- This ADR does not change matching standards (ADR 0001/0002): every
  permuter or coddog result is still just a candidate to be compiled and
  byte-compared, never accepted on similarity alone.
