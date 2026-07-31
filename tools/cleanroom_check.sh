#!/usr/bin/env bash
# Clean-room enforcement sweep (see docs/CLEANROOM.md): fails if anything that
# looks like a ROM, an extracted asset, an instruction dump, or an opaque blob
# has ended up tracked in git.  Paranoid and cheap by design -- a safety net,
# not a substitute for reading your own diff.
#
#   tools/cleanroom_check.sh                  scan the worktree (default)
#   tools/cleanroom_check.sh --staged         scan the index, i.e. what a
#                                             commit right now would contain
#   tools/cleanroom_check.sh --range A..B     scan every tree in a commit range
#
# This script decides WHAT to look at and hands the work list to
# tools/cleanroom_detectors.py, which decides WHETHER IT IS ROM CONTENT.  The
# split exists so all three modes share one set of detectors and one set of
# measured thresholds; adding a detector should never mean teaching three
# code paths about it.
#
# --range scans the *whole tree* of every commit in the range, not just the
# changed files, so a bad file that slipped in earlier is caught the next time
# anything is pushed.  That is affordable because the detectors deduplicate by
# blob SHA: an unchanged file is inspected once no matter how many commits it
# appears in.  Full history of this repo (52 commits, 183 distinct blobs) scans
# in well under a second.

set -u
cd "$(dirname "$0")/.." || exit 2

usage() {
	cat <<'EOF'
usage: tools/cleanroom_check.sh [--staged | --range <rev-range>]

  (no arguments)      scan tracked files as they exist in the worktree
  --staged            scan the staged index (what `git commit` would record)
  --range <rev-range> scan every commit tree in the range, e.g. HEAD~5..HEAD
                      or origin/master..HEAD.  Accepts anything git rev-list
                      accepts, including --all.

Exits 0 when clean, 1 on a clean-room finding, 2 on a usage/plumbing error.
EOF
}

mode=worktree
range=

while [ $# -gt 0 ]; do
	case "$1" in
	--staged)
		mode=staged
		shift
		;;
	--range)
		mode=range
		range=${2-}
		if [ -z "$range" ]; then
			echo "cleanroom: --range needs a rev-range argument" >&2
			exit 2
		fi
		shift 2
		;;
	--range=*)
		mode=range
		range=${1#--range=}
		shift
		;;
	-h | --help)
		usage
		exit 0
		;;
	*)
		echo "cleanroom: unknown argument '$1'" >&2
		usage >&2
		exit 2
		;;
	esac
done

# Emit work items for the detectors: kind, ident, label, path (tab separated,
# path last so a path containing a tab survives the split).
#
#   kind=file  ident is a worktree path -- read the bytes off disk
#   kind=blob  ident is a git blob SHA  -- read the bytes out of the object db
#   kind=link  a submodule gitlink      -- path checks only, no bytes here
emit_index() {
	# `git ls-files -s` prints "<mode> <sha> <stage>\t<path>", NUL-terminated
	# under -z so no path needs quoting or unquoting.
	local want_kind=$1 label=$2 meta path filemode sha kind
	while IFS= read -r -d '' entry; do
		meta=${entry%%$'\t'*}
		path=${entry#*$'\t'}
		filemode=${meta%% *}
		meta=${meta#* }
		sha=${meta%% *}
		if [ "$filemode" = "160000" ]; then
			printf 'link\t%s\t%s\t%s\n' "$sha" "$label" "$path"
			continue
		fi
		kind=$want_kind
		# Worktree mode falls back to the index blob for a tracked file that
		# is missing from disk (a deletion staged elsewhere, a sparse
		# checkout), so the scan never silently skips it.
		if [ "$kind" = file ] && [ ! -f "$path" ]; then
			kind=blob
		fi
		if [ "$kind" = file ]; then
			printf 'file\t%s\t%s\t%s\n' "$path" "$label" "$path"
		else
			printf 'blob\t%s\t%s\t%s\n' "$sha" "$label" "$path"
		fi
	done < <(git ls-files -sz)
}

emit_range() {
	# `git ls-tree -r -z` prints "<mode> <type> <sha>\t<path>".
	local commit short meta path filemode type sha
	while IFS= read -r commit; do
		[ -n "$commit" ] || continue
		short=${commit:0:9}
		while IFS= read -r -d '' entry; do
			meta=${entry%%$'\t'*}
			path=${entry#*$'\t'}
			filemode=${meta%% *}
			meta=${meta#* }
			type=${meta%% *}
			sha=${meta##* }
			if [ "$filemode" = "160000" ] || [ "$type" = commit ]; then
				printf 'link\t%s\tcommit %s\t%s\n' "$sha" "$short" "$path"
			else
				printf 'blob\t%s\tcommit %s\t%s\n' "$sha" "$short" "$path"
			fi
		done < <(git ls-tree -r -z "$commit")
		# $range is deliberately unquoted: callers pass rev-list syntax, which
		# may be several words ("--all", "A..B C..D").
		# shellcheck disable=SC2086
	done < <(git rev-list $range)
}

case "$mode" in
worktree) worklist() { emit_index file worktree; } ;;
staged) worklist() { emit_index blob staged; } ;;
range) worklist() { emit_range; } ;;
esac

# The detectors are stdlib-only, so plain python3 is enough -- no venv, which
# matters because the git hooks run before anyone has necessarily run `gmake
# setup` in this shell.
PY=${PYTHON:-python3}
if ! command -v "$PY" >/dev/null 2>&1; then
	echo "cleanroom: $PY not found; cannot run the clean-room detectors" >&2
	exit 2
fi

count=0
tmp=$(mktemp) || exit 2
trap 'rm -f "$tmp"' EXIT
worklist >"$tmp"
count=$(wc -l <"$tmp" | tr -d '[:space:]')

if [ "$count" = "0" ]; then
	echo "cleanroom check OK -- nothing to scan ($mode)"
	exit 0
fi

if ! "$PY" tools/cleanroom_detectors.py <"$tmp"; then
	status=$?
	if [ "$status" -gt 1 ]; then
		exit "$status"
	fi
	cat >&2 <<'EOF'

cleanroom check FAILED.

Nothing ROM-derived may be tracked in this repository: no disassembly, no
instruction text, no hexdumps, no extracted assets, no workbench ledgers.
See docs/CLEANROOM.md.

If a file above is a false positive, do not weaken the threshold to make it
pass -- restructure the file, or add it to the allowlist in
tools/cleanroom_detectors.py with a written reason.  Never bypass this with
`git commit --no-verify` or `git push --no-verify`.
EOF
	exit 1
fi

case "$mode" in
worktree) echo "cleanroom check OK -- $count tracked files clean (worktree)" ;;
staged) echo "cleanroom check OK -- $count staged files clean (index)" ;;
range) echo "cleanroom check OK -- $count tree entries clean ($range)" ;;
esac
