# Matching tools

This page is an index. [ADR 0007](adr/0007-matching-tools.md) defines how the
tools are used in the matching process.

## Required build tools

| Tool | Purpose |
|---|---|
| splat | Split the ROM and generate the linker script |
| asm-processor | Compile C files that still contain `GLOBAL_ASM` includes |
| IDO 5.3 | Reproduce the original compiler output |
| GNU binutils | Assemble, link, inspect ELF files, and extract sections |
| `n64crc` | Write the Nintendo 64 header checksum |

`gmake setup` initializes the Python environment, required submodules,
toolchain, extraction, and hooks.

## Daily comparison tools

| Tool | Purpose | Documentation |
|---|---|---|
| `diff.sh` / asm-differ | Compare target and current functions | Upstream asm-differ help |
| `mips_to_c.sh` | Produce an initial C translation | Upstream m2c help |
| `tools/wb_compare.sh` | Build a target object and diagnose the current candidate | Script help and workbench guide |
| `tools/function_preflight.py` | Prove one function's ownership and evidence surface | Function evidence preflight below |
| `tools/allocator_trace_receipt.py` | Map and summarize faithful IDO allocator traces | [Allocator trace receipts](allocator-trace-receipts.md) |
| `tools/flag_sweep.py` | Rank known compiler flag groups | [Flag sweep](flag-sweep.md) |
| `tools/skeleton_scan.py` | Find structural reference candidates | [Skeleton search](skeleton-scan.md) |
| `tools/nm_ranking.py` | Rank guarded non-matching functions | [Non-matching ranking](nm-ranking.md) |
| `tools/permute_batch.py` | Run bounded source permutation | [Bounded permutation](permute-batch.md) |

Use the standard order: find a plausible reference, compile a natural C
candidate, test known flags, diagnose the mismatch, and use bounded permutation
only for a narrow remaining compiler difference.

## Function evidence preflight

Run the evidence preflight before a flag sweep or allocator experiment:

```sh
tools/function_preflight.py overlay16ApplyGradient
tools/function_preflight.py func_overlay_016_F00001E0_1873678 --json
```

Either the friendly C name or splat's generated overlay name resolves to the
same identity. The command incrementally builds the canonical linked ELF and
the correct full-TU candidate object, automatically selecting
`build_non_matching/` for a guarded candidate. Split and target builds run as
separate phases. Before comparison, both the full-TU object and canonical ELF
must be current according to Make's dependency graph; the Makefile and checked
normalization fragments receive an explicit timestamp check because recipe
changes do not automatically age outputs. `--no-build` and `wb_compare.sh`
fail closed with a rebuild diagnosis when evidence is stale. The report covers
the owned range, next ownership or padding boundary, exports, inbound call
sites, and candidate ABI and frame. For overlays it reports shipped runtime
relocation records. For resident functions it reports authenticated static
relocation tuples separately from sparse startup-table records, where zero
runtime records can be valid. It then reports candidate static-surface
agreement plus the current word score and first mismatch. Ambiguous aliases,
sources, ranges, or relocation identities are errors. Output excludes
instruction listings, words, and hexdumps.

Promotion does not make the preflight unusable when splat removes the
function's extracted fallback. With no fallback present, the resolver enters
`post_promotion` mode only for one unconditional requested C definition with
no matching `GLOBAL_ASM`. It requires tracked exact ownership in the overlay
atlas or resident symbol table, and that evidence must agree with the unique
definition plus the linked ELF's value and size. A missing fallback cannot
promote a guarded `NON_MATCHING` body. Post-promotion uses the ordinary
`build/` object and the fully relocated ROM as its byte oracle; it also
requires the candidate and target relocation counts, offsets, and types to
agree. JSON reports distinguish this route with `resolution_mode` and
`workbench.comparison_mode`.

`tools/wb_compare.sh` uses the same resolver, so manual candidate-symbol and
build-directory settings are unnecessary for normal guarded functions:

```sh
tools/wb_compare.sh overlay16ApplyGradient --json
tools/wb_compare.sh --diagnose overlay16ApplyGradient --trace trace.log --trace-proc 0
```

Wrapper options precede the symbol. Arguments after the symbol pass unchanged
to `decomp-workbench compare` or, with `--diagnose`, to
`decomp-workbench diagnose`. `--rom` retains the linked-ROM final oracle.
Invoke `wb_compare.sh --rom <linked-C-name>` directly after promotion;
`function_preflight.py` selects it automatically after its tracked
post-promotion checks pass.

