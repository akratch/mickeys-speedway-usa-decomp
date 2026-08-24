# 0004. Build parallelism

Status: Accepted
Date: 2026-08-24

## Context

`AGENTS.md` capped simultaneous compiler jobs at two, enforced through
`tools/with_compile_token.sh`'s two shared tokens, and treated that ceiling
as campaign policy independent of the actual machine. `docs/
acceleration-survey.md` §5 measured the real cost on the development
machine (M3 Max, 14 cores, 36 GiB): a small libultra translation unit
compiles in 0.09 s, `src/main/matrix.c` in 0.11 s, and a no-op `gmake -n`
alone takes 0.62 s. No contention measurement was ever recorded to justify
the cap. The workbench's own campaign runner was already using `jobs: 6`,
quietly ignoring the policy it was supposed to follow. §12 separately timed
a full `-j12` build-and-verify cycle at 13 seconds on the same machine.

Separately, §7 found the entire August working tree — 23 modified tracked
files, 704 untracked paths, `config/normalizations/` (491 files),
`docs/campaigns.md` (5,091 lines at the time), 747 overlay C files, 46
libultra TUs — existed only in an uncommitted working directory, with no
worktree isolation between agents. That is a correctness problem
independent of the compile-job ceiling: two agents building in the same
tree contend on the same `build/` output regardless of job count.

## Decision

Drop the compiler-job ceiling. `gmake -j$(nproc)` (or the local equivalent)
is the normal build invocation; there is no campaign-mandated cap on
simultaneous compiler processes. `tools/with_compile_token.sh` is retired
as a throughput throttle. It — or an equivalent lock — is kept for exactly
one purpose: serializing the two-phase `gmake verify` (build, then
byte-compare), which is not safe to run concurrently against the same
`build/` directory from two workers.

Contention is avoided structurally, not by rationing jobs: **every worker
builds inside its own lane worktree**, created with `tools/new_lane.sh`,
which gives each agent its own `build/` and `asm/` so lanes never contend
for the same objects. Parallelism across lanes is bounded only by machine
resources, not by an agent-count policy.

## Consequences

- `AGENTS.md` no longer mentions a compiler-job ceiling or
  `tools/with_compile_token.sh` as a throttle; it says to build with
  `gmake -j$(nproc)` (or the machine's real core count) inside your lane.
- `tools/with_compile_token.sh` stays in the tree, scoped to guarding
  `gmake verify`'s two-phase sequence; nothing else needs it.
- The "four collaboration slots" framing in the old `AGENTS.md` is retired
  along with the job ceiling (ADR 0009): the constraint that mattered was
  never slot count, it was lane isolation.
- No change to `gmake verify`'s correctness requirement: it still must
  reproduce SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548` regardless of
  how many jobs built it.
