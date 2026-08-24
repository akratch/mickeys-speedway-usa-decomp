#!/usr/bin/env bash
# Finish an in-progress merge after manual conflict resolution.
#
#   tools/finish_merge.sh
#
# Refuses to commit unless every gate passes: no conflict markers in tracked
# files, splat re-extract, atlas + digest regeneration, stale-extern rename,
# byte-identical verify, recomputed derived claims, check-docs, scoreboard,
# overlay-atlas. Prints the progress lines on success.
set -euo pipefail
if git grep -q '^<<<<<<< ' -- . ':!*.md'; then echo "conflict markers remain:" >&2; git grep -l '^<<<<<<< ' -- . >&2; exit 1; fi
if git diff --name-only --diff-filter=U | grep -q .; then echo "unmerged paths remain:" >&2; git diff --name-only --diff-filter=U >&2; exit 1; fi
gmake extract 2>&1 | tail -1
gmake overlay-atlas-write >/dev/null 2>&1 || true
.venv/bin/python tools/refresh_atlas_digest.py >/dev/null
.venv/bin/python tools/fix_stale_externs.py | tail -1
out=$(tools/with_verify_lock.sh gmake -j12 verify 2>&1 | tail -1); echo "$out"
case "$out" in OK*) ;; *) echo "verify FAILED; not committing" >&2; gmake -j12 2>&1 | grep -iE 'error|undefined ref|defined twice' | head -5 >&2; exit 1 ;; esac
.venv/bin/python tools/fix_jumptable_claim.py | tail -1
gmake check-docs 2>&1 | tail -1
gmake scoreboard 2>&1 | tail -1
gmake overlay-atlas 2>&1 | tail -1
git add -A README.md config/ mickey.us.yaml docs/modules.md docs/overlays.md symbol_addrs.us.txt src include Makefile
git commit -q --no-edit 2>&1 | grep -v exempt || true
git log --oneline -1
timeout 240 gmake progress 2>&1 | grep -E '^functions|decompiled|NON_MATCH'