For a compact post-promotion receipt, run:

```sh
gmake promotion-proof SYMBOL=overlay41SpawnItems
```

`tools/promotion_proof.py` consumes the preflight JSON rather than duplicating
its resolver. It requires post-promotion mode, linked-ROM comparison, exact
words and frame, and exact relocation shape and effective identities. Add
`PROMOTION_PROOF_ARGS=--canonical` to also run the bounded full-ROM and overlay
relocation-surface proofs. `--no-build` is available when the evidence graph is
already fresh.

## Overlay tools

| Tool | Purpose | Documentation |
|---|---|---|
| `tools/overlay_tables.py` | Decode runtime table structure | [Overlays](overlays.md) |
| `tools/overlay_atlas.py` | Generate the canonical overlay map and YAML projection | [Overlays](overlays.md) |
| `tools/overlay_donor_scan.py` | Compare all overlays with locked reference objects | [Reference builds](references.md) |
| `tools/overlay_graph_match.py` | Rank structural JFG module correspondences | [Overlay graph](overlay-graph.md) |
| consolidation helpers | Maintain grouped overlay source ownership | [Overlay consolidation](overlay-consolidation.md) |

### Overlay atlas release deltas

`python3 tools/overlay_atlas.py --delta` compares exact-C overlay ownership
without rebuilding or checking out an old tree:

```sh
python3 tools/overlay_atlas.py --delta HEAD^
python3 tools/overlay_atlas.py --delta release-base release-candidate
python3 tools/overlay_atlas.py --delta old-atlas.json ../candidate-checkout
python3 tools/overlay_atlas.py --delta release-base release-candidate --format json
```

A state may be a Git tree-ish, atlas JSON file, or checkout containing
`config/overlays.us.json`. The report lists promotions and retractions,
individual ranges, gross byte totals, and net exact-C change. Identity is
always `(overlay, text, offset)`, never a shared synthetic VMA or filename.
Duplicate or overlapping identities, inconsistent extents/totals, and
same-key extent changes fail closed.

## Deterministic public-release reconciliation

`tools/public_release.py` is a push-incapable final preflight for a checked-out
publication branch. The branch and remote are explicit:

```sh
gmake public-release \
  PUBLIC_RELEASE_ARGS="--remote public --branch master"
```

It confirms fast-forward ancestry, checks derived artifacts, computes exact
scoreboard and overlay-atlas deltas, runs the serial release gates, and scans
the resulting tree plus every outgoing commit tree and message. This catches
operator-only paths or private text even if a later outgoing commit deletes
them. It never fetches, merges, copies between checkouts, or pushes.

When generated files need refreshing, add `--write-derived`. Write mode calls
only the documented overlay-atlas, atlas-digest, post-process, overlay-symbol,
and scoreboard generators and leaves their output unstaged. Review and commit
those changes, then rerun the clean default dry run.

Maintainers with the out-of-tree reference farm can add
`--check-reference-builds`. This opt-in check runs the lock-pinned reference
verifier before reconciliation generators, failing early on a missing, stale,
or divergent DKR/JFG farm. It is off by default because ordinary contributors
do not need the external farm, and it honors the verifier's `REFS_ROOT`
override.

## objdiff

`tools/setup_objdiff.sh` downloads `objdiff-cli` into the ignored
`tools/objdiff/` directory. `tools/gen_objdiff_config.py` generates
`objdiff.json` from the current build. `tools/objdiff_report.sh` produces a
report against an expected object tree.

Some metadata-trimmed objects are not accepted by objdiff's ELF reader. The
report script regenerates an ignored exclusion list from Makefile
`POSTPROCESS` rules. Use `tools/wb_compare.sh` for a function that is absent
from the batch report.

## decomp-permuter

Place a local decomp-permuter checkout at `tools/permuter` or link that path to
the checkout. The directory is ignored. `tools/permuter_settings.toml` contains
the compiler and assembler commands used by the wrapper scripts.

Do not commit imported candidates, permuter work directories, or target
assembly. Review every selected mutation as C source before promotion.

## Local outputs

The following stay untracked:

- ROMs, extracted assembly, and assets;
- compiler and binutils installations;
- expected object snapshots;
- objdiff reports and exclusions;
- flag-sweep, workbench, ranking, and permuter directories; and
- all raw comparison or attempt logs.

Run `gmake cleanroom` before committing. A generated report may contain target
data even when its summary looks harmless.
