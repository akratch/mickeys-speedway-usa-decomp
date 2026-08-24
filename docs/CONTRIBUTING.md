# Contributing

## Set up once

```sh
gmake setup
```

That bootstraps the venv, installs dependencies, copies the toolchain, verifies
your baserom's SHA1, splits the ROM, and points git at `.githooks/`, which is
what activates the clean-room gates. `core.hooksPath` is per-clone
configuration, not a tracked file, so **every fresh clone needs this once**.
`gmake hooks` does that step alone.

Full build instructions are in [`README.md`](../README.md).

## The clean-room rule

Nothing ROM-derived is ever tracked in git: no disassembly, no instruction
text, no hexdumps or byte arrays of ROM bytes, no extracted assets, no ROM
images, no decompilation-workbench ledgers. [`CLEANROOM.md`](CLEANROOM.md) is
the policy: what may be consulted, what may be adopted, and how adopted
material is disclosed. Read it before taking a name or a function body from
another project.

It has gone wrong once: two workbench `ledger.jsonl` files reached the remote
with the ROM's own disassembly inside their diff-site records, and the fix was
a history rewrite. The gates below exist so that cannot recur.

## The gates

| Where | What it scans |
|---|---|
| `.githooks/pre-commit` | the index, exactly what the commit would record |
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

**Server-side, on `master`:** the GitHub ruleset `protect-master`
(id `20111399`, active) blocks force-push (`non_fast_forward`) and branch
deletion. That is the one layer that cannot be stepped over with `--no-verify`,
but it guards the *branch*, not the *content*: GitHub rejected a push ruleset
restricting by file path, extension or size with "only org-owned repos can have
push rules", which this personal fork is not. A required status check on
`master` remains the only way to block a bad push before it lands rather than
after, and would mean routing changes through pull requests; that workflow
change has not been made. One consequence: purging content from `master`'s
history again means disabling `protect-master` first and re-enabling it after.

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

They do **not** catch:

| Hole | Measured limit |
|---|---|
| a padded sub-threshold trickle | the per-file word limit is 192, so one file carrying **191 machine words, about 764 bytes of ROM**, passes if it is padded enough to keep the aggregate's 12-words/KiB rate floor from counting it; eight such files carry about **6 KB** with the aggregate budget reading zero |
| digest-shaped hex strings | the first 64 per file are exempt; **87 digests, about 2.7 KiB per file** on the measured region, up to 141 / ~4.4 KiB on others, since what trips is the spread of the words they decode to |
| fewer than 64 hex halves pairs in a file | 63 pairs, about 252 bytes, is not caught *by that route* (the fixture is still caught by the layout rules) |
| deliberate steganography | undecidable in general |

Detecting arbitrarily-encoded data is undecidable, and these rules are
calibrated for mistakes rather than adversaries. [`CLEANROOM.md`](CLEANROOM.md)
lists the limits with their measurements, and names what is load-bearing: the
path whitelist and manifest schema check, the ROM path and binary rules, the
tool-level ledger redaction, this policy, and `protect-master`.

**Do not use `--no-verify`**, and do not lower a threshold to get a file
through. If a file is a real false positive, restructure it or add an
allowlist entry in `tools/cleanroom_detectors.py` with a reason. If something
ROM-derived is already committed, it must be rewritten out of history; a
commit that deletes the file still ships its bytes to anyone who fetches.

## Evidence discipline

Every claim in `docs/modules.md` states how it was established, using the four
tiers in its §1: **A** byte-identity, **B** call graph, **C** string
correspondence, **D** structural inference. Declare the tier inline, per
symbol, in both `docs/modules.md` and `symbol_addrs.us.txt`. Tier A has an
adoption threshold (§1.2); an adoption below it is argued individually in that
section's table.

Derived numbers (matched function and byte counts, percentages, segment
sizes) are recomputed from the lists they summarise, never copied forward.
`gmake check-docs` re-derives the mechanically checkable ones and fails on
drift.

### Overlay donor-first workflow

Before naming or decompiling any overlay function:

1. identify it as `(overlay, section, offset)` from
   `config/overlays.us.json`; the shared synthetic VMA is not a unique runtime
   address;
2. run `gmake overlay-donors-scan-check`, which rechecks every overlay against
   the pinned DKR v77, secondary DKR v80, and JFG object surfaces;
3. consult DKR first for game-code structure and terminology because it is the
   complete closest-lineage decomp, then use JFG where its overlay/runtime
   layout is closer;
4. cite the exact object/name when bytes match, or label the comparison
   semantic when only systems, strings, or call shape correspond. A donor's
   generated placeholder is never an adopted name.

`gmake overlay-donors` is the farm-free integrity check for the committed
107-row-per-donor ledger. The scan-check is the stronger maintainer check and
requires the out-of-tree reference builds described in `references.md`.

