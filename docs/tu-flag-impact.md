# Per-TU compiler-flag impact analysis

`tools/tu_flag_impact.py` answers the question a one-function flag sweep cannot:
if a compiler flag helps one guarded candidate, what does that same per-file
policy do to every other `NON_MATCHING` candidate compiled in the translation
unit?

For example, to test the R4300 multiply-hazard pass suggested by
`func_80048080` without changing the Make policy:

```sh
nice -n 10 .venv/bin/python tools/tu_flag_impact.py func_80048080 \
  --add-flags=-Wab,-r4300_mul --timeout-seconds 120
```

Flags begin with `-`, so use the `--add-flags=...` form. Multiple tokens may
be quoted in one value, and both options are repeatable. Replacing an exclusive
family requires an explicit removal:

```sh
nice -n 10 .venv/bin/python tools/tu_flag_impact.py SYMBOL \
  --remove-flags=-O2 --add-flags=-O3 --timeout-seconds 120
```

The timeout is a hard outer wall-clock cap over identity resolution, the two
whole-TU builds, and scoring. It must be between 1 and 600 seconds. The tool
compiles sequentially, so it never creates more than one compiler process of
its own at a time. Invoke it through `nice` on an interactive workstation.

## What the command proves

The command composes the campaign's existing evidence machinery:

1. `function_preflight.py` resolves the seed and then every guarded C symbol
   to one target identity, fallback and owning source file.
2. `permute_batch.py` enumerates balanced `#ifdef NON_MATCHING` candidate /
   `GLOBAL_ASM` pairs and asks `gmake -n` for the owning TU's real codegen
   flags.
3. `flag_sweep.py` compiles the complete source with `-DNON_MATCHING`, once
   with configured flags and once with the requested delta. Its canonical
   ownership, target assembly, symbol extraction, relocation masking and
   scoring routines are reused unchanged.

Every guarded function in the file gets a row. `base d/s/first` and
`trial d/s/first` are masked differing words, candidate-minus-target byte
size, and first mismatch offset. `changed` is the raw number of generated
words changed between the two candidate functions. `reloc t/b/v` gives target,
baseline and trial relocation-site counts. The verdict uses the same ordering
as `flag_sweep.py`: fewer masked differences, then a smaller absolute size
delta, then a later first mismatch.

The summary distinguishes newly relocation-masked-exact consumers from exact
consumers lost by the flag. `--json` emits the same complete report as a single
schema-versioned JSON value. No instruction text or word values are printed.

## Fail-closed boundary

There is no partial report. The command exits nonzero and emits no result table
if it cannot prove the seed's guarded selection, enumerate every complete
candidate/fallback pair, resolve one canonical target for each, recover the
configured flags from Make, compile both complete objects, or extract and score
any one consumer. It also refuses a TU with post-compile policy it cannot
reproduce, an absent removal, duplicate/conflicting flag families, incomplete
`--rescore` cache, or expiry of the outer deadline.

The content-addressed objects, target assemblies and extracted sections stay
under ignored `build/tu_flag_impact/`. `--rescore` requires both cached objects
and never compiles; source, headers, flag delta and toolchain inputs are part of
the cache identity.

## What it does not prove

The comparison is between unlinked objects. As in `flag_sweep.py`, relocation
payload bits are masked, and a masked-exact row is only a candidate lead. The
report does not compare relocation identities, linked overlay ranges, or the
final ROM; it never edits source or Make policy. A useful flag must still
receive a TU-wide policy impact review, and each promoted function must pass
the configured full-TU, exact relocation, linked-range and ROM proofs required
by ADR 0001.
