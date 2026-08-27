# 0004. Build parallelism

Status: Accepted
Date: 2026-08-24

## Context

Campaign policy once capped simultaneous compiler jobs at two, enforced
through `tools/with_compile_token.sh`'s two shared tokens, treating that
ceiling as fixed independent of the actual machine. The real cost was
measured on the development machine (M3 Max, 14 cores, 36 GiB): a small
libultra translation unit
compiles in 0.09 s, `src/main/matrix.c` in 0.11 s, and a no-op `gmake -n`
alone takes 0.62 s. No contention measurement was ever recorded to justify
the cap. The workbench's own campaign runner was already using `jobs: 6`,
quietly ignoring the policy it was supposed to follow. `lane/throughput`
(merged into `campaign/unchain` as commit 4af26a3) measured contention
directly on the same machine: a full clean build averaged 55.7 s under
`gmake -j2` versus 17.2 s under `gmake -j12`, and per-translation-unit
compiles ran 0.11 s (libultra) and 0.15 s (overlay).

Separately, a large working tree once existed only in an uncommitted working
directory with no worktree isolation between workers (see ADR 0010). That is a
correctness problem independent of the compile-job ceiling: two workers
building in the same tree contend on the same `build/` output regardless of
job count.

## Decision

Drop the compiler-job ceiling. `gmake -j$(sysctl -n hw.ncpu)` (or the local equivalent)
is the normal build invocation; there is no campaign-mandated cap on
simultaneous compiler processes. `tools/with_compile_token.sh` is retired
as a throughput throttle. It, or an equivalent lock, is kept for exactly
one purpose: serializing the two-phase `gmake verify` (build, then
byte-compare), which is not safe to run concurrently against the same
`build/` directory from two workers.

Contention is avoided structurally, not by rationing jobs: **every worker
builds inside its own lane worktree**, created with `tools/new_lane.sh`,
which gives each worker its own `build/` and `asm/` so lanes never contend
for the same objects. Parallelism across lanes is bounded only by machine
resources, not by a worker-count policy.

## Consequences

- The normal build invocation is `gmake -j$(sysctl -n hw.ncpu)` (or the
  machine's real core count) inside your lane; there is no compiler-job ceiling
  and `tools/with_compile_token.sh` is no longer a throttle.
- `tools/with_compile_token.sh` stays in the tree, scoped to guarding
  `gmake verify`'s two-phase sequence; nothing else needs it.
- The earlier fixed-slot framing is retired along with the job ceiling: the
  constraint that mattered was never slot count, it was lane isolation.
- No change to `gmake verify`'s correctness requirement: it still must
  reproduce SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548` regardless of
  how many jobs built it.
