# 0007. Matching tools

Status: Accepted
Date: 2026-08-24

## Context

Exact object search does not find related functions when registers, constants,
or calls differ. Unbounded source mutation also produces hard-to-read results
and poor records of why a candidate improved.

## Decision

- Use `find_known_objects.py`, coddog, or `skeleton_scan.py` to find exact and
  structural reference candidates.
- Use asm-differ and decomp-workbench comparison for instruction and relocation
  differences.
- Try the known compiler-flag groups before changing a natural C candidate.
- Run decomp-permuter only as a bounded batch against an existing candidate.
- Review the winning diff and express the result as clear C source.
- Do not use post-compile instruction editing or unbounded custom brute-force
  scripts as part of the matching build.

## Consequences

Each attempt has a defined candidate, command, score, first mismatch, and
result. A tool result still needs full object and ROM verification under ADR
0001.
