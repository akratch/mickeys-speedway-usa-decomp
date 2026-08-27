# 0002. No post-compile instruction editing

Status: Accepted
Date: 2026-08-24

## Context

Some earlier build rules changed compiled instructions to force an exact
object. That conflicts with the matching standard in ADR 0001. Other build
steps change only object metadata or values that the original build generated
after linking.

## Decision

The build may perform these operations:

- set required ELF header or ABI flags;
- rename, prefix, or strip symbols;
- trim trailing section-alignment padding;
- filter or rebind relocations while temporary per-function overlay objects
  remain in use; and
- calculate ROM header or program checksums that the original build calculated.

The build must not insert, delete, reorder, replace, or edit an instruction
word to obtain a match. Tools that perform those operations may be used for
diagnosis, but their output cannot enter a canonical matched object.

## Consequences

A function that needs an instruction edit remains `NON_MATCHING` over its
assembly fallback. Relocation and trimming helpers are temporary layout support
and should be removed as overlays are consolidated under ADR 0006.
