#!/usr/bin/env bash
# Build ctx.c: every project header, preprocessed into one flat file, so m2c
# can resolve real struct/type names instead of guessing. Regenerate whenever
# headers change; ./mips_to_c.sh picks it up automatically if it is present.
set -euo pipefail
cd "$(dirname "$0")"

# `&` (the whole match), not `\0`: \0 is a GNU sed extension, and BSD sed --
# which is what macOS ships, and this project documents gmake/macOS -- writes
# a literal "0" instead, producing a file full of `#include "0"`.
find include src -type f -name '*.h' | sed -e 's|.*|#include "&"|' > ctx_includes.c
.venv/bin/python tools/m2ctx.py ctx_includes.c
