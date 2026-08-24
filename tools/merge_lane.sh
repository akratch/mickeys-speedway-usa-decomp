#!/usr/bin/env bash
# Integrate one lane branch into the current branch (campaign/unchain).
#
#   tools/merge_lane.sh <lane-name>
#
# Runs the lane's own gates from a clean build (verify, check-docs, clean-room
# range scan), merges lane/<name>, resolves the two generated files that always
# conflict by regenerating them (README scoreboard block, overlay atlas), then
# re-runs verify/check-docs/overlay-atlas/check-scoreboard here. Exits non-zero
# and leaves the merge in progress if anything else conflicts or a gate fails.
set -euo pipefail
name=${1:?lane name}
root=$(git rev-parse --show-toplevel)
lane=$(dirname "$root")/mickey-lane-$name
branch=lane/$name
echo "== lane gates ($lane)"
lane_out=$(cd "$lane" && gmake clean >/dev/null && gmake -j12 verify 2>&1 | tail -1); echo "$lane_out"
case "$lane_out" in OK*) ;; *) echo "lane $name does not verify from a clean build; not merging" >&2; exit 1 ;; esac
(cd "$lane" && gmake check-docs 2>&1 | tail -1)
tools/cleanroom_check.sh --range "HEAD..$branch" 2>&1 | tail -1
echo "== merge $branch"
# --no-commit: the merge is committed only after every gate below passes.
if ! git merge --no-commit --no-ff "$branch" >/dev/null 2>&1; then
  conflicts=$(git diff --name-only --diff-filter=U)
  for f in $conflicts; do
    case "$f" in
      README.md|config/overlays.us.json|config/overlay-donors.us.json|config/postprocess-audit.us.json) git checkout --theirs "$f" && git add "$f" ;;
      docs/modules.md|docs/overlays.md) .venv/bin/python tools/resolve_modules_split.py || { echo "unresolved conflict: $f" >&2; exit 1; } ;;
      mickey.us.yaml) .venv/bin/python tools/resolve_comment_hunks.py "$f" && git add "$f" || { echo "unresolved conflict: $f" >&2; exit 1; } ;;
      *) echo "unresolved conflict: $f" >&2; exit 1 ;;
    esac
  done
fi
if git grep -q '^<<<<<<< ' -- . ':!*.md'; then echo "conflict markers left in tracked files:" >&2; git grep -l '^<<<<<<< ' -- . >&2; exit 1; fi
echo "== integration gates"
gmake overlay-atlas-write >/dev/null 2>&1 || true
.venv/bin/python tools/refresh_atlas_digest.py >/dev/null
gmake extract 2>&1 | tail -1
gmake overlay-atlas-write >/dev/null 2>&1 || true
.venv/bin/python tools/refresh_atlas_digest.py >/dev/null
.venv/bin/python tools/fix_stale_externs.py | tail -1
gmake -j12 >/dev/null 2>&1 || true   # warm-up: the first parallel build after a re-split can race
out=$(tools/with_verify_lock.sh gmake -j12 verify 2>&1 | tail -1); echo "$out"
case "$out" in OK*) ;; *) echo "verify FAILED after merging $branch; merge left uncommitted (git merge --abort to drop it)" >&2; gmake -j12 2>&1 | grep -iE 'error|undefined ref|defined twice' | head -5 >&2; exit 1 ;; esac
gmake scoreboard 2>&1 | tail -1
gmake overlay-atlas 2>&1 | tail -1
.venv/bin/python tools/fix_jumptable_claim.py >/dev/null 2>&1 || true
gmake check-docs 2>&1 | tail -1
git add -A README.md config/ docs/modules.md docs/overlays.md mickey.us.yaml symbol_addrs.us.txt src include Makefile 2>/dev/null || true
git commit -q -m "Merge $branch into $(git rev-parse --abbrev-ref HEAD)

Gates at merge time: verify byte-identical, check-docs, overlay-atlas,
scoreboard regenerated." 2>&1 | grep -v exempt || true
gmake check-scoreboard 2>&1 | tail -1
git log --oneline -1
