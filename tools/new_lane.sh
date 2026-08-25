#!/usr/bin/env bash
# Create an isolated worktree ("lane") for one worker.
#
#   tools/new_lane.sh <name> [--no-extract] [base-branch]
#
# Creates ../mickey-lane-<name> on branch lane/<name> from base-branch
# (default campaign/unchain), shares the untracked toolchain, baserom, venv and
# vendored tool checkouts with this repository by symlink, and runs the splat
# extract so the lane can build. Each lane has its own build/ and asm/, so
# lanes never contend for the same objects. Prints the lane path.
set -euo pipefail
name=${1:?lane name}; shift
extract=1; base=campaign/unchain
for a in "$@"; do
  case "$a" in --no-extract) extract=0 ;; *) base=$a ;; esac
done
root=$(git rev-parse --show-toplevel)
dest=$(dirname "$root")/mickey-lane-$name
if [ -e "$dest" ]; then echo "lane exists: $dest" >&2; exit 2; fi
git -C "$root" worktree add -q -b "lane/$name" "$dest" "$base"
for p in baseroms tools/ido tools/binutils .venv tools/objdiff; do
  [ -e "$root/$p" ] && ln -s "$root/$p" "$dest/$p"
done
# Submodules: clone from this repository's own module store (no network),
# so the lane's git status stays clean. A symlink here makes git complain
# that it "expected submodule path not to be a symbolic link".
for p in tools/asm-processor tools/asm-differ tools/m2c; do
  git -C "$dest" -c protocol.file.allow=always -c "submodule.$p.url=$root/.git/modules/$p" \
    submodule update --init --quiet "$p" 2>/dev/null \
    || { rm -rf "$dest/$p"; ln -s "$root/$p" "$dest/$p"; }
done
# The permuter checkout is outside the repository; tools/permute.sh expects
# tools/permuter to point at it (git-ignored, machine-specific).
[ -e "$root/tools/permuter" ] && ln -s "$(readlink "$root/tools/permuter" || echo "$root/tools/permuter")" "$dest/tools/permuter"
if [ "$extract" = 1 ]; then
  (cd "$dest" && gmake extract >"$dest/.lane-extract.log" 2>&1) || {
    echo "extract failed, see $dest/.lane-extract.log" >&2; exit 1; }
fi
echo "$dest"
