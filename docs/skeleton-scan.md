# Instruction-skeleton search

`tools/skeleton_scan.py` searches permitted reference objects for functions
with the same instruction structure as Mickey code. It can find likely shared
source after registers, constants, and linked addresses change.

## Skeleton definition

The scanner keeps opcode and function-class fields while masking registers,
immediates, and jump targets. A skeleton match is weaker than byte identity. It
is a candidate for source and naming review, not a match under ADR 0001.

Reference directories may be supplied with `--refs` or through
`MICKEY_DECOMP_REFS`. The default minimum size is ten words.

## Find candidates

```sh
.venv/bin/python tools/skeleton_scan.py scan --region resident \
    --unnamed-only
.venv/bin/python tools/skeleton_scan.py scan --region overlays --json
.venv/bin/python tools/skeleton_scan.py scan --region rom:1000-20000
```

`--emit-symbols` prints candidate symbol lines for unambiguous resident hits.
Review uniqueness, reference provenance, relocations, and function boundaries
before adopting any line.

## Compare region similarity

```sh
.venv/bin/python tools/skeleton_scan.py kinship --region resident
.venv/bin/python tools/skeleton_scan.py kinship --region overlays --ngram 8
```

`kinship` compares masked n-grams. It is useful for prioritization across large
regions, but does not establish individual function identity.

## Find nearest functions

```sh
.venv/bin/python tools/skeleton_scan.py similar \
    --target 0x80031A30 --top 10
.venv/bin/python tools/skeleton_scan.py similar \
    --target 61:+0x120 --top 10
```

A resident target needs a known size in `symbol_addrs.us.txt`. An overlay
target uses `N:+0xOFFSET` and must begin at a recorded text-ownership range.

The similarity score ranks candidates by shared masked n-grams. Inspect the
highest result's types, callers, and control flow before using it as a source
reference.

## Limits

- Different control flow can hide related functions.
- Short or common instruction shapes can produce ambiguous names.
- A skeleton does not compare relocation identities.
- A whole-function match does not prove the surrounding translation-unit
  boundary.
- Reference placeholders are not adoptable names.

The scanner prints names, sizes, counts, and scores. It does not write target
instruction bytes or disassembly to tracked files.

coddog can provide additional decoded-instruction and edit-distance searches
when a suitable project index is available. The local skeleton scanner remains
the direct object-directory tool used by this repository.
