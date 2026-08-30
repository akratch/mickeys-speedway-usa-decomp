# Overlay consolidation

This is the implementation guide for
[ADR 0006](adr/0006-overlay-source-layout.md). Consolidation changes source
ownership, not matching status.

## Before editing

For the selected overlay, record:

- text ownership and section boundaries from `config/overlays.us.json`;
- function order and current matching status;
- per-object compiler flags and post-compile operations;
- data, read-only data, BSS, and relocation ownership; and
- shared structs, extern declarations, and static objects.

Do not combine functions that need incompatible compiler flags or file-scope
declarations. One main file per overlay is the preferred layout, but two files
are correct when the target object layout requires two flag groups.

## Procedure

1. Put functions in ROM order in `overlay_NNN.c`.
2. Move compatible declarations to the overlay header. Keep distinct structs
   distinct even when old private typedefs used the same name.
3. Preserve `volatile`, signedness, union views, and symbol linkage when they
   affect generated code.
4. Keep separate translation units for incompatible flag groups or static data
   ordering.
5. Assign section ownership in `tools/overlay_atlas.py`; preserve real assembly
   padding and binary tails.
6. Replace per-function `mk/overlays.mk` rules with one rule per surviving
   translation unit.
7. Regenerate the atlas and YAML projection.
8. Compare every formerly matched range and every relocation.
9. Verify the linked overlay and complete ROM.

```sh
gmake overlay-atlas-write
gmake postprocess-audit
gmake verify
gmake check-docs
gmake cleanroom
```

## Common failures

| Failure | Check |
|---|---|
| Functions move in the linked image | Confirm source order equals ROM order |
| A neighboring function changes size | Split files at the compiler-flag boundary |
| Loads disappear or move | Restore the original qualifier or alias view |
| Read-only data moves | Preserve static declaration and literal order |
| Trim helper rejects growth | Find the code-generation change; do not trim real instructions |
| Atlas progress drops after a mixed merge | Prove exact subranges and record them as reviewed mixed-TU ranges |

A source file containing both exact C and a `NON_MATCHING` fallback is
conservatively treated as non-matching unless exact subranges have independent
object, relocation, linked-range, and ROM proof. Record those ranges in the
atlas. Do not split a correct object only to change the progress count.

## Data ownership

Move initialized data only when section order, alignment, symbol identity, and
relocations are known. The overlay header has one combined initialized-data
size, so the atlas uses `data_rodata` until a reliable internal boundary is
established. Do not invent separate data and read-only-data boundaries from
content appearance alone.

BSS has no ROM bytes. Establish it from loader sizes, relocations, and adjacent
symbols. A successful ROM comparison does not by itself prove BSS layout.

Commit one overlay at a time with its atlas, YAML, build policy, headers, and
source changes together.
