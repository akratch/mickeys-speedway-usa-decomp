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

Every line below was re-measured by running the detectors against a fixture
built from this game's own ROM — 400 real instruction words, or 1600 bytes,
per fixture. The numbers in brackets are `words / spread` as the detectors
scored them, against per-file limits of 192 words **and** 32 distinct high
bytes.

- disassembly listings (`asm/*.s`, `objdump` output), upper or lower case
- hexdumps: `xxd` [485/58], `xxd -p` [1146/73], `hexdump -C`, `od`,
  64-column hex lines
- C arrays of ROM words or bytes, including `u`/`UL` type suffixes and
  underscore separators [400/40], and **zero-padded** to 9 digits [836/44]
- machine words in prose, in hex ranges, as dotted quads [400/40], in octal
  [1040/75], in decimal [985/97]
- **hex 16-bit halves, however they are labelled** — bare [385/58],
  comma-separated [385/58], one per line [385/58], as JSON array pairs
  `"w0": ["0x27bd", "0xffe8"]` [375/57], as split `_hi`/`_lo` keys [375/57],
  as `\u27bd\uffe8` escapes [385/58], and as markdown table rows [385/58].
  The gate is **volume, not adjacency**: a file must carry 64 halves pairs
  before any are read as words (see below for the two rules that failed first,
  and the residual)
- escaped byte strings (`"\x27\xbd..."`) [400/40]
- base64 [400/40], base64url [386/117], base32 [415/50] and ascii85 [342/167],
  **line-wrapped at any width** — every column from 4 to 40 was measured with a
  faithful hard column wrap, for base64 *and* for ascii85, and every one is
  caught [400/40] — and split across several files. Two earlier versions of
  this claim were wrong. The first listed only "20, 31, 64 and 76 columns"
  because the base64 block-join carried a 16-character per-line floor. The
  second said "any width" while the *ascii85* floor was still 32, so ascii85
  wrapped narrower than 32 columns was not caught at all. Both floors are now
  4, measured to cost nothing: over every tracked file the tightest margin is
  identical at every floor from 32 down to 4, and no file fires at any of
  them.
- runs whose length is **undecodable for their encoding** — base64 at 1 mod 4,
  base32 at 1/3/6 mod 8, ascii85 at 1 mod 5 [399/40, 413/50, 399/40]. These
  used to raise inside the decoder, get swallowed, and vanish unexamined; a
  base32 blob in that shape scored 26 words
- JSON ledgers of any of the above, single-line or pretty-printed [436/41]
- the two workbench ledgers that caused the original incident [5593/201 and
  263/47]
- a leak spread across many files **when each file is word-dense**: 3 files of
  134 words each score 436 against an aggregate budget of 96

### What it does not catch

Known, measured holes — not speculation. Each was reproduced against the
current detectors.

- **High-byte stripping.** Write each word as six hex digits with the top byte
  dropped and the file scores **zero words, zero spread** — every rule here is
  silent. This is called out separately from "an encoding we did not
  implement" because it is not one: it is a one-character change to a form
  listed above as caught, and the top byte of a MIPS text section is nearly
  constant within an opcode class, so the transform is trivially invertible by
  whoever wrote it. It defeats the *statistic*, not the decoder. `spread` is
  the discriminator carrying the per-file rule, and any bijective transform
  that collapses the high byte — masking it, storing it separately, shifting
  the alignment — takes the rule with it.
- **Prose dilution.** A single file carrying **191 `0x`-prefixed words, about
  764 bytes of ROM**, one per padded sentence, passes everything: the ceiling is
  set by the rule, not by the fixture — `WORD_TABLE_COUNT_LIMIT` is 192, so 191
  is the most a file can yield and still pass, and the aggregate ignores it
  because the aggregate's rate floor (12 words/KiB) is a *per-file* rate that
  padding drives arbitrarily low. Eight such files carry roughly **6 KB**
  (8 × 764 = 6112 bytes) with the aggregate budget still reading **0** —
  measured, not inferred. Two earlier versions of this section were wrong here
  and both are worth recording: one said the ceiling was 96 words / 384 bytes,
  which is `AGGREGATE_WORD_BUDGET`, a different rule, and half the real figure;
  the next said 188 words / 752 bytes, which was one padded fixture's incidental
  size rather than the ceiling. Quote the limit, not the fixture that happened
  to be measured against it.
