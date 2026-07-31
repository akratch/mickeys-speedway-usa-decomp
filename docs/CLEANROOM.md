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

## What the automated gates can and cannot do

`tools/cleanroom_check.sh` runs at commit, at push, and in CI. It is worth
being exact about what that buys, because a gate trusted for more than it
delivers is worse than no gate at all.

**Content detection of arbitrarily-encoded data is undecidable in general.**
Any file can be made to carry ROM bytes in a form no detector anticipates. The
content rules here are calibrated against *mistakes*, not against an adversary,
and they are measured: every threshold is set from the whole of this
repository's history on the pass side and fixtures of real ROM data on the
fail side.

### What the content layer reliably catches

Measured, at 400 real ROM instruction words per fixture:

- disassembly listings (`asm/*.s`, `objdump` output), upper or lower case
- hexdumps: `xxd`, `xxd -p`, `hexdump -C`, `od`, 64-column hex lines
- C arrays of ROM words or bytes, including `u`/`UL` type suffixes,
  underscore separators and zero padding
- machine words in prose, in hex ranges, as 16-bit halves, as dotted quads,
  in octal, in decimal
- escaped byte strings (`"\x27\xbd..."`)
- base64, base64url, base32 and ascii85 blobs, including line-wrapped and
  split across several files
- JSON ledgers of any of the above, single-line or pretty-printed
- the two workbench ledgers that caused the original incident
- a leak spread thinly across many files, via the aggregate budget

### What it does not catch

Known, measured holes — not speculation:

- **Sub-threshold trickles.** A single file carrying fewer than 96 machine
  words — about 384 bytes of ROM — is below both the per-file rule (192 words)
  and the aggregate budget (96). A measured fixture with 63 words passes
  today. Repeated commits of such files would accumulate unnoticed.
- **Digest-shaped strings.** Up to 64 standalone 32/40/64/128-character hex
  tokens per file are exempt where they appear inline beside other content,
  because this tree legitimately records hashes and a SHA-256 is
  indistinguishable from ROM data by any content metric. Deliberately abused
  that is roughly 2 KB of ROM per file.
- **Deliberate steganography.** Any encoding the normalizer does not
  implement, data hidden in whitespace or in the low bits of otherwise
  plausible numbers, or content split below every threshold on purpose.
- **Anything under a name the workbench redactor does not recognise** (see
  `ledger_redaction._sweep`, which covers keys beginning with `target`, not
  string contents).

### What is actually load-bearing

The guarantees that do not depend on out-guessing an encoding are structural:

1. **The path whitelist under `.decomp-workbench/`** — only
   `campaigns/*/manifest.json` may ever be tracked, and that file is validated
   against its schema (default-deny on keys), not sampled for suspicious
   content. There is nowhere in it to put a ROM word.
2. **The `asm/`, `assets/`, `baseroms/`, `expected/` path rules**, and the
   binary and oversize rules, which do not care about encoding at all.
3. **The tool-level fix** — the workbench no longer writes the ROM's
   instruction text into a ledger, so the file that leaked cannot be recreated.
4. **Policy and review**: `CLAUDE.md`, `docs/CONTRIBUTING.md`, and a human
   reading the diff. The gates exist so a mistake is caught, not so review can
   be skipped.
5. **A server-side push ruleset** on the remote — *not yet configured*. Every
   layer in this repository is client-side and skippable with `--no-verify`,
   and CI reports after publication rather than preventing it. A required
   status check on a protected branch is the only layer that cannot be stepped
   over, and it is the recommended next step.

Treat the content detectors as defence in depth against mistakes. They are not
an adversary-proof boundary and must not be described as one.
