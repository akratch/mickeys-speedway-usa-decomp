# 0002. No post-compile instruction editing

Status: Accepted
Date: 2026-08-24

## Context

`docs/acceleration-survey.md` §13.2 enumerated every object-level
post-processing step in the five reference decomps (DKR, JFG, BK, PD, dp64):

| Project | Step | Touches |
|---|---|---|
| DKR, PD, dp64 | `patchmips3.py` | one byte of the ELF header `e_flags`, so a `-mips3` object links |
| BK | `set_o32abi_bit.py`, `objcopy --prefix-symbols`, `strip` | ELF flags and symbol table |
| DKR | `calc_func_checksums.py`, `n64crc` | post-link data words: the game's own anti-tamper checksums and header CRC, which the original build also computed |
| PD | `mkrom` | the same: piracy-checksum placeholders and the header CRC |
| dp64 | `elf2dll.py` symbol-binding/GOT overrides | relocation metadata for DLLs that still contain `GLOBAL_ASM`, documented in dp64 itself as "hacks … should only be used by the decomp" |
| dp64 | `recomp_rom_patcher.py` | instruction bytes, but only in the deliberately non-matching static-recomp output |

**No gold-standard project edits an instruction word after compilation in
order to reach a match.** Mickey's own `set_elf_flags.py` on `ll.c` is
already the `patchmips3.py` case and needs no change. Everything else in the
tree that edits object bytes, `normalize_elf_instructions.py`,
`normalize_o63_*.py`, `resize_elf_function.py`,
`extend_elf_function_to_text.py`, and their `drop-*`/`reorder`/`fields`/
commute operations, has no precedent in any reference project and produces
the 274-function, 63.5%-of-overlay-bytes gap ADR 0001 describes.

Relocation filtering/rebinding and section trimming are metadata-only and
have a partial precedent in dp64's `elf2dll` overrides, but they exist in
Mickey's tree because of the per-function translation-unit model (ADR 0006),
not because any reference project needed them for matching. They are
scaffolding for a layout decision this project is retiring, not a permanent
part of the pipeline.

## Decision

**Permitted**, because every reference project does one or more of these and
none of them touch an instruction word:

- ELF header/ABI bits, e.g. `set_elf_flags.py`'s `e_flags` edit for `-mips3`
  objects (DKR/PD/dp64's `patchmips3.py` case).
- Symbol-table renames, prefixing, and stripping.
- Post-link data words the *original build itself computed*: ROM checksums,
  header CRC, and equivalent anti-tamper/piracy-check placeholders (DKR's
  `calc_func_checksums.py`/`n64crc`, PD's `mkrom`).
- Trimming a section's trailing alignment padding.
- Relocation filtering/rebinding and section-trim steps, tolerated only as
  linker-model scaffolding for the per-function-TU layout, to be retired
  when ADR 0006's per-overlay-TU consolidation lands.

**Not permitted, ever, to reach a match:** any change to an instruction
word: field edits, insertion, deletion, reordering, or operand commuting.
Concretely: `normalize_elf_instructions.py`, `normalize_o63_*.py`,
`resize_elf_function.py`, `extend_elf_function_to_text.py`, and any
`drop-*`/`reorder`/`fields`/commute operation on compiled instructions.

Every function currently matched only by one of the prohibited steps is
converted to `#ifdef NON_MATCHING` C over `#pragma GLOBAL_ASM`, DKR's own
form for a function whose C is written but not yet byte-identical, rather
than deleted or left mis-reported as matched.

## Consequences

- The 274 instruction-altered functions move from "matched" to
  `NON_MATCHING`/`GLOBAL_ASM` under ADR 0001, becoming real queue items
  rather than closed ones.
- `normalize_elf_instructions.py`, `normalize_o63_*.py`,
  `resize_elf_function.py`, and `extend_elf_function_to_text.py` are retired
  from the matching build; they are not deleted from history, since they
  remain useful *diagnostically* the way DKR's workbench case studies used
  compiler-internals instrumentation, but their output never again silently
  reaches a canonical object.
- Relocation filter/rebind and trim steps stay in place until ADR 0006's
  per-overlay consolidation removes the per-function-TU model that requires
  them.
- Hard cases are still solvable without object editing: DKR's own endgame
  case studies (`trackbg_render_flashy`, `func_80049794`, `func_8008FF1C`)
  were all closed by source restructuring guided by compiler-internals
  instrumentation, not by touching the object. That is the expected route
  for Mickey's converted functions too.
