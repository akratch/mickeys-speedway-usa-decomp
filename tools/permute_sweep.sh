#!/usr/bin/env bash
# Full-queue permuter sweep in a dedicated lane, resynced to the integration
# branch first so every search runs against the source it will be promoted
# into (docs/matching-triage.md: a permuter result is only valid against the
# tree it searched -- a stale lane produced the func_80012574/func_80041CE4
# "false ceilings").
#
#   tools/permute_sweep.sh [lane-name] [permute_batch.py args...]
#
# Default lane: permute-sweep (../mickey-lane-permute-sweep, branch
# lane/permute-sweep). Creates it if missing. Steps:
#   1. hard-reset the lane to master, gmake extract, warm build,
#      gmake verify (the sweep never starts from a non-verifying base);
#   2. run permute_batch.py --apply --commit --order ranking --resume under
#      the load gate, with the caps below unless overridden;
#   3. gmake extract (prunes the .s of every promoted function so the
#      scoreboard counts them), then print progress.
# Each verified promotion is its own commit on the lane branch; integrate
# with tools/merge_lane.sh <lane-name> as usual.
#
# Machine safety: 2 concurrent searches x 4 permuter threads, promotions at
# -j6, and every launch waits for load < 9 (14-core default). Raise only with
# the workstation idle.
set -euo pipefail
cd "$(dirname "$0")/.."
root=$PWD

lane=${1:-permute-sweep}
[ $# -gt 0 ] && shift
lane_dir="$root/../mickey-lane-$lane"

if [ ! -d "$lane_dir" ]; then
    tools/new_lane.sh "$lane" --no-extract >/dev/null
fi

cd "$lane_dir"
if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
    echo "$0: $lane_dir has uncommitted tracked changes; refusing to reset it." >&2
    exit 1
fi
base=$(git -C "$root" rev-parse master)
echo "resync: lane/$lane -> master $base"
git reset -q --hard "$base"
# Untracked leftovers from an earlier sweep (permuter scratch dirs at the
# root, stale asm/) would otherwise shadow the fresh extract.
git clean -qfd -e .venv -e baseroms -e tools/ido -e tools/binutils -e tools/objdiff -e tools/permuter -e .metadata_never_index
gmake extract >/dev/null
gmake -j6 >/dev/null
gmake -j6 >/dev/null   # second pass: the first parallel build after a re-split can race
gmake verify | tail -1

log="build/permuter/sweep-$(date +%Y%m%d-%H%M).log"
mkdir -p build/permuter
echo "sweep log: $lane_dir/$log"
.venv/bin/python -u tools/permute_batch.py \
    --apply --commit --order ranking --resume \
    --jobs 2 --permuter-threads 4 --build-jobs 6 \
    --minutes 20 --extend-minutes 20 --load-threshold 9 \
    "$@" 2>&1 | tee "$log"

gmake extract >/dev/null
gmake -j6 >/dev/null
gmake verify | tail -1
.venv/bin/python tools/progress.py --version us | head -6
echo "promotions on lane/$lane:"
git log --oneline "$base..HEAD" | cat