- **Decimal 16-bit halves.** Hex halves are recombined into words; the same
  split written in decimal is not, and a 400-word fixture scores **0**.
- **Fewer than 64 hex halves pairs in a file.** Halves are gated on volume:
  a file must contain at least 64 pairs — two 4-digit hex runs separated by at
  most 24 characters — before any pair is read as a word. Below that, none
  are. A file carrying 63 pairs (about 252 bytes of ROM in halves form) is
  therefore not caught by this route.

  Two narrower rules were tried first and both are worth recording, because
  each looked correct and each was defeated. Pairing at **unbounded distance**
  fused a value on line 18 with one on line 234 and produced 7 of the 13
  phantom high bytes in `docs/modules.md`. Requiring a long **chain of
  tightly-adjacent** halves killed those phantoms but was defeated by putting a
  label between the pairs — keyed JSON, split `_hi`/`_lo` keys, `\u` escapes
  and markdown rows all reset the chain at every label and scored **zero**,
  which is a one-line change for anyone leaking. Volume is what actually
  separates a dump from prose: measured at a 24-character gap, the most pairs
  in any blob in this repository's entire history is **14**, while the weakest
  of nine halves fixture families carries **375**.
- **Digest-shaped strings.** The first 64 distinct 32/40/64/128-character hex
  tokens per file are exempt where they appear inline beside other content,
  because this tree legitimately records hashes and a SHA-256 is
  indistinguishable from ROM data by any content metric. Tokens past the 64th
  count normally, so the exemption is a slope rather than a cliff: measured, a
  file of digest-shaped ROM passes at 64 and 65 tokens and is caught at 88.
  **Deliberately abused that is about 2.7 KiB of ROM per file** — 87 digests
  is the ceiling measured on the standard fixture region (ROM 0x32630), and it
  is **strongly** byte-value dependent, because what actually trips is the
  spread of the words they decode to rather than their count: re-measured over
  six ROM regions the ceiling ranges from 87 to 141 digests, 2.7 KiB to 4.4 KiB.
  Quote it as a range or say "about 2.7 KiB on the measured region"; a single
  figure here is a fixture property, not a property of the rule. An earlier
  figure of 3.2 KB here was an estimate, not a measurement. The widening
  relative to the old 2 KB is deliberate and is the price of removing
  a discontinuity that made a documentation file recording a 65th hash score
  520 words — a false positive on legitimate growth, sitting directly under
  the tightest margin in the tree.
- **Deliberate steganography.** Any encoding the normalizer does not
  implement, data hidden in whitespace or in the low bits of otherwise
  plausible numbers, or content split below every threshold on purpose.
- **Anything under a name the workbench redactor does not recognise** (see
  `ledger_redaction._sweep`: it covers target-naming keys in any case at any
  depth in any container, but it does not read string *contents*, so target
  code under an innocuous key name or as a bare list element survives).
- **Absolute filesystem paths in the workbench manifests**, which are a
  publication problem rather than a clean-room one, and are *known and
  deliberately not scrubbed*. Both tracked manifests under
  `.decomp-workbench/campaigns/` record `/Users/<name>/…` in
  `identity_inputs`, and the manifest schema check permits them: `_m_path`
  requires a path, not a relative one. They leak a maintainer's home directory
  layout into a repository intended for publication, and they are the reason
  this bullet exists rather than a quiet fix — because the quiet fix would be
  worse. `identity` is
  `sha256(json.dumps(identity_inputs, sort_keys=True, separators=(",",":")))`
  — verified, both manifests reproduce their recorded digest exactly — and the
  campaign directory name embeds its first twelve hex characters
  (`SetLinkSlot-ee19dab3a5b2`). Rewriting the paths to relative ones therefore
  invalidates the digest and the directory name together, and re-deriving both
  would produce a record of a campaign that was never run under those inputs.
  A manifest exists to be reproducible; falsifying one to tidy a home directory
  out of it trades a cosmetic leak for a fabricated record, which is the worse
  trade in this repository specifically. **The fix at publication time is to
  drop `.decomp-workbench/` from the published tree, or to re-run the two
  campaigns from a neutral root and commit the manifests they genuinely
  produce** — not to edit these.

