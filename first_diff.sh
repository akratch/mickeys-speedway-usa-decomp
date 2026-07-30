#!/usr/bin/env bash
# Report the first byte where the built ROM diverges from the baserom, and
# which object it lands in. Only useful once `gmake verify` fails.
set -euo pipefail
cd "$(dirname "$0")"

exec .venv/bin/python tools/first_diff.py "$@"
