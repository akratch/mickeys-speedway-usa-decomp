# Contributing

## Set up once

```sh
gmake setup
```

That bootstraps the venv, installs dependencies, copies the toolchain, verifies
your baserom's SHA1, splits the ROM — and points git at `.githooks/`, which is
what activates the clean-room gates. `core.hooksPath` is per-clone
configuration, not a tracked file, so **every fresh clone needs this once**. If
you only want the gates, `gmake hooks` does that step alone.

Full build instructions are in [`README.md`](../README.md).

## The clean-room rule

This project aims to be publishable. That depends entirely on nothing
ROM-derived ever being tracked in git: no disassembly, no instruction text, no
hexdumps or byte arrays of ROM bytes, no extracted assets, no ROM images, no
decompilation-workbench ledgers. [`CLEANROOM.md`](CLEANROOM.md) is the policy —
what may be consulted, what may be adopted, and how adopted material is
disclosed. Read it before taking a name or a function body from another
project.

It has gone wrong once already. Two workbench `ledger.jsonl` files reached the
remote with the ROM's own disassembly quoted inside their diff-site records —
enough to reconstruct most of a function — and the fix was a history rewrite.
The gates below exist so that cannot recur.

## The gates

| Where | What it scans |
|---|---|
| `.githooks/pre-commit` | the index — exactly what the commit would record |
| `.githooks/pre-push` | every commit tree in the push, not just the tip |
| `.github/workflows/cleanroom.yml` | the same, on every push and pull request |

All three run `tools/cleanroom_check.sh`, which looks for ROM/asset paths,
tracked files under `.decomp-workbench/` other than a campaign manifest,
non-text blobs, oversized files, MIPS instruction text, bare machine-word
runs, machine words written adjacently or spread across the 32-bit space in
any encoding, hexdump-shaped lines, base64 blobs by longest run and by volume,
and an aggregate word budget across each tree. Thresholds are measured against
this repository's whole history on one side and the purged ledgers plus an
evasion fixture set on the other; the numbers and their margins are in
`tools/cleanroom_detectors.py`.

The first two layers are client-side and the third is after the fact: hooks
are per-clone configuration, `--no-verify` steps over them, and by the time CI
speaks the objects are already published. They are depth against mistakes, not
a barrier against a determined bypass.

**Server-side, on `master`:** a GitHub ruleset named `protect-master`
(id `20111399`, active) blocks force-push (`non_fast_forward`) and branch
deletion. That is the one layer above that cannot be stepped over with
`--no-verify` — but it guards the *branch*, not the *content*: GitHub rejected
a push ruleset restricting by file path, extension, or size with "only
org-owned repos can have push rules", which this personal fork is not. A
required status check on `master` remains the only way to block a bad push
before it lands rather than after, and would mean routing changes through
pull requests instead of pushing to `master` directly; that workflow change is
a maintainer decision and has not been made. One consequence of blocking
force-push worth knowing: if content ever needs purging from `master`'s
history again (as happened once already — see above), `protect-master` has to
be disabled first, then re-enabled after the rewrite.

Run it yourself before committing:

```sh
gmake cleanroom                                   # the worktree
gmake cleanroom CLEANROOM_ARGS=--staged           # the index
gmake cleanroom CLEANROOM_ARGS="--range A..B"     # a commit range
```

### What they catch, and what they don't

Measured against 400-word real-ROM fixtures, the content rules catch asm
listings, every hexdump format, C arrays (including `u`/`UL` suffixes and
underscore separators), machine words in prose or hex ranges or 16-bit halves
or octal or decimal, escaped byte strings, base64/base64url/base32/ascii85
(wrapped, or split across files), JSON ledgers, and leaks spread thinly across
a tree.

They do **not** catch a sub-threshold trickle — the per-file word limit is 192,
so **one file carrying up to 191 machine words, about 764 bytes of ROM, passes
everything** if it is padded enough to keep the aggregate's 12-words/KiB rate
floor from counting it; eight such files carry about **6 KB** with the aggregate
budget still reading zero. Nor do they catch digest-shaped hex strings up to the
point where the exempted first 64 stop covering them — **measured at 87 digests,
about 2.7 KiB per file**, and higher on other ROM regions (up to 141 / ~4.4 KiB),
since what actually trips is the spread of the words they decode to. Nor
deliberate steganography. Detecting
arbitrarily-encoded data is undecidable, and these rules are calibrated for
mistakes rather than adversaries. [`CLEANROOM.md`](CLEANROOM.md) lists the
limits with their measurements — including a separate volume floor on 16-bit
halves, where a file of fewer than 64 hex halves pairs (63 pairs, about 252
bytes) is not caught *by that route*, though the fixture is still caught by the
layout rules — and names what is actually load-bearing: the
path whitelist and manifest schema check, the ROM path and binary rules, the
tool-level ledger redaction, this policy, and the `protect-master` ruleset
described above (force-push/deletion only — it cannot restrict content, see
above for why).