The pattern across all of these is worth stating plainly, because three rounds
of review found it independently: **content detection here is calibrated for
accidents, and the accident case is well covered. Against someone trying, the
per-file rule dies to any transform that flattens `spread`, and the aggregate
dies to padding.** Both are one-line changes. Do not read the list above as a
perimeter.

### What is actually load-bearing

The guarantees that do not depend on out-guessing an encoding are structural:

1. **The path whitelist under `.decomp-workbench/`** — only
   `campaigns/*/manifest.json` may ever be tracked, and that file is validated
   against a **path-typed allow-list**: every leaf the workbench writes is
   enumerated with its permitted JSON types and a checker for its value's
   shape, and anything else — unknown key, unexpected type, float where a
   float was not declared, out-of-range number, over-long string, over-long
   array, unexpected nesting — is rejected. A legitimate schema change fails
   the check until someone updates the table, and that is the intended cost.

   **This used to say "there is nowhere in it to put a ROM word". That was
   false and is retracted.** A review walked around the old check five ways,
   one of them the real schema verbatim. The honest statement is a *bound*: a
   sha256 is 32 arbitrary bytes and a manifest legitimately carries several, so
   typing cannot make a hash field carry less than a hash. What the validator
   does is count and cap them — at most 64 source records, so at most
   `4 + 2×64 = 132` digest-typed leaves — and force every other leaf into a
   shape with no room. **Residual capacity: about 4.2 KB, and only for someone
   deliberately writing ROM bytes into fields that are supposed to be hashes.**
   The two real manifests carry 34 and 16 digests. Everything outside those
   leaves is also measured statistically, with the schema-validated digests
   blanked first, so the file answers to both layers.
2. **The `asm/`, `assets/`, `baseroms/`, `expected/` path rules**, and the
   binary and oversize rules, which do not care about encoding at all.
3. **The tool-level fix** — the workbench ledger writer strips every
   target-naming field, at any depth, in any container, replacing it with a
   16-bit salted digest and a masked opcode. It does not read string contents,
   so it is not a guarantee that no instruction text can reach a ledger; it is
   a guarantee that the *schema no longer asks for it*, which is what made the
   original incident automatic. Ledgers stay gitignored regardless.
4. **Policy and review**: `CLAUDE.md`, `docs/CONTRIBUTING.md`, and a human
   reading the diff. The gates exist so a mistake is caught, not so review can
   be skipped.
5. **A server-side ruleset on the remote.** Every layer above is client-side
   and skippable with `--no-verify`, and CI reports after publication rather
   than preventing it. `protect-master` (GitHub ruleset id `20111399`, active
   on `master`) closes part of that gap: it blocks force-push
   (`non_fast_forward`) and branch deletion, which is genuinely
   non-bypassable. It does not, and structurally cannot, restrict *content* —
   GitHub rejected a push ruleset scoped to file path/extension/size with
   "only org-owned repos can have push rules," which this personal fork is
   not. A required status check on a protected branch remains the only layer
   that could block bad content before it lands rather than after, and it is
   still the recommended next step; it has not been configured, and adopting
   it would mean routing changes through pull requests rather than pushing to
   `master` directly. See `docs/CONTRIBUTING.md`'s "Checks" section for the
   full detail, including the corollary that force-push being blocked means a
   future history purge of `master` requires temporarily disabling
   `protect-master` first.

### False positives are a failure of this system too

A gate that fires on legitimate work gets turned off, and `--no-verify` turns
off *everything*. This policy positively encourages adapting function bodies
from the published decomps named above, so source files are expected to carry
hex constants, struct offsets and small instruction citations.

