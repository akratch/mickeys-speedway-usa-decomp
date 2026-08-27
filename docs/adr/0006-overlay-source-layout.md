# 0006. Overlay source layout

Status: Accepted
Date: 2026-08-24

## Context

The initial overlay split used one C file per function. That duplicated private
types and declarations, left data sections without natural owners, and required
extra relocation handling. Established Rare decompilations group an overlay's
code and data into a small number of translation units with shared headers.

## Decision

Use one main translation unit per overlay:
`src/overlays/oNNN/overlay_NNN.c`. It owns the overlay's text, read-only data,
initialized data, and BSS through shared declarations.

A per-function file may be used while first establishing a function, but it is
temporary. Consolidate an overlay when several functions or shared types make
the split files harder to maintain. Confirm section bytes and relocations after
each move.

## Consequences

Private duplicate types and alias declarations are removed during
consolidation. Temporary relocation-filter and section-trim rules are removed
when the consolidated object no longer needs them. Consolidation does not
change a function's matching status.
