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
a barrier against a determined bypass. A server-side GitHub push ruleset or a
required status check on a protected branch would be the first layer that
cannot be skipped; this repository does not have one yet.

Run it yourself before committing:

```sh
gmake cleanroom                                   # the worktree
gmake cleanroom CLEANROOM_ARGS=--staged           # the index
gmake cleanroom CLEANROOM_ARGS="--range A..B"     # a commit range
```

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

## Every commit must hold the line

```sh
gmake verify      # byte-identical: 507341c0a40ca3e9a7cee969b396ee53facfb548
gmake cleanroom   # exit 0
gmake check-docs  # derived numbers still true
git status        # clean
```
