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
(cd "$lane" && gmake clean >/dev/null && gmake -j12 verify 2>&1 | tail -1 && gmake check-docs 2>&1 | tail -1)
tools/cleanroom_check.sh --range "HEAD..$branch" 2>&1 | tail -1
echo "== merge $branch"
if ! git merge --no-edit "$branch" >/dev/null 2>&1; then
  conflicts=$(git diff --name-only --diff-filter=U)
  for f in $conflicts; do
    case "$f" in
      README.md|config/overlays.us.json|config/overlay-donors.us.json|config/postprocess-audit.us.json) git checkout --theirs "$f" && git add "$f" ;;
      docs/modules.md|docs/overlays.md) .venv/bin/python tools/resolve_modules_split.py || { echo "unresolved conflict: $f" >&2; exit 1; } ;;
      *) echo "unresolved conflict: $f" >&2; exit 1 ;;
    esac
  done
  git commit -q --no-edit
fi
echo "== integration gates"
tools/with_verify_lock.sh gmake -j12 verify 2>&1 | tail -1
gmake overlay-atlas-write >/dev/null 2>&1 || true
.venv/bin/python tools/refresh_atlas_digest.py >/dev/null
gmake scoreboard 2>&1 | tail -1
gmake overlay-atlas 2>&1 | tail -1
gmake check-docs 2>&1 | tail -1
if ! git diff --quiet; then
  git add -A README.md config/overlays.us.json config/overlay-donors.us.json config/postprocess-audit.us.json 2>/dev/null || true
  git commit -q -m "Regenerate scoreboard/atlas after merging $branch" || true
fi
gmake check-scoreboard 2>&1 | tail -1
git log --oneline -1
