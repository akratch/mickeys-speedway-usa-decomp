#!/usr/bin/env bash
# asm-differ entrypoint: ./diff.sh [flags] <symbol>
#
# Uses the project venv rather than the host python3 -- asm-differ needs
# colorama/watchdog/levenshtein/cxxfilt, which `gmake setup` installs there
# via requirements.txt. Configuration lives in diff_settings.py.
set -euo pipefail
cd "$(dirname "$0")"
exec .venv/bin/python tools/asm-differ/diff.py "$@"
