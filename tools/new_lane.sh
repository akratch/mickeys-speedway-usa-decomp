#!/usr/bin/env bash
# Create an isolated worktree ("lane") for one worker.
#
#   tools/new_lane.sh <name> [--no-extract] [base-branch]
#
# Creates ../mickey-lane-<name> on branch lane/<name> from base-branch
# (default: caller HEAD),
# shares the untracked toolchain, baserom, venv and
# vendored tool checkouts with this repository by symlink, and runs the splat
# extract so the lane can build. Each lane has its own build/ and asm/, so
# lanes never contend for the same objects. Prints the lane path.
set -euo pipefail
name=${1:?lane name}; shift
extract=1; base=
for a in "$@"; do
  case "$a" in --no-extract) extract=0 ;; *) base=$a ;; esac
done
# Resolve an explicitly supplied relative ref (especially HEAD) in the calling
# worktree before switching Git operations to the primary checkout.
caller=$(git rev-parse --show-toplevel)
# Always anchor lane creation in the primary checkout. When this helper is
# invoked from an existing linked worktree, --show-toplevel names that lane and
# its .git is a file, so "$root/.git/modules" cannot be the shared submodule
# store. The common directory is stable from every worktree.
common=$(git rev-parse --path-format=absolute --git-common-dir)
if [ "$(basename "$common")" != .git ]; then
  echo "expected a non-bare repository with a .git common directory: $common" >&2
  exit 2
fi
root=$(dirname "$common")
if [ -z "$base" ]; then base=HEAD; fi
base_commit=$(git -C "$caller" rev-parse --verify "$base^{commit}")
display_dest=$(dirname "$root")/mickey-lane-$name
dest=$display_dest
# `.metadata_never_index` did not stop per-lane mdworker storms on macOS.
# Register the real worktree below Spotlight's `.noindex` directory suffix and
# preserve the established lane path as a compatibility symlink.
if [ "$(uname -s)" = Darwin ]; then
  dest=$display_dest.noindex
fi
if [ -e "$display_dest" ] || [ -L "$display_dest" ] || [ -e "$dest" ]; then
  echo "lane exists: $display_dest" >&2
  exit 2
fi
# Creating several full worktrees can make macOS Spotlight index every copied
# source/build path at once. Create the registered worktree without populating
# it, install the marker, and only then materialize the tracked tree. Other
# platforms harmlessly ignore this git-ignored empty file.
git -C "$root" worktree add -q --no-checkout \
  -b "lane/$name" "$dest" "$base_commit"
: > "$dest/.metadata_never_index"
git -C "$dest" read-tree "$base_commit"
git -C "$dest" checkout-index --all
for p in baseroms tools/ido tools/binutils .venv tools/objdiff; do
  [ -e "$root/$p" ] && ln -s "$root/$p" "$dest/$p"
done
# Submodules: clone from this repository's own module store (no network),
# so the lane's git status stays clean. A symlink here makes git complain
# that it "expected submodule path not to be a symbolic link".
for p in tools/asm-processor tools/asm-differ tools/m2c; do
  if ! git -C "$dest" -c protocol.file.allow=always \
      -c "submodule.$p.url=$common/modules/$p" \
      submodule update --init --quiet "$p"; then
    echo "submodule init failed for $p; refusing a dirty symlink fallback" >&2
    exit 1
  fi
done
# The permuter checkout is outside the repository; tools/permute.sh expects
# tools/permuter to point at it (git-ignored, machine-specific).
[ -e "$root/tools/permuter" ] && ln -s "$(readlink "$root/tools/permuter" || echo "$root/tools/permuter")" "$dest/tools/permuter"
if [ "$extract" = 1 ]; then
  (cd "$dest" && gmake extract >"$dest/.lane-extract.log" 2>&1) || {
    echo "extract failed, see $dest/.lane-extract.log" >&2; exit 1; }
fi
if [ "$dest" != "$display_dest" ]; then
  ln -s "$(basename "$dest")" "$display_dest"
fi
echo "$display_dest"
