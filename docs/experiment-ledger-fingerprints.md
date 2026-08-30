# Isolated experiment fingerprints

This note supplements `docs/experiment-ledger.md` for journal schema version 2.
It documents the duplicate-detection key used when an append ingests a current
`function_preflight.py` JSON report. The journal remains ignored local evidence,
not matching proof.

## Why the key changed

Schema version 1 used the SHA-256 of the complete candidate translation-unit
object. That safely rejected an identical object, but a change to any other
function or data item in the same object changed the digest. The same owned
function could therefore be compiled and diagnosed repeatedly.

Schema version 2 keeps that object hash as artifact provenance but does not use
it as the primary duplicate key. It records a `candidate_fingerprint` containing
only an algorithm name, SHA-256, owned byte size, and relocation count. No code
bytes, instruction text, disassembly, or relocation rows enter the journal.

## Fingerprint evidence

The `mips-elf-function-v1` algorithm accepts an append only when all of these
agree:

- the preflight report and CLI resolve to one candidate symbol and source;
- the report is not older than its candidate object;
- the ELF is a bounded, big-endian, relocatable ELF32 object with one `.text`
  section and one symbol table;
- exactly one nonzero function definition owns the candidate symbol, its size
  equals the preflight candidate word count, and no other function overlaps it;
- every function-relative relocation is aligned, uniquely sited, supported,
  and names an unambiguous symbol identity;
- the ELF relocation count equals both detailed and summary preflight counts;
- preflight resolved every candidate static relocation identity.

The digest covers the owned function bytes after masking only the field written
by each supported MIPS relocation. It also covers, for every relocation, its
function-relative offset, type, target-symbol identity, and decoded REL addend.
HI16/LO16 groups must pair unambiguously. Consequently an unrelated function's
bytes, symbol location, or relocation rows cannot change this key, while a
change to the owned code or its relocation semantics does.

Unsupported, stale, missing, duplicate, overlapping, section-only, or otherwise
ambiguous evidence is a refusal. The tool never falls back to a whole-object
digest for a new preflight append.

## Journal compatibility

Readers validate schema versions 1 and 2 in the same append-only journal.
Existing version-1 rows are never rewritten. A new version-2 append compares
its isolated digest with prior version-2 rows for the same symbol. When a prior
version-1 row has no isolated digest, exact whole-object equality is still
checked so the old duplicate guarantee is preserved; unrelated object changes
cannot be retroactively isolated from an immutable legacy row.

The fully explicit CLI path, which has no preflight object evidence, continues
to emit schema version 1. It records scalar metrics but cannot claim isolated
compiler-output deduplication.
