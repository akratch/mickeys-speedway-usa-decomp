#!/usr/bin/env bash
# Snapshots the current build/ as objdiff's "expected" (target) side:
# expected/build/. Deliberately not a `gmake` target -- expected/ is meant to
# be a *known-good* build's objects, so this only ever runs by hand, right
# after `gmake verify` has confirmed build/mickey.us.z64 is byte-identical to
# the real ROM.
#
#   gmake -j8 && gmake verify && ./tools/make_expected.sh
#
# expected/ is gitignored (like build/, asm/, assets/): it is a local cache
# of a build tree, not source.
set -eu
cd "$(dirname "$0")/.."

if [ ! -f build/mickey.us.z64 ]; then
    echo "$0: build/mickey.us.z64 not found -- run gmake first." >&2
    exit 1
fi

echo "Verifying the current build before snapshotting it as 'expected'..."
gmake verify

rm -rf expected/build
mkdir -p expected
# build/ also carries baseroms extracted by splat's own cache and other
# non-object scratch; objdiff only ever reads *.o, but there is no harm (and
# a lot of simplicity) in mirroring the whole tree the way DKR/dp64 do.
cp -R build expected/build

echo "expected/build/ now mirrors this verified build/."
echo "Re-run this script any time build/ changes and re-verifies, to keep"
echo "objdiff's target side in sync."