**Do not use `--no-verify`**, and do not lower a threshold to get a file
through. If a file is a real false positive, restructure it or add an
allowlist entry in `tools/cleanroom_detectors.py` with a reason. If something
ROM-derived is already committed, it must be rewritten out of history — a
commit that deletes the file still ships its bytes to anyone who fetches.

## Evidence discipline

Every claim in `docs/modules.md` states how it was established, using the four
tiers in its §1: **A** byte-identity, **B** call graph, **C** string
correspondence, **D** structural inference. Declare the tier inline, per
symbol, in both `docs/modules.md` and `symbol_addrs.us.txt`. Tier A has an
adoption threshold (§1.2); an adoption below it is argued individually in that
section's table rather than waved through.

Derived numbers — matched function and byte counts, percentages, segment sizes
— are recomputed from the lists they summarise, never copied forward.
`gmake check-docs` re-derives the mechanically checkable ones and fails on
drift.

## Checks

**Before committing, at minimum:** `gmake verify && gmake cleanroom && gmake check-docs`.
`cleanroom` also runs automatically at commit/push if `gmake hooks` has been
run (see above) — the rest are yours to run by hand; nothing else is wired
into a hook.

| Command | Checks | Needs a build? | Enforced by |
|---|---|---|---|
| `gmake verify` | ROM rebuilds byte-identically (SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`) | yes | manual |
| `gmake cleanroom` | no ROM-derived content in the worktree/index/range | no | pre-commit, pre-push, CI |
| `gmake check-docs` | derived numbers in the docs (`docs/modules.md` etc.) match the tree | no† | manual |
| `gmake check-scoreboard` | README's generated Progress block matches what the tree produces right now | yes | manual only — CI runs a **partial** subset, `--check-partial`, that does not need a build (see [`scoreboard.yml`](../.github/workflows/scoreboard.yml) and the note below) |
| `gmake audit-decoders` | the clean-room content detectors aren't inventing words that aren't really there (run this after touching `tools/cleanroom_detectors.py`, not instead of `cleanroom`) | no | manual |
| `gmake check-fixtures` | the other direction — real ROM in every encoding at every wrap width is *still caught*, which `audit-decoders` is structurally blind to. Fixtures are synthesized from `baseroms/mickey.us.z64` at run time and never written to disk; needs a baserom, so it can never run in CI. Run it with `audit-decoders`, not instead of it | no (needs a baserom) | manual |
| `gmake progress` | prints the same matched-function/byte/symbol counts as the scoreboard, without touching README.md | yes | manual |
| `gmake scoreboard` | regenerates README's Progress block from the tree (run this, then commit, whenever matching progress changes) | yes | manual |
| `gmake clean` | removes `build/` only — safe, and all a rebuild normally needs | no | manual |
| `gmake distclean` | `clean`, plus the *extracted* state: `asm/`, `assets/`, the linker script and the auto-generated `undefined_*.us.txt`. Recovering from it needs `gmake extract`, which needs a baserom | no | manual |

† `check-docs`'s jump-table count needs `asm/` and is skipped, not failed,
before `gmake extract` has run.

**Why `check-scoreboard` can't be a full CI job.** It needs a linked ELF
(`gmake progress`'s and `gmake scoreboard`'s comments explain why), and
producing one needs `gmake extract` to split `asm/` out of a baserom — which
[`CLEANROOM.md`](CLEANROOM.md) forbids ever committing. CI checks out a bare
tree with no `baseroms/`, so it can never build the ELF this check depends on;
there is no workaround that doesn't mean shipping a ROM. `--check-partial`
(wired into [`scoreboard.yml`](../.github/workflows/scoreboard.yml)) is the
strongest honest subset: it recomputes the block's two non-ELF-derived
figures — the adopted-symbol count from `symbol_addrs.us.txt` and the
matched-TU list from `src/` — using the real generator, and separately checks
the committed block's own arithmetic (each ratio's percentage against its own
numerator/denominator, the per-area rows summing to the total row). It
**cannot** catch a scoreboard that has drifted on the ELF-derived figures
themselves — functions/bytes matched, the per-area breakdown — because that
needs the build CI cannot produce. Only `gmake check-scoreboard`, run locally
by whoever matched the function, catches that; there is no way to move it into
CI without committing a ROM.

```sh
gmake verify           # byte-identical: 507341c0a40ca3e9a7cee969b396ee53facfb548
gmake cleanroom         # exit 0
gmake check-docs        # derived numbers still true
gmake check-scoreboard  # README's Progress block still matches the tree
git status              # clean
```
