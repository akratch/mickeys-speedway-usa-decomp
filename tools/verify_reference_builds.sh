#!/usr/bin/env bash
# Checks a local reference-build farm against tools/reference-builds.lock: for
# each title, that the checkout sits on the pinned commit and that the objects
# under it hash to the digest the lock records.  This is what makes a rebuilt
# farm provably the farm the tier-A names in this tree were mined from.
#
#   tools/verify_reference_builds.sh                 check every locked title
#   tools/verify_reference_builds.sh jfg conker      check named titles only
#   tools/verify_reference_builds.sh --print         print digests, compare nothing
#   tools/verify_reference_builds.sh --root DIR      farm lives somewhere else
#
# This script decides WHICH objects to look at, out of the lock; the digest
# itself is tools/reference_build_digest.py's, which hashes what mining reads
# rather than the whole object file -- read its header before changing either.
#
# --print is how the lock is refreshed after a deliberate change to the farm:
# it emits the same digests in lock order with nothing to compare them to.  It
# is not a pass.  A rebuild that disagrees with the lock is a finding, and
# blessing one silently is the thing this script exists to prevent.
#
# Needs the farm, which is out of tree by design (docs/CLEANROOM.md), so like
# `verify` and `check-fixtures` this cannot run in CI.
#
# Exits 0 when every checked title matches, 1 on a mismatch, 2 on a usage or
# plumbing error.

set -u

repo=$(cd "$(dirname "$0")/.." && pwd) || exit 2
lock=$repo/tools/reference-builds.lock
root=${REFS_ROOT:-$HOME/Desktop/dev/decomp-refs}
mode=check
# A plain string, not an array: lock section names never contain whitespace,
# and `${arr[@]}` under `set -u` is an error on the bash macOS ships.
titles=

usage() {
	cat <<'EOF'
usage: tools/verify_reference_builds.sh [--root DIR] [--print] [title ...]

  --root DIR   farm location (default $REFS_ROOT, else ~/Desktop/dev/decomp-refs)
  --print      print each title's digest instead of comparing it
  title ...    lock section names; default is all of them

Exits 0 when everything checked matches, 1 on a mismatch, 2 on a usage or
plumbing error.
EOF
}

while [ $# -gt 0 ]; do
	case "$1" in
	--root)
		[ $# -ge 2 ] || {
			echo "verify-refs: --root needs a directory" >&2
			exit 2
		}
		root=$2
		shift 2
		;;
	--root=*)
		root=${1#--root=}
		shift
		;;
	--print)
		mode=print
		shift
		;;
	-h | --help)
		usage
		exit 0
		;;
	-*)
		echo "verify-refs: unknown argument '$1'" >&2
		usage >&2
		exit 2
		;;
	*)
		titles="$titles $1"
		shift
		;;
	esac
done

[ -f "$lock" ] || {
	echo "verify-refs: no lock at $lock" >&2
	exit 2
}

# Stdlib-only, so plain python3 is enough -- no venv, same reason as the
# clean-room detectors: this has to run on a machine that has a farm, which is
# not necessarily a machine that has built Mickey.
PY=${PYTHON:-python3}
command -v "$PY" >/dev/null 2>&1 || {
	echo "verify-refs: $PY not found; cannot compute digests" >&2
	exit 2
}

# One field out of one section of the lock.  Deliberately not a general parser:
# the lock is a fixed shape written by hand, and a parser that tolerates more
# than that shape would tolerate a typo that turns a check into a no-op.
field() {
	awk -v want="[$1]" -v key="$2" '
		/^\[/ { in_section = ($0 == want); next }
		!in_section || /^#/ { next }
		$1 == key && $2 == "=" { $1 = ""; $2 = ""; sub(/^  */, ""); print; exit }
	' "$lock"
}

if [ -z "$titles" ]; then
	titles=$(sed -n 's/^\[\(.*\)\]$/\1/p' "$lock" | tr '\n' ' ')
fi
[ -n "${titles// /}" ] || {
	echo "verify-refs: lock has no titles" >&2
	exit 2
}

rc=0
for title in $titles; do
	want_digest=$(field "$title" digest)
	want_commit=$(field "$title" commit)
	want_count=$(field "$title" objects)
	roots=$(field "$title" object_roots)
	if [ -z "$want_digest" ] || [ -z "$roots" ]; then
		echo "verify-refs: '$title' is not a section of $lock" >&2
		exit 2
	fi

	dir=$root/$title
	if [ ! -d "$dir" ]; then
		echo "MISSING  $title -- no checkout at $dir"
		rc=1
		continue
	fi

	# $roots is deliberately unquoted: a title may have several object roots
	# (Banjo-Kazooie builds its ultralib in a second tree).
	# shellcheck disable=SC2086
	if ! result=$("$PY" "$repo/tools/reference_build_digest.py" "$dir" $roots); then
		echo "UNBUILT  $title -- no objects to hash under $roots;" \
			"run tools/setup_reference_builds.sh"
		rc=1
		continue
	fi
	got_digest=${result%% *}
	got_count=${result##* }

	if [ "$mode" = print ]; then
		echo "digest       = $got_digest   # $title, $got_count objects"
		continue
	fi

	got_commit=$(git -C "$dir" rev-parse HEAD 2>/dev/null || echo unknown)
	if [ "$got_commit" != "$want_commit" ]; then
		echo "MISMATCH $title -- checkout is at $got_commit, lock pins $want_commit"
		rc=1
		continue
	fi
	if [ "$got_digest" != "$want_digest" ]; then
		echo "MISMATCH $title -- $got_count objects hash to $got_digest"
		echo "         lock records $want_count objects hashing to $want_digest"
		rc=1
		continue
	fi
	echo "OK       $title -- $got_count objects, digest matches the lock"
done

if [ "$mode" = print ]; then
	exit 0
fi
if [ "$rc" -ne 0 ]; then
	cat >&2 <<'EOF'

reference-build check FAILED.

A title that does not match the lock is not the build the tier-A names in
docs/references.md were mined from, so nothing in that farm re-derives those
names.  Rebuild it with tools/setup_reference_builds.sh, or -- if the farm
changed on purpose -- re-mine and update the lock and docs/references.md
together.  Do not edit a digest to match a build you have not re-mined from.
EOF
	exit 1
fi
echo "reference-build check OK -- every checked title matches the lock"
