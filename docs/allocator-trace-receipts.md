# Allocator trace receipts

`tools/allocator_trace_receipt.py` turns one instrumented IDO 5.3 allocator
capture into a compact, public-safe evidence receipt. It solves three recurring
problems in the trace workflow:

1. it maps a function symbol to uopt's run-local procedure ordinal from the
   exact named Ucode stream uopt consumed;
2. it refuses allocator evidence until the traced compiler output passes the
   section, relocation, and symbol fidelity gate against stock output; and
3. it records immutable baseline hashes and attempt accounting without copying
   instruction listings or raw compiler traces into documentation.

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
  --attempts 1 --budget 4
```

The full command also requires the number of detailed attempts already spent
and the task budget. It checks that the selected procedure's `p1dec`/`p2dec`
count equals the index capture before invoking `decomp-workbench fidelity`.
When the all-procedure index capture has enough detail, omit `--uopt-trace` and
the command reuses `--index-trace`, avoiding a second compiler run.
Successful text output is intentionally short. `--json` emits the complete
receipt schema for another tool.

The allocator section summarizes:

- integer globalcolor decisions by phase, outcome, register histogram, and a
  stable decision digest;
- floating-point globalcolor decisions using the pinned profile's class-2
  marker; and
- unknown classes separately, so a producer change cannot be silently called
  integer allocation.

The digests make two receipts cheaply comparable while keeping web rows and
raw trace detail out of tracked files.

## Ugen limit

The current ugen producer emits integer- and FP-temporary result rows but no
compiled-procedure identity. A mixed-TU trace therefore cannot be attributed
to one symbol safely. The command reports ugen evidence as `not-provided` by
default and accepts `--ugen-trace` only when the retained Ucode contains one
compiled procedure. In that proven scope it summarizes `ALLOC_GP_RESULT` and
`ALLOC_FP_RESULT` counts, register histograms, and sequence digests.

This is deliberately narrower than the manual workflow used on
`func_80041CE4`: its uopt procedure can now be mapped and receipted
automatically, but its mixed-TU ugen temp/FP path still needs either a
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

The workbench's `docs/compiler-instrumentation.md` describes the trace setup
and the distinction between uopt globalcolor and ugen temporary allocation.
