# 0001. Matching standard

Status: Accepted
Date: 2026-08-24

## Context

The project needs one objective definition of a matched function. A compiled
object can be made equal to the target by changing its instructions after
compilation, but that result does not show that the C source reproduces the
original program.

## Decision

A function is matched only when all of the following are true:

- the tracked C source is compiled by the configured, unmodified IDO compiler;
- no later step changes an instruction word;
- the function is linked at its correct location;
- every owned byte and every required relocation matches the target; and
- the complete ROM still verifies byte for byte.

Code under `NON_MATCHING` or `NON_EQUIVALENT` is unmatched. Original
hand-written assembly listed in `verified_asm.us.txt` is reported separately.

## Consequences

Equal size, semantic agreement, or a high comparison score is not enough. A
useful body that does not match remains under a conditional guard with its
assembly fallback. Progress tools count only compiler-produced matches.
