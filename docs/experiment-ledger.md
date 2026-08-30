# Matching experiment ledger

`tools/experiment_ledger.py` keeps an append-only local journal of compact
matching metrics. It is evidence for choosing the next hypothesis, not match
proof and not a tracked campaign ledger. The default file is
`build/experiment-ledger.jsonl`; the CLI refuses every destination outside
`build/`, which is gitignored.

The journal never accepts instruction text, machine words, ROM bytes, target
rows, disassembly, or absolute paths. Do not use it to replace the canonical
promotion proof, a plateau handoff, or the source itself.

## Append one experiment

```sh
tools/experiment_ledger.py append func_80001234 \
  --source src/main/example.c \
  --hypothesis "Narrow local lifetime around the conditional" \
  --candidate-words 77 --target-words 77 \
  --raw-differences 7 --masked-differences 2 \
  --frame 0x30 \
  --candidate-relocations 7 --target-relocations 7 \
  --relocation-identities 7 \
  --first-raw-mismatch +0x20 --first-masked-mismatch +0x34 \
  --verdict improved
```

Use `frameless` for a zero-byte frame and `none` for a mismatch offset whose
difference count is zero. Optional artifacts use a repository-relative path
below `build/` and a SHA-256 digest:

```sh
tools/experiment_ledger.py append func_80001234 ... \
  --artifact build/wb/func_80001234.candidate.o=SHA256
```

The symbol must be one ASCII C identifier, and the named `src/**/*.c` file
must exist and name that symbol. Numeric invariants also fail closed: masked
differences cannot exceed raw differences; mismatch offsets are aligned byte
offsets and are present exactly when the associated count is nonzero;
relocation identities cannot exceed either relocation count; and `exact`
requires equal word counts, zero differences, and exact relocation counts and
identities.

The hypothesis is a bounded one-line mechanism description. Assembly
mnemonics, machine-word-shaped values, hexadecimal/escaped-byte payloads,
control characters, and path text are refused. The tool does not silently
redact them: once appended, a record is immutable, so unsafe input must never
enter the journal.

## Read and rank

```sh
tools/experiment_ledger.py list
tools/experiment_ledger.py list --symbol func_80001234 --limit 5 --json
tools/experiment_ledger.py best func_80001234
tools/experiment_ledger.py best --json
tools/experiment_ledger.py summarize
tools/experiment_ledger.py summarize --symbol func_80001234 --json
```

`list` preserves append order. `best` selects one record per symbol by exact
verdict, then the fewest relocation-masked differences, raw differences,
word-count delta, and unresolved relocation identities. Verdict class breaks
remaining ties, followed by the latest immutable record. `summarize` reports
record, symbol, exact-symbol, verdict, and per-symbol best-masked counts.

## Schema and durability

Each physical line is one strict schema-version-1 JSON object. Unknown,
missing, duplicate, malformed, oversized, or internally inconsistent fields
make both reading and further appends fail. A record contains only:

- `schema_version`, UTC `timestamp`, `symbol`, `source`, and `hypothesis`;
- candidate/target word counts, raw and relocation-masked difference counts;
- frame byte count, candidate/target relocation counts, and exact relocation
  identity count;
- first raw/masked mismatch byte offsets and `verdict`;
- optionally, up to eight `artifacts`, each containing only a relative
  `build/` path and lowercase SHA-256.

Appending takes an exclusive file lock, validates the complete existing
journal, and performs one bounded `O_APPEND` write. It then calls `fsync` on
the file and, when creating the journal, on its containing directory. Readers
take a shared lock. Existing bytes are never rewritten; a truncated or corrupt
journal must be preserved for inspection rather than extended.

This protects cooperating local processes and ordinary interruption. It is
not a tamper-evident database and cannot prevent a separate program from
rewriting an ignored file. Artifact hashes let a contributor determine whether
an optional local object still matches the attempt that recorded it.
