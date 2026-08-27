#!/bin/sh
# Run one compiler/build command while holding one of N shared compile
# slots. Concurrent builds may share build/wb, so this keeps them from
# oversubscribing the machine's cores without capping the pool below what the
# hardware can actually do.
#
# Slot count defaults to `sysctl -n hw.ncpu` minus 2 (leaving headroom for
# the interactive workstation); override with MICKEY_COMPILE_SLOTS. This is
# a plain mutex over N slots, not a priority or scheduling policy: it does
# not nice(1) the wrapped command.
#
# Usage: tools/with_compile_token.sh <cmd> [args ...]

set -eu

if [ "$#" -eq 0 ]; then
    echo "usage: $0 command [args ...]" >&2
    exit 2
fi

slots=${MICKEY_COMPILE_SLOTS:-}
if [ -z "$slots" ]; then
    ncpu=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
    slots=$((ncpu - 2))
    [ "$slots" -ge 1 ] || slots=1
fi

token_root=${MICKEY_COMPILE_TOKEN_DIR:-build/wb/.compiler-tokens}
mkdir -p "$token_root"

token=
cleanup_token() {
    if [ -n "$token" ]; then
        rm -f "$token/pid" "$token/cwd"
        rmdir "$token" 2>/dev/null || true
    fi
}
trap cleanup_token EXIT HUP INT TERM

while [ -z "$token" ]; do
    number=1
    while [ "$number" -le "$slots" ]; do
        candidate="$token_root/slot-$number"
        if mkdir "$candidate" 2>/dev/null; then
            token=$candidate
            printf '%s\n' "$$" > "$token/pid"
            pwd > "$token/cwd"
            break
        fi

        # Reclaim only a fully initialized token whose owning process is gone.
        # A just-created directory without a pid is left alone to avoid racing
        # its owner between mkdir and the pid write.
        if [ -f "$candidate/pid" ]; then
            # The owner may finish between the existence check and read. Treat
            # that normal cleanup race as an empty observation and retry.
            owner=$(sed -n '1p' "$candidate/pid" 2>/dev/null || true)
            case "$owner" in
                ''|*[!0-9]*) ;;
                *)
                    if ! kill -0 "$owner" 2>/dev/null; then
                        rm -f "$candidate/pid" "$candidate/cwd"
                        rmdir "$candidate" 2>/dev/null || true
                    fi
                    ;;
            esac
        fi

        number=$((number + 1))
    done

    if [ -z "$token" ]; then
        sleep 1
    fi
done

set +e
"$@"
status=$?
set -e

cleanup_token
token=
trap - EXIT HUP INT TERM
exit "$status"
