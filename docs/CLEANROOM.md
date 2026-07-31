# Clean-Room Policy

This project aims to be a fully public, legally distributable matching
decompilation. To keep it publishable:

## Never committed to this repository
- ROM images or partial ROM dumps (any region/version)
- Extracted assets, disassembly (`asm/`), or binary blobs derived from a ROM
- Proprietary compiler binaries (IDO) or SDK binaries

## Prohibited knowledge sources
Code, symbol names, comments, or structure may NOT be derived from:
- Any leaked Rare/Nintendo source code
- The leaked Star Fox Adventures debug build (July 2002) or its symbols
- The leaked Dinosaur Planet build or its symbols
- Any other unreleased/leaked build of any game

Contributors who have studied leaked Rare source code should not
contribute matched C implementations.

**Narrow exception — binary file-format facts only:** the *public*
Dinosaur Planet decompilation project (its documentation and tooling,
built from a retail ROM — not the leaked build) may be consulted
purely to understand the binary layout of Rare's DLL/overlay
reloc-table format (field offsets, relocation types, table structure),
since Mickey's overlay system uses the same mechanism. This exception
never extends to symbol names, code, or comments — only structural/
format facts about how the retail binary is laid out.

## Permitted knowledge sources
- Analysis of retail ROMs you legally possess
- Published matching-decomp repositories built from retail ROMs
  (Diddy Kong Racing, Jet Force Gemini, Perfect Dark, Banjo-Kazooie,
  Conker's Bad Fur Day): their symbol naming, their documentation, and
  **their C source — including adapting a function body from one**,
  where Mickey's ROM shows the same engine's code. Such repositories are
  themselves clean-room work product built from retail ROMs, so this is
  not a route back to any prohibited source above. Two conditions:
  **(a)** the adapted body must be disclosed with a `PROVENANCE` note at
  the point of use, naming the project and the file it came from, and
  **(b)** Mickey's own ROM decides every disagreement — a borrowed body
  is a starting point to be matched against this game's bytes, never an
  authority over them. `src/main/runlink.c` is the worked example, and
  `docs/modules.md` section 1.3 states how the disclosure is written.
- Official Nintendo 64 SDK documentation, headers, and library source as
  distributed in existing public decomp projects
- Emulator tracing/debugging of retail ROMs

## Build inputs
The build requires you to supply your own legally dumped ROM
(`baseroms/`), verified by SHA1 before use.
