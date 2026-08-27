# 0003. Progress reporting

Status: Accepted
Date: 2026-08-24

## Context

A single progress number can mix compiled C, original assembly, extracted
assembly, and non-matching C. It also hides the difference between resident
function counts and overlays whose complete function boundaries are not known.

## Decision

Generate progress from the current build and source tree. Report:

- matched resident functions;
- matched C bytes for resident code, overlays, and the whole game;
- verified original hand-written assembly;
- extracted assembly;
- `NON_MATCHING` and `NON_EQUIVALENT` bytes; and
- named resident functions and adopted symbols.

Function counts apply only to the resident segment. Overlay and whole-game
progress use byte counts. Only functions that satisfy ADR 0001 count as matched
C.

## Consequences

`gmake scoreboard` updates the README section. `gmake check-scoreboard` rejects
stale values. The generated tables state their denominators and percentages so
different kinds of progress are not combined without explanation.
