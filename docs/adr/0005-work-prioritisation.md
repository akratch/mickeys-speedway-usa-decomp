# 0005. Work prioritisation

Status: Accepted
Date: 2026-08-24

## Context

Reference-object scans find substantially more useful matches in resident code
than in overlays. Raw byte size alone therefore gives a poor estimate of the
time required to match a target.

## Decision

Prioritize work in this order:

1. exact or near-exact reference-backed resident functions;
2. named resident functions with clear translation-unit and type information;
3. small functions with narrow compiler mismatches; and
4. remaining targets, ranked by expected matched bytes per unit of work.

Use Diddy Kong Racing first for shared game systems and Jet Force Gemini where
its engine or overlay layout is closer. A reference result is a candidate, not
proof; ADR 0001 still applies.

## Consequences

The ready list is regenerated from current match, reference, and blocker data.
Large overlay targets do not take priority over a smaller resident target with
stronger evidence.
