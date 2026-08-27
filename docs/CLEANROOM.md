# Clean-room policy

This repository must remain safe to distribute. Contributors must use only the
sources listed here and must not commit data copied or derived from a ROM.

## Files that must not be tracked

- ROM images, partial ROM dumps, or ROM fragments
- extracted assets, disassembly, instruction listings, or object files made
  from a ROM
- hexdumps, byte arrays, machine-word tables, or encoded ROM data
- proprietary compiler or Nintendo 64 SDK binaries
- local comparison logs, caches, or workbench data
- credentials, private paths, or files from unrelated checkouts

Generated directories such as `asm/`, `assets/`, `baseroms/`, `build/`, and
`.decomp-workbench/` are local only. The git hooks reject tracked content in
these locations.

## Prohibited sources

Do not use leaked source code, leaked symbols, unreleased source archives, or
debug builds. This includes leaked Rare and Nintendo material, the leaked
*Star Fox Adventures* debug build, and the leaked *Dinosaur Planet* source.

A contributor who has studied leaked source for the relevant code must not
submit reconstructed implementations of that code.

The public *Dinosaur Planet* decompilation may be consulted only for facts about
the binary overlay relocation format. Its names, code, and comments are not
permitted sources for this project. See
[ADR 0008](adr/0008-provenance.md).

## Permitted sources

- analysis of a legally obtained retail *Mickey's Speedway USA* ROM
- official Nintendo 64 documentation, headers, and library source published by
  established decompilation projects
- the published retail-ROM decompilations of *Diddy Kong Racing*, *Jet Force
  Gemini*, *Perfect Dark*, *Banjo-Kazooie*, and *Conker's Bad Fur Day*
- emulator traces from a legally obtained retail ROM, provided no generated
  trace or extracted data is committed

The listed decompilations may supply names and implementation references. A
borrowed or adapted body must have a `PROVENANCE` comment at its point of use,
naming the project and file. Borrowed names must be disclosed in the relevant
symbol block. Mickey's compiled bytes decide every disagreement.

Example:

```c
/* PROVENANCE: adapted from Jet Force Gemini, src/main/runLink.c. */
```

Do not treat a similar subsystem, placeholder name, or matching function size
as proof. [The module map](modules.md) defines the evidence levels used for
names.

## Build inputs

The build requires a user-supplied ROM under `baseroms/`. Setup checks its
SHA-1 before extraction. The ROM and all extracted files remain untracked.

## Automated checks

`tools/cleanroom_check.sh` checks the worktree, staged files, or a commit range.
It rejects forbidden paths, binary and oversized files, disassembly-like text,
large collections of machine words, common text encodings of binary data, and
unapproved workbench files.

```sh
gmake cleanroom
gmake cleanroom CLEANROOM_ARGS=--staged
gmake cleanroom CLEANROOM_ARGS="--range A..B"
```

The pre-commit hook scans the index. The pre-push hook scans every new commit.
Public CI runs the same checker.

Content detection has limits. It can catch common mistakes but cannot prove
that arbitrary text contains no hidden data. Path rules, ignored build
directories, point-of-use provenance, review, and a clean commit history remain
required.

Do not bypass hooks or weaken a detector to admit a file. Investigate a false
positive and document any narrow exemption in the detector source. If
prohibited data reaches a commit, deleting it in a later commit is not enough;
remove it from all reachable history before publication.
