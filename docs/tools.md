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
`build_non_matching/` for a guarded candidate. It reports the owned range and
next ownership or padding boundary, exports, inbound call sites, candidate ABI
and frame, every authenticated relocation tuple and stable identity,
relocation-surface agreement, and the current word score and first mismatch.
`--no-build` makes existing artifacts a hard requirement. Ambiguous aliases,
sources, ranges, or relocation identities are errors. Output excludes
instruction listings, words, and hexdumps.

`tools/wb_compare.sh` uses the same resolver, so manual candidate-symbol and
build-directory settings are unnecessary for normal guarded functions:

```sh
tools/wb_compare.sh overlay16ApplyGradient --json
tools/wb_compare.sh --diagnose overlay16ApplyGradient --trace trace.log --trace-proc 0
```

Wrapper options precede the symbol. Arguments after the symbol pass unchanged
to `decomp-workbench compare` or, with `--diagnose`, to
`decomp-workbench diagnose`. `--rom` retains the linked-ROM final oracle.

## Overlay tools

| Tool | Purpose | Documentation |
|---|---|---|
| `tools/overlay_tables.py` | Decode runtime table structure | [Overlays](overlays.md) |
| `tools/overlay_atlas.py` | Generate the canonical overlay map and YAML projection | [Overlays](overlays.md) |
| `tools/overlay_donor_scan.py` | Compare all overlays with locked reference objects | [Reference builds](references.md) |
| `tools/overlay_graph_match.py` | Rank structural JFG module correspondences | [Overlay graph](overlay-graph.md) |
| consolidation helpers | Maintain grouped overlay source ownership | [Overlay consolidation](overlay-consolidation.md) |

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
