#!/bin/sh
# Run one low-priority compiler/build command while holding one of the two
# workstation-wide compiler tokens. All decomp agents share build/wb, so this
# keeps the agent pool saturated without exceeding the Mac's two-job ceiling.

set -eu

if [ "$#" -eq 0 ]; then
    echo "usage: $0 command [args ...]" >&2
    exit 2
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
    for number in 1 2; do
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
    done

    if [ -z "$token" ]; then
        sleep 1
    fi
done

set +e
nice -n 10 "$@"
status=$?
set -e

cleanup_token
token=
trap - EXIT HUP INT TERM
exit "$status"
