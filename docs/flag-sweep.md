# Compiler flag sweep

`tools/flag_sweep.py` compiles one translation unit under the compiler groups
known to occur in this project. Run it before changing source only to test
flags.

## Usage

```sh
.venv/bin/python tools/flag_sweep.py src/libultra/sinf.c \
    --function __sinf
```

Useful options:

| Option | Purpose |
|---|---|
| `--target-symbol NAME` | Use a different name for the target assembly |
| `--target-asm PATH` | Assemble an explicit target file |
| `--jobs N` | Set concurrent compiler processes |
| `--limit N` | Print only the best N rows |
| `--keep` | Retain objects under `build/flag_sweep/` |
| `--objdiff` | Ask objdiff for the top candidate's additional score |

## Flag groups

The script combines these groups and adds the phase-specific combinations used
by the Makefile:

| Axis | Values |
|---|---|
| Optimization | none, `-O0`, `-O1`, `-O2`, `-O3`, `-g`, `-g3`, `-O2 -g3` |
| Instruction set | `-mips1 -32`, `-mips2 -32`, `-mips3 -32` |
| Extra option | none, R4300 multiply scheduling, loop-unroll settings, warning suppression |

The tables near the top of `tools/flag_sweep.py` are authoritative. Keep them
in step with the Makefile when a measured flag group is added.

## Target selection

The tool chooses the target in this order:

1. the file passed with `--target-asm`;
2. a matching file under `asm/nonmatchings/`; or
3. the linked function in `build/mickey.us.elf`.

For a linked target, the ROM offset comes from the ELF section's load and
virtual addresses. This works for resident code and overlays; a fixed resident
address formula does not work for synthetic overlay addresses.

## Comparison

Relocated instruction fields are masked before word comparison. The report
contains:

- whether the masked bytes are equal;
- candidate size minus target size;
- differing word count; and
- the first mismatch offset.

Rows sort by exactness, differing words, size difference, and first mismatch.
An exact flag-sweep row is a candidate, not a final match. Confirm the complete
relocation records, linked range, and ROM with the standard comparison tools.

All generated files remain under ignored `build/flag_sweep/`. The report does
not print raw target words or disassembly.
