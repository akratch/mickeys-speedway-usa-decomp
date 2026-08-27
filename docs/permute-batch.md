# Bounded permutation

`tools/permute_batch.py` runs decomp-permuter against functions that already
have guarded C candidates. It is a batch tool for narrow compiler mismatches,
not a source generator or an interactive search loop.

## Usage

```sh
# Show the queue.
.venv/bin/python tools/permute_batch.py --list

# Run one function without changing source.
.venv/bin/python tools/permute_batch.py --function <name> --minutes 15

# Run a limited overlay batch.
.venv/bin/python tools/permute_batch.py --overlay 15 --limit 4 --minutes 15
```

Options after `--` are passed to `permuter.py`.

| Option | Purpose |
|---|---|
| `--function NAME` | Select one function |
| `--overlay N` | Select one overlay |
| `--limit N` | Limit the number of functions |
| `--minutes N` | Set the per-function wall-clock limit |
| `--jobs N` | Set concurrent functions |
| `--permuter-threads N` | Set threads used by each permuter process |
| `--apply` | Attempt verified source promotion for a zero-score result |

## Candidate preparation

The tool finds guarded `NON_MATCHING` functions, locates their target assembly,
and prepares an ignored working directory. It compiles with the translation
unit's known flag group and defines `NON_MATCHING` so the C body is visible.
All neighboring functions retain their assembly fallbacks during verification.

An overlay target is located by overlay and section offset. The shared
synthetic virtual address is not used as its identity.

## Reviewing results

A lower permuter score means only that the isolated candidate is closer under
the permuter's comparison. Review the winning source diff. Reject mutations
that add constant conditions, obscure types, change semantics, or exist only
to manipulate the compiler without a defensible source interpretation.

A zero score in the scratch directory is still provisional. Translation-unit
context, static data, relocations, and neighboring functions can change the
real object.

## Applying a result

Without `--apply`, the tool never edits tracked source. With `--apply`, it may
splice a zero-score candidate, rebuild the real object, compare the owned
range, and run verification. Promotion must satisfy the same requirements as a
manual match:

- readable and semantically faithful C;
- exact function bytes and size;
- exact relocation count, type, offset, and target;
- correct linked placement; and
- a byte-identical ROM.

Keep the assembly fallback unless every check passes. Scratch files remain in
ignored work directories and must not be committed.