Round 4 found `src/main/runlink.c` — this repository's own worked example of
sanctioned adaptation — at 162 words / spread 28 against limits of 192 / 32,
about thirty words from firing. The cause was a defect, not a threshold: 116 of
those words were base64 **false decodes** of `#pragma GLOBAL_ASM("asm/.../
func_80031A30.s")` paths, and decoded garbage is uniformly distributed, so it
inflated `spread` fastest. Fixing the decoder took the file to 83 words /
spread 6, and a further fix below took it to 47 words / spread 4.

Two more defects of the same kind were found the same way — by asking where a
file's words actually came from, rather than by adjusting a limit:

- `_`-stripping treated `D_80081898` and `func_10003920` — symbol names, which
  decomp source and documentation are made of — as digit-grouped literals, and
  the fabricated word (`0xd8008189`) landed on a high byte nothing else in the
  file used.
- **16-bit halves were paired at any distance.** Every 4-digit hex run in a
  document went into one list and was paired in order, so a value on line 18
  was fused with a value on line 234 into a 32-bit "word" that exists nowhere.
  Being built from two unrelated numbers its high byte is arbitrary, and it was
  responsible for **7 of the 13 distinct high bytes in `docs/modules.md`** and
  for 9 of `symbol_addrs.us.txt`'s 13. Halves now pair only when adjacent.

Each of these was a steady drip of noise into `spread` — the metric protecting
every file here.

A closing audit traced **every** normalized word in **every blob in this
repository's history**
back to the mechanism that produced it, and fixed the two that were still
inventing data: a shell assignment (`OBJDUMP=tools/binutils/mips64-elf-objdump`)
was being decoded first as base64 and then, once that was blocked, as ascii85;
and adjacent-halves pairing was still fusing comment lists (`0x1B74, 0x27A0`)
and markdown table cells (`| 1232 | 1105 |`). Every synthetic decoder now
contributes **zero** words across the whole history — the only producers left
are real hex addresses and a couple of decimal tokens. That audit is no longer
a one-off: it is `gmake audit-decoders`, and it fails.

No blob count is quoted here on purpose. It changes with every text commit, so
any number written down is stale the moment the next one lands — this document
said "238" for several rounds after the true figure had moved on, which is the
recompute-don't-remember rule broken inside the document that states it.
`gmake audit-decoders AUDIT_ARGS=--all` prints the count it actually scanned;
that is the figure, and it is the only one that is ever current. Note it will
read slightly *below* `git rev-list --all | … | sort -u | wc -l`, because the
audit skips empty and non-text blobs — reconcile the two that way rather than
assuming one of them is wrong. The first draft of this very paragraph quoted
both numbers "for scale" and they were stale one commit later; that is how
persistent this failure mode is.

The tightest margin over **every** blob in this repository's history is now
**6.40×** (`docs/modules.md`, 95 words / spread 5 against limits of 192 / 32 —
so 6.40× under spread, which is the gate protecting it), up from 2.46×
before the halves fix and 1.19× before the decoder work began.
`symbol_addrs.us.txt` sits at 16.0×, and it is protected by spread rather than
count — it carries 436 words against a 192 limit, nearly all sharing the high
byte `0x80`, which is the whole reason the rule is a pair.
**Watch this number as the tree grows**: it is the one that says whether the
gate is still a safety net or is about to become an obstacle. If a gate
does fire on work you believe is legitimate, the answer is
`CONTENT_EXEMPTIONS` in `tools/cleanroom_detectors.py` — one path, one
detector, a written reason, and a diff someone reviews — **not** `--no-verify`.
It is empty today.

Treat the content detectors as defence in depth against mistakes. They are not
an adversary-proof boundary and must not be described as one.

### Maintaining the detectors

> **A decoder change must be re-audited, never argued.**
> Run `gmake audit-decoders` after touching `normalize_words_by_stage` or
> anything it calls. Do not reason about whether the change is safe — run it.

This is the most important operational rule in this document, and it is written
here because it was learned the expensive way.

