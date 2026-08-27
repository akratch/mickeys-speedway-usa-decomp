# 0010. Commit discipline

Status: Accepted
Date: 2026-08-24

## Context

A review on 2026-08-24 found a month of work living only in the working
directory: last commit 2026-08-22 (a documentation change), before that
2026-07-31; 23 modified tracked files (+13,583/-284) and 704 untracked paths;
the entire object-editing apparatus ADR 0002 retires — `config/normalizations/`
(491 files), `docs/campaigns.md` (5,091 lines at the time), 747 overlay C files,
46 new libultra TUs — never committed. Prior policy had discouraged committing
mid-campaign, and nothing had replaced it.

The consequences were concrete, not hypothetical: no clean-room sweep had
run on any of it, because the pre-commit hook scans the index, not the
working tree; `gmake check-docs` couldn't pin any number in
`docs/campaigns.md`; nothing could be bisected; a `git clean` or a bad
`checkout` would have lost the entire month's work. The repository root
also accumulated `-o`, `-o.unlinked`, `attempt*`, `candidate*`, `variant*`,
`preprocessed_*`, and roughly 700 `.tmp-*` files, the untracked residue of
the hand-rolled search loops ADR 0007 retires.

Separately, `docs/campaigns.md` itself, at 321,904 bytes with a hex/word
table inside it, fails the clean-room content gate outright (measured
directly: `gmake cleanroom` on it flags an oversize file and a word-table
finding).

## Decision

Work is **committed as it lands**, not batched into an end-of-campaign
dump. Concretely:

- Every worker operates in its own lane worktree (`tools/new_lane.sh`,
  branch `lane/<name>`, ADR 0004) with hooks always active, never
  `--no-verify` (this is already `docs/CONTRIBUTING.md` policy; this ADR does not
  weaken it, it just says workers *do* commit, on their own lane).
- Commits are function-sized: one exact function, its symbol-table line,
  and its atlas row land together. This is also the granularity objdiff
  and the permuter (ADR 0007) report on, so it's not an arbitrary size
  choice.
- Lane branches integrate through the existing `campaign/unchain` /
  integration flow; nothing is left to live only in a working directory
  between integration points.
- Log-style documents (`docs/campaigns.md` and anything like it) are kept
  under the tracked-file size the clean-room gate accepts (well under its
  256 KB practical ceiling) and free of hex/word tables. A document that
  outgrows that is **split**: a short tracked summary (goals, exits,
  results, no hex tables, no per-function byte ledgers) stays in git;
  detailed per-epoch ledgers move outside git entirely, the same place
  `.decomp-workbench/` ledgers already live per `docs/CLEANROOM.md`.

## Consequences

- `docs/campaigns.md` as it stood (321 KB, gate-failing) is split: a
  summary under 60 KB stays tracked; the detailed per-epoch ledgers are
  retained outside git, with a note in the summary saying so.
- Each worker's handoff report states files changed and commit hashes on
  its own lane branch, matching the existing evidence-discipline
  requirement in `docs/CONTRIBUTING.md`.
- The rule is: commit on your own lane branch, in small commits, hooks always
  on, never touch another lane's worktree or branch.
- This does not relax any clean-room gate; it makes the gates actually run,
  by making commits, where the pre-commit hook fires, the normal unit of
  work instead of something deferred indefinitely.
