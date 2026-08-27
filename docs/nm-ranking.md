# Ranking non-matching functions

`tools/nm_ranking.py` ranks functions that have a C body under
`#ifdef NON_MATCHING`. It helps select small compiler mismatches before larger
structural rewrites.

## Usage

```sh
.venv/bin/python tools/nm_ranking.py --top 25 --markdown
```

Common options:

| Option | Purpose |
|---|---|
| `--jobs N` | Set concurrent isolated compiles |
| `--top N` | Print the first N ranked rows |
| `--limit N` | Process at most N functions |
| `--out PATH` | Write the machine-readable report |
| `--objdiff-report PATH` | Add scores from an existing objdiff report |
| `--no-table` | Suppress the terminal table |

## Method

For each function, the tool builds two isolated objects:

- the target assembly under `asm/nonmatchings/`; and
- the guarded C candidate with the translation unit's compiler flags.

The isolated path avoids whole-object trim rules that may reject the larger
output of a non-matching translation unit. When isolation cannot import the C
body directly, the tool selects that body and keeps neighboring assembly
fallbacks while compiling the original translation unit.

The report includes target size, candidate size difference, differing word
count, first mismatch, and an optional objdiff percentage. It assigns a broad
category:

| Category | Meaning |
|---|---|
| `size-mismatch` | Candidate and target sizes differ |
| `schedule-only` | The same words appear in a different order |
| `register-only` | Differences are limited to register fields |
| `immediate-only` | Differences are limited to constant or address fields |
| `other` | The difference needs direct inspection |

These categories are routing hints. They do not establish semantic
correctness, relocation identity, or a full-object match.

## Using the ranking

Start with equal-size functions that differ by few words. Confirm the
translation-unit flags and relocations before changing C. Use the workbench
diagnosis to choose a source-level change. Move to larger structural targets
only after the near matches have been checked.

The generated report and all compiled objects are local comparison data. Keep
them in ignored paths and do not commit them.
