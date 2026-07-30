#!/usr/bin/env bash
# Verifies any baseroms present against known-good SHA1s. Exits nonzero on mismatch.
set -u

want() {
  case "$1" in
    mickey.us.z64) echo 507341c0a40ca3e9a7cee969b396ee53facfb548 ;;
    mickey.pal.z64) echo c583ed998a6b422a22ffd3f8376c3cef0c3710d9 ;;
    mickey.jpn.z64) echo 5b4f7bad6591de2199c095352a811a2eb7fc6f53 ;;
  esac
}

rc=1
for f in mickey.us.z64 mickey.pal.z64 mickey.jpn.z64; do
  p="$(dirname "$0")/../baseroms/$f"
  [ -f "$p" ] || continue
  got=$(shasum -a 1 "$p" | cut -d' ' -f1)
  if [ "$got" = "$(want "$f")" ]; then echo "OK  $f"; rc=0
  else echo "BAD $f (got $got)"; exit 2; fi
done
exit $rc
