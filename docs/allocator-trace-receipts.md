# Allocator trace receipts

`tools/allocator_trace_receipt.py` turns one instrumented IDO 5.3 allocator
capture into a compact, public-safe evidence receipt. It solves three recurring
problems in the trace workflow:

1. it maps a function symbol to uopt's run-local procedure ordinal from the
   exact named Ucode stream uopt consumed;
2. it refuses allocator evidence until the traced compiler output passes the
   section, relocation, and symbol fidelity gate against stock output; and
3. it records immutable baseline hashes and attempt accounting without copying
   instruction listings or raw compiler traces into documentation; and
4. it joins hash-bound workbench frame evidence, producer-emitted stack homes,
   and procedure-scoped ugen temp events into a deterministic first-mechanism
   comparison.

Raw traces and objects remain untracked workbench evidence. The receipt is a
diagnostic handoff, not a match claim.

## Capture contract

Use the same candidate source and flags for every object in one receipt:

- `candidate.o` is the output from the stock IDO toolchain.
- `candidate.B` is the retained positional Ucode input consumed by that uopt
  invocation. Create the capture toolchain with `decomp-workbench capture make
  INSTRUMENTED_IDO_ROOT CAPTURE_DIR --phase uopt --link`; the run's
  `before-<N>-*` positional input is this file.
- `index.log` is one complete instrumented-uopt capture made with `CDX_LOG=1`
  and a nonnumeric `CDX_PROC` value. The pinned profile refuses the name and
  emits one `procindex` row for every globalcolor invocation.
- `traced.o` is the object emitted by the instrumented index capture. If the
  all-procedure log is too broad for the next investigation, `detail.log` may
  come from a second compile with `CDX_LOG=1` and the numeric procedure selected
  below. Keep each capture separate; do not merge logs from several compiles.

First map the symbol before spending the detailed capture:

```sh
tools/allocator_trace_receipt.py func_80041CE4 \
  --candidate-object build/trace/particles.stock.o \
  --index-trace build/trace/particles.index.log \
  --ucode-stream build/trace/particles.candidate.B \
  --map-only
```

The command accepts the map only when all of these are true:

- the symbol owns exactly one nonempty ELF `FUNC` range and aliases agree on
  start and size;
- every retained `Uent` has a valid following procedure-name `Ucomm`;
- the named Ucode procedure count equals the complete contiguous `procindex`
  count; and
- the requested procedure name occurs exactly once.

The Ucode record order is the optimizer's procedure order; ELF order is not
used for this join. That distinction matters in Mickey because `asm-processor`
adds helper procedures which do not preserve a simple source/ELF ordinal.
Any ambiguity is an error, not a best guess. Procedure ordinals are run-local:
repeat this mapping after every translation-unit source or flag change.

Capture the reported numeric procedure and emit the full receipt:

```sh
tools/allocator_trace_receipt.py func_80041CE4 \
  --candidate-object build/trace/particles.stock.o \
  --traced-object build/trace/particles.traced.o \
  --index-trace build/trace/particles.index.log \
  --ucode-stream build/trace/particles.candidate.B \
  --uopt-trace build/trace/particles.detail.log \
  --workbench-summary build/wb/func_80041CE4.summary.json \
  --attempts 1 --budget 4
```

The full command also requires the number of detailed attempts already spent
and the task budget. It checks that the selected procedure's `p1dec`/`p2dec`
count equals the index capture before invoking `decomp-workbench fidelity`.
When the all-procedure index capture has enough detail, omit `--uopt-trace` and
the command reuses `--index-trace`, avoiding a second compiler run.
Successful text output is intentionally short. `--json` emits the complete
receipt schema for another tool.

Generate the hash-bound frame input from the same candidate object before the
trace capture:

```sh
tools/wb_compare.sh --summary-json func_80041CE4 \
  > build/wb/func_80041CE4.summary.json
```

The receipt accepts only `mickey-wb-summary-v1`, requires its requested and
candidate symbols to equal the receipt symbol, and requires its candidate
object SHA-256 to equal `--candidate-object`. A stale summary is an error. The
candidate and target frame sizes are otherwise `unavailable`; the tool does
not rediscover them from an instruction listing.

The allocator section summarizes:

- integer globalcolor decisions by phase, outcome, register histogram, and a
  stable decision digest;
- floating-point globalcolor decisions using the pinned profile's class-2
  marker; and
- unknown classes separately, so a producer change cannot be silently called
  integer allocation.

The digests make two receipts cheaply comparable while keeping web rows and
raw trace detail out of tracked files.

## Structured allocator summary

The additive `trace_summary` section keeps the original v1 receipt fields and
CLI behavior intact. Its three evidence lanes are:

- `frame`: candidate and target byte sizes from the hash-bound workbench
  summary;
