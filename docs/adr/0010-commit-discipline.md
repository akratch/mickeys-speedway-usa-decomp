# 0010. Commit discipline

Status: Accepted
Date: 2026-08-24

## Context

Large uncommitted changes are difficult to review, verify, recover, and scan
for prohibited content. Generated comparison files can also collect at the
repository root when matching work is not kept in an isolated worktree.

## Decision

- Work in a dedicated worktree and branch.
- Keep hooks enabled and never bypass them.
- Commit completed work promptly.
- Prefer one exact function with its symbol and progress changes per commit.
- Keep local logs, candidates, objects, and comparison output untracked.
- Keep tracked documentation concise; detailed local attempt records stay in
  ignored work directories.

## Consequences

Each commit can be reviewed and reverted independently. Clean-room checks run
on the actual content being recorded. A handoff names the changed files,
commits, commands, exact result, unresolved mismatch, and next action.
