#!/usr/bin/env bash
# Clean-room enforcement sweep (see docs/CLEANROOM.md): fails the build if
# anything that looks like a ROM, extracted asset, or opaque binary blob has
# ended up tracked in git. Meant to be cheap and paranoid, not exhaustive --
# it is a safety net, not a substitute for reviewing diffs.
set -u
cd "$(dirname "$0")/.."

fail=0

# 1. Filename-pattern sweep: paths/extensions the clean-room policy says
#    must never be committed (mirrors the .gitignore clean-room section --
#    ROM images, and anything extracted/derived from one).
pattern_hits=$(git ls-files | grep -iE '\.(z64|n64|v64|bin)$|(^|/)baseroms/|(^|/)asm/|(^|/)assets/|(^|/)expected/' || true)
if [ -n "$pattern_hits" ]; then
  echo "cleanroom: tracked files matching ROM/asset patterns:" >&2
  echo "$pattern_hits" | sed 's/^/  /' >&2
  fail=1
fi

# 2. Binary-blob sweep: every tracked file should be text. git ls-files -s
#    also lists submodule gitlinks (mode 160000), which have no blob in this
#    repo's working tree to inspect, so those are skipped. Empty files are
#    allowed regardless of what `file` guesses for them.
while IFS=$'\t' read -r meta path; do
  mode=${meta%% *}
  [ "$mode" = "160000" ] && continue
  [ -f "$path" ] || continue
  [ -s "$path" ] || continue
  encoding=$(file -b --mime-encoding -- "$path")
  if [ "$encoding" = "binary" ]; then
    echo "cleanroom: tracked file '$path' looks binary (file: $encoding)" >&2
    fail=1
  fi
done < <(git ls-files -s)

if [ "$fail" -ne 0 ]; then
  echo "cleanroom check FAILED -- see above" >&2
  exit 1
fi
echo "cleanroom check OK -- no tracked ROM/asset/binary files found"