- `stack_homes`: only homes for which the producer emitted a virtual or final
  offset. An explicit `width`, `bytes`, or `size` becomes `width_bytes`; an
  explicit `access` or `access_class` becomes `access_class`. Missing fields
  are `null` or `unavailable`, never inferred from a data type, opcode, or
  opaque compiler word; and
- `temp_events`: procedure-attributed ugen result rows as deterministic
  birth/pop and death/push events. Each event retains the conventional register
  name and producer source line, but omits the raw row, trace line, emitted
  ordinal, compiler addresses, and host path.

Source files are reduced to a safe basename. Procedure attribution comes from
the same one-procedure Ucode proof that already gates `--ugen-trace`; a
multi-procedure ugen capture remains inadmissible.

The optional `--target-evidence` document permits field-by-field comparison
when compact target-side evidence has already been measured:

```json
{
  "schema": "mickey-allocator-target-evidence-v1",
  "symbol": "func_80041CE4",
  "frame_size_bytes": 32,
  "stack_homes": {
    "status": "available",
    "homes": [
      {"offset": 24, "width_bytes": 4, "access_class": "load-store"}
    ]
  },
  "temp_events": {"status": "unavailable"}
}
```

The schema is deliberately closed: extra fields, raw traces, malformed widths
or access classes, and a different symbol are rejected. Use `null` or
`{"status":"unavailable"}` when target evidence cannot establish a field.
In particular, target machine code does not itself prove a ugen temp birth;
do not transcribe a plausible event sequence from the candidate trace.

`comparison.fields` reports each lane as `equal`, `divergent`, `partial`, or
`unavailable`. `comparison.first_divergence` then names the first proved
mechanism and one bounded source lever. Its current mechanisms distinguish a
frame-size difference, stack-home count/displacement/width/access, an extra or
missing temp birth/death, and event order/source attribution. It reports
`no-divergence` only when all three lanes are available and equal. Equal known
fields plus one unavailable field remain `unavailable`, not a guessed match.

For a `func_80050E9C`-style trace, one additional integer pop/birth on the
path-loop line is reported as `extra-temp-birth`; the next lever is to inspect
that attributed expression for a redundant conversion, comparison carrier, or
grouping. The tool does not prescribe a source edit and the receipt remains
diagnostic evidence rather than match proof.

## Ugen limit

The current ugen producer emits integer- and FP-temporary result rows but no
compiled-procedure identity. A mixed-TU trace therefore cannot be attributed
to one symbol safely. The command reports ugen evidence as `not-provided` by
default and accepts `--ugen-trace` only when the retained Ucode contains one
compiled procedure. In that proven scope it summarizes `ALLOC_GP_RESULT` and
`ALLOC_FP_RESULT` counts, register histograms, sequence digests, and lifecycle
events. `ALLOC_GP_RESULT`/`ALLOC_FP_RESULT` are births and free-list pops;
`FREE`/`FORCE_FREE` are deaths and pushes. Request rows are validated but are
not misreported as allocated registers.

This is deliberately narrower than the manual workflow used on
`func_80041CE4`: its uopt procedure can now be mapped and receipted
automatically, but its mixed-TU ugen temp/FP lane still needs either a
one-function faithful scratch or a future producer-side procedure marker.

## Failure meanings

- A count/order failure means the symbol-to-procedure join is unproved. Do not
  substitute a remembered procedure number.
- A decision-count failure means the detail log is incomplete, stale, or from
  another compile.
- A fidelity failure means the instrumented compiler is not evidence for the
  stock compiler on that candidate.
- A budget failure means the handoff omitted or exceeded its bounded-attempt
  accounting.
- A target-evidence failure means the compact JSON was malformed, stale,
  symbol-conflicting, or tried to carry an unsupported field.
- An `unavailable` comparison is a request for narrower evidence, not evidence
  that candidate and target agree.

## Attempt-zero worker recipe

Before the first source edit, a worker should:

1. run `wb_compare.sh --summary-json` and retain its ignored JSON beside the
   workbench target object;
2. map the fresh Ucode procedure with `--map-only`;
3. capture one fidelity-clean detail trace and, only for a one-procedure input,
   one ugen trace;
4. run the full receipt with `--workbench-summary`, `--attempts 1`, and the
   assigned budget; and
5. read `comparison.first_divergence` before selecting the first source lever.

Add `--target-evidence` only for target fields already measured independently.
After every TU or flag change, remap the procedure, recapture, and regenerate
the receipt; procedure ordinals and allocator-local identities are run-local.
Preserve the JSON under ignored `build/` evidence and quote only its compact
mechanism, metrics, and next lever in a handoff.

The workbench's `docs/compiler-instrumentation.md` describes the trace setup
and the distinction between uopt globalcolor and ugen temporary allocation.
