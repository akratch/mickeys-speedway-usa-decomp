# 0004. Build parallelism and worktree isolation

Status: Accepted
Date: 2026-08-24

## Context

Independent builds conflict when they share one `build/` directory. A fixed
two-job compiler limit made clean builds slower without solving that problem.

## Decision

Use `tools/new_lane.sh` to give each independent branch its own worktree,
`build/`, and `asm/`. Build with the machine's available cores; the project has
no fixed compiler-job limit.

Two `gmake verify` commands must not run concurrently in the same worktree.
`tools/with_verify_lock.sh` serializes that two-stage operation where a shared
worktree is unavoidable. A lane's own worktree does not need the shared lock.

## Consequences

Parallel work is isolated by filesystem layout. Each worktree may use its
available CPU without writing another worktree's objects. Verification still
must reproduce the target ROM hash.