`tools/cleanroom_detectors.py` decodes a file into candidate 32-bit words and
then measures that stream. **Five separate times, what it measured was not in
the file**: a `#pragma GLOBAL_ASM` path decoded as base64; symbol names like
`D_80081898` `_`-joined into words; unrelated markdown table cells fused as
16-bit halves; this repository's own `OBJDUMP=…` build variable decoded first
as base64 and then as ascii85; and ragged hex runs read from one end only.
Decoded garbage is uniformly distributed, so each one inflated `spread` — the
metric that decides whether a file looks like ROM — and two of them brought
this tree's own files to within 1.2× of failing the gate on work this very
policy encourages.

**Twice, a fix re-routed the phantom instead of removing it.** Blocking base64
on that shell variable handed it straight to the ascii85 branch. Requiring
16-bit halves to be adjacent still left comment lists pairing. Neither was
visible by reading the diff; both were caught by re-running the audit. That is
why the rule is "run it", not "check it".

What `gmake audit-decoders` asserts:

1. the per-stage word buckets reconstruct `normalize_words`' real output
   exactly (there is one implementation, not a mirrored copy);
2. the decoder set is **closed in all three directions** — every declared stage
   has a ceiling, every ceiling has a stage, and every bucket the normalizer
   actually *produces* is declared. The third is not redundant: words in an
   undeclared bucket are still returned by `normalize_words`, so without it a
   phantom could ship while the audit reported success;
3. the eight *synthetic* decoders contribute **zero** words. Nothing in this
   tree is encoded, so a nonzero count is a false decode by definition, not a
   budget to spend;
4. the two decoders that read genuinely-written numbers (`hex-run`,
   `dec-token`) stay under a generous tripwire that catches a flood rather than
   budgeting normal growth.

**What it does not cover, and this matters.** The audit looks in the
false-*positive* direction only — a decoder that starts inventing words. It is
blind to the opposite failure, a decoder that quietly *stops* producing words
it should, because a decoder going silent looks exactly like a decoder
behaving. Both defects found in one review round were of that kind: a base64
run whose length was 1 mod 4 raised inside the decoder and was swallowed, and
a per-line floor meant narrow-wrapped payloads were never joined. Neither moved
a single number in the audit.

**False-negative coverage comes from `gmake check-fixtures`** — 64 fixture
families of real ROM words in every encoding, wrapped at every width, each
asserted to still be caught — not from this tool. For several rounds this
paragraph named a "fixture suite" that existed only in a scratchpad and could
not be run by anyone reading it; it is now
[`tools/false_negative_fixtures.py`](../tools/false_negative_fixtures.py) and it
is a make target.

The fixtures are **generated at run time from `baseroms/mickey.us.z64` and
never written to disk**, which is not an implementation convenience but the only
available design: a fixture that proves the detectors catch ROM data *is* ROM
data, so committing one would violate the policy this very file states, and
`gmake cleanroom` would correctly reject it. The consequence is that this check,
like `gmake verify`, cannot run in CI — there is no baserom there — and it fails
rather than skips when the baserom is missing, because a green bar from a suite
that tested nothing is worse than a red one.

It asserts in both directions. Every fixture that carries ROM must be caught;
and every family corresponding to a hole listed above under "What it does not
catch" must still *pass*, so that if the detectors quietly get stronger this
document stops being able to overstate the weakness.

Verified by driving it: reverting either wrap floor from 4 to 32 leaves
`gmake audit-decoders` green (256 texts, no decoder inventing words) while
`gmake check-fixtures` reports 16 missed families including every base64 wrap
from 4 to 31 columns; removing the undecodable-length trim likewise leaves the
audit green while the fixtures report `b32_len_mod8_is_1` and `b64_wrap27`
silently producing nothing. Run both. Neither replaces the other, and a clean
audit is not evidence that anything still works.

It runs over tracked files by default (about 0.2 s) and over every blob in
history with `AUDIT_ARGS=--all` (about 4 s). It is deliberately **not** wired
into `gmake cleanroom` or the git hooks: those run on every commit, must stay
fast, and must fail only for clean-room reasons. This check answers a different
question — *is the detector still measuring reality?* — and it is aimed at
whoever edits a decoder. Making an unrelated red bar block commits is precisely
how a gate ends up bypassed with `--no-verify`.