## Checks

**Before committing, at minimum:** `gmake verify && gmake cleanroom && gmake check-docs`.
`cleanroom` also runs automatically at commit/push if `gmake hooks` has been
run; nothing else is wired into a hook.

| Command | Checks | Needs a build? | Enforced by |
|---|---|---|---|
| `gmake verify` | ROM rebuilds byte-identically (SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`) | yes | manual |
| `gmake cleanroom` | no ROM-derived content in the worktree/index/range | no | pre-commit, pre-push, CI |
| `gmake check-docs` | derived numbers in the docs (`docs/modules.md` etc.) match the tree | no† | manual |
| `gmake check-scoreboard` | README's generated Progress block matches what the tree produces right now | yes | manual only; CI runs the `--check-partial` subset, which does not need a build (see [`scoreboard.yml`](../.github/workflows/scoreboard.yml) and the note below) |
| `gmake audit-decoders` | the clean-room content detectors aren't inventing words that aren't there (run after touching `tools/cleanroom_detectors.py`, not instead of `cleanroom`) | no | manual |
| `gmake check-fixtures` | the other direction: real ROM in every encoding at every wrap width is *still caught*, which `audit-decoders` is structurally blind to. Fixtures are synthesized from `baseroms/mickey.us.z64` at run time and never written to disk, so it can never run in CI. Run it with `audit-decoders`, not instead of it | no (needs a baserom) | manual |
| `gmake check-reference-builds` | a local reference-decomp farm still hashes to the digests `tools/reference-builds.lock` pins, i.e. is the farm the 190 tier-A names were mined from. Needs the farm, which is out of tree by design, so it can never run in CI. `gmake reference-builds` rebuilds one from the pins; see [`references.md`](references.md) | no (needs the farm and its baseroms) | manual |
| `gmake overlay-tables` | decodes the four overlay ROM blocks and re-asserts the layout `docs/modules.md` §5.3 states: the reloc count word, 370 of 375 call sites holding a real `jal`, the 107-fold module gap arithmetic. Needs a baserom, so it can never run in CI | no (needs a baserom) | manual |
| `gmake overlay-atlas` | regenerates the canonical 107-module manifest and 106 generated yaml code segments in memory and fails on drift | no (needs a baserom) | manual |
| `gmake overlay-donors` | validates complete DKR v77/v80 and JFG results for all 107 overlays against pinned metadata; does not need the farm | no | `check-docs` |
| `gmake overlay-donors-scan-check` | reruns those object comparisons and fails if the committed donor ledger differs | no (needs the reference farm and Mickey baserom) | manual before overlay adoption |
| `gmake prune-asm` | deletes the `asm/` files splat orphaned: `asm/<tu>.s` for a subsegment that is now `c`, and any `asm/nonmatchings/` file no `#pragma GLOBAL_ASM` names. Run by every split, so it is rarely invoked by hand; without it `gmake progress` under-reports | no | every `extract`/split |
| `gmake progress` | the same matched-function/byte/symbol counts as the scoreboard, without touching README.md | yes | manual |
| `gmake scoreboard` | regenerates README's Progress block from the tree (run it, then commit, whenever matching progress changes) | yes | manual |
| `gmake clean` | removes `build/` only | no | manual |
| `gmake distclean` | `clean`, plus the *extracted* state: `asm/`, `assets/`, the linker script and the auto-generated `undefined_*.us.txt`. Recovering needs `gmake extract`, which needs a baserom | no | manual |

† `check-docs`'s jump-table count needs `asm/` and is skipped, not failed,
before `gmake extract` has run.

**Why `check-scoreboard` can't be a full CI job.** It needs a linked ELF, and
producing one needs `gmake extract` to split `asm/` out of a baserom, which
[`CLEANROOM.md`](CLEANROOM.md) forbids committing. `--check-partial` (wired
into [`scoreboard.yml`](../.github/workflows/scoreboard.yml)) is the strongest
subset that runs without one: it recomputes the block's two non-ELF-derived
figures (the adopted-symbol count from `symbol_addrs.us.txt` and the
matched-TU list from `src/`) using the real generator, and separately checks
the committed block's own arithmetic (each ratio's percentage against its own
numerator/denominator, the per-area rows summing to the total row). It cannot
catch drift in the ELF-derived figures: functions/bytes matched, the per-area
breakdown. Only `gmake check-scoreboard`, run locally by whoever matched the
function, catches that.

```sh
gmake verify            # byte-identical: 507341c0a40ca3e9a7cee969b396ee53facfb548
gmake cleanroom         # exit 0
gmake check-docs        # derived numbers still true
gmake check-scoreboard  # README's Progress block still matches the tree
git status              # clean
```
