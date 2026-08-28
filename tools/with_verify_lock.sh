#!/bin/sh
# Run one command while holding a single global lock, for the two-phase
# `gmake verify` (and other full-tree builds) in a worktree/build directory
# shared by multiple agents. Unlike tools/with_compile_token.sh (N parallel
# compile slots), this is a strict mutex: only one holder at a time, because
# a concurrent second `verify`/full build against the same build/ directory
# would race the same output files.
#
# Usage: tools/with_verify_lock.sh <cmd> [args ...]

set -eu

if [ "$#" -eq 0 ]; then
    echo "usage: $0 command [args ...]" >&2
    exit 2
fi

lock_dir=${MICKEY_VERIFY_LOCK_DIR:-build/wb/.verify-lock}
mkdir -p "$(dirname "$lock_dir")"

held=0
cleanup_lock() {
    if [ "$held" -eq 1 ]; then
        rm -f "$lock_dir/pid"
        rmdir "$lock_dir" 2>/dev/null || true
    fi
}
trap cleanup_lock EXIT HUP INT TERM

while [ "$held" -eq 0 ]; do
    if mkdir "$lock_dir" 2>/dev/null; then
        printf '%s\n' "$$" > "$lock_dir/pid"
        held=1
        break
    fi

    # Reclaim only a fully initialized lock whose owning process is gone. A
    # just-created directory without a pid is left alone to avoid racing its
    # owner between mkdir and the pid write.
    if [ -f "$lock_dir/pid" ]; then
        owner=$(sed -n '1p' "$lock_dir/pid" 2>/dev/null || true)
        case "$owner" in
            ''|*[!0-9]*) ;;
            *)
                if ! kill -0 "$owner" 2>/dev/null; then
                    rm -f "$lock_dir/pid"
                    rmdir "$lock_dir" 2>/dev/null || true
                fi
                ;;
        esac
    fi

    sleep 1
done

set +e
"$@"
status=$?
set -e

cleanup_lock
held=0
trap - EXIT HUP INT TERM
exit "$status"
