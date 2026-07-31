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

# 3. Content sweep: a file can pass 1 and 2 -- plain text, innocuous name --
#    and still be a dump of the ROM's instructions. That is how two workbench
#    ledger.jsonl files landed in this history: JSON whose diff-site records
#    quoted the target disassembly verbatim, enough to reconstruct 129 of one
#    function's 146 instructions.
#
#    The tell is many distinctive MIPS mnemonics packed densely. Neither half
#    alone discriminates: prose that discusses one `jal` in a small header is
#    dense but tiny, and a large document may mention a handful of mnemonics
#    without being a dump. So both must trip -- an absolute count AND a rate.
#    Measured on this tree, the widest-margin tracked file carries 16 mnemonic
#    tokens; the smaller of the two purged ledgers carried 88 at 1.3/KiB.
MNEMONICS='addiu|lw|sw|jal|beq|lui|sltu'
DENSITY_LIMIT_MILLI=1000   # tokens per KiB, x1000 to stay in integer math
COUNT_LIMIT=40             # absolute tokens; below this, density is noise

while IFS=$'\t' read -r meta path; do
  mode=${meta%% *}
  [ "$mode" = "160000" ] && continue
  [ -f "$path" ] || continue
  [ -s "$path" ] || continue
  [ "$(file -b --mime-encoding -- "$path")" = "binary" ] && continue

  # tr splits on any non-identifier character, so grep -x matches whole tokens
  # only: "jal" in prose counts, "jalr" and "swap" do not. Portable where GNU
  # grep's \b is not.
  count=$(tr -cs 'A-Za-z0-9_' '\n' < "$path" | grep -cxE "$MNEMONICS" || true)
  [ "$count" -ge "$COUNT_LIMIT" ] || continue

  bytes=$(wc -c < "$path" | tr -d '[:space:]')
  density=$(( count * 1024 * 1000 / bytes ))
  if [ "$density" -ge "$DENSITY_LIMIT_MILLI" ]; then
    echo "cleanroom: tracked file '$path' reads as an instruction dump:" >&2
    echo "  $count MIPS mnemonics in ${bytes}B = $((density / 1000)).$(printf '%03d' $((density % 1000))) per KiB" >&2
    echo "  (flagged when count >= $COUNT_LIMIT and rate >= $((DENSITY_LIMIT_MILLI / 1000)).$(printf '%03d' $((DENSITY_LIMIT_MILLI % 1000))) per KiB)" >&2
    fail=1
  fi
done < <(git ls-files -s)

if [ "$fail" -ne 0 ]; then
  echo "cleanroom check FAILED -- see above" >&2
  exit 1
fi
echo "cleanroom check OK -- no tracked ROM/asset/binary files, no instruction dumps"
