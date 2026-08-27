#!/usr/bin/env bash
# One decomp-permuter run for one function.
#
#   ./tools/permute.sh <function> [permuter.py args...]
#
# Finds the function's C file and its target .s, imports both into a scratch
# directory under build/permuter/<function>/ (gitignored: build/ is never
# tracked), then runs permuter.py against it with a wall-clock cap.
#
# SCRATCH FIDELITY: the scratch object must be bit-identical to the real
# per-TU object, or a score-0 in the scratch is a "false ceiling" that does
# not transfer to `gmake verify`. Two corrections are applied to the importer's
# scratch (see docs/permute-batch.md "Scratch fidelity"):
#   - Real per-file compile flags (-mips2, -Wab,-r4300_mul, ...) recovered from
#     `gmake -n <obj>` replace the importer's -mips1 default.
#   - Any post-compile `objcopy --redefine-sym/--add-symbol` the Makefile's
#     POSTPROCESS applies to that .o (e.g. src/main/track.c) is appended to the
#     scratch's compile.sh after cc, so the scratch object gets the same symbol
#     rewrite. Digest-guarded ELF surgery (add_elf_relocations/trim_elf_section)
#     is NOT replicated -- it is tied to the matched bytes and would abort on a
#     permuted object; such TUs are flagged with a warning instead.
#   - --stack-diffs is forced on: the permuter's scorer otherwise normalizes
#     stack-relative offsets and reports a false 0 for a candidate that is only
#     a spill/local at the wrong sp offset -- bytes that still fail verify.
#   NOT yet corrected: gfx-macro TUs whose real headers expand gDP*/gSP*/_SHIFTL
#   differently from the importer's latedefine stubs (see the block near the end
#   of this file's flag-correction logic and docs/permute-batch.md).
#
# MACHINE SAFETY: the permuter is niced (-n 15), wall-clock capped (timeout),
# and its worker count is capped at 4 (many -j workers freeze the machine).
# Keep those three intact.
#
#   PERMUTE_MINUTES=20 ./tools/permute.sh <function>   # default cap: 20 min
#   ./tools/permute.sh <function> -j 4                 # override -j
#
# Prints the base score, the best score found, and a diff of the winning
# source against the function's current C, if any improvement was found.
#
# TWO CASES FOR THE TARGET .s, per docs/tools.md:
#
#   1. The function is still `#pragma GLOBAL_ASM(...)`: splat's own
#      asm/nonmatchings/**/<function>.s is exactly the target and is used
#      as-is.
#
#   2. The function is already implemented in C (matching or not): splat
#      stops emitting that function's .s once a C definition exists for it
#      (Makefile's own comment on $(SPLAT_STAMP) touches this), so there is
#      no asm/nonmatchings file to read. This script recovers the target by
#      temporarily replacing the function's C body with a GLOBAL_ASM pragma,
#      forcing `gmake extract` to re-emit the .s from the *baserom* (not the
#      built object -- splat disassembles the ROM directly, so this is exact
#      regardless of the C's current match status), capturing that file, and
#      restoring the original C. Nothing under asm/ is ever committed
#      (gitignored wholesale), and the source file is restored before this
#      script exits either way (including on error, via the trap below).
set -euo pipefail
cd "$(dirname "$0")/.."

if [ $# -lt 1 ]; then
    echo "usage: $0 <function> [permuter.py args...]" >&2
    exit 2
fi
func=$1; shift

PYTHON=.venv/bin/python
IMPORT=tools/permuter/import.py
PERMUTER=tools/permuter/permuter.py
OUT=build/permuter/$func
MINUTES=${PERMUTE_MINUTES:-20}

mkdir -p "$OUT"

# --- Locate the function's C file ---------------------------------------
# A plain grep for "func(" matches call sites as readily as the definition
# (e.g. contpfs.c calls __osContRamRead 11 times; contramread.c defines it
# once), so require the definition shape: name, arg list, then a `{` -- the
# same pattern tools/permute.sh's splat-re-extract step below uses to find
# the function body. Falls back to a GLOBAL_ASM pragma referencing
# .../<func>.s, for functions with no C definition at all yet.
c_file=$("$PYTHON" - "$func" <<'PYEOF'
import re, sys, pathlib
func = sys.argv[1]
def_pat = re.compile(r'^[A-Za-z_][A-Za-z0-9_ \t\*]*\b' + re.escape(func) + r'\s*\([^;{]*\)\s*\{', re.MULTILINE)
pragma_pat = re.compile(re.escape(func) + r'\.s"\)')
def_hit = pragma_hit = None
for p in sorted(pathlib.Path('src').rglob('*.c')):
    text = p.read_text(errors='replace')
    if def_pat.search(text):
        def_hit = p
        break
    if pragma_hit is None and pragma_pat.search(text):
        pragma_hit = p
print(def_hit or pragma_hit or '')
PYEOF
)
if [ -z "$c_file" ]; then
    echo "$0: no src/**/*.c defines '$func' or GLOBAL_ASMs a .../$func.s." >&2
    exit 1
fi
echo "C file: $c_file"

# --- Locate (or regenerate) the target .s -------------------------------
asmfile=$(find asm/nonmatchings -type f -name "$func.s" 2>/dev/null | head -1 || true)

restore_c=""
cleanup() {
    if [ -n "$restore_c" ]; then
        mv "$restore_c" "$c_file"
        echo "Restored $c_file"
    fi
}
trap cleanup EXIT

if [ -z "$asmfile" ]; then
    echo "No asm/nonmatchings/**/$func.s -- function is already implemented in" \
         "C. Regenerating the target from the baserom (splat re-extract)." >&2

    backup=$(mktemp "${TMPDIR:-/tmp}/permute-$func.XXXXXX.c")
    cp "$c_file" "$backup"
    restore_c=$backup

    before=$(find asm/nonmatchings -type f -name '*.s' | sort)

    "$PYTHON" - "$c_file" "$func" <<'PYEOF'
import re, sys
path, func = sys.argv[1], sys.argv[2]
src = open(path).read()
# Match a plausible function definition: return type(s), the name, a
# parenthesised argument list, then a brace. Balance braces by hand since
# regex can't nest.
pat = re.compile(
    r'^[A-Za-z_][A-Za-z0-9_ \t\*]*\b' + re.escape(func) + r'\s*\([^;{]*\)\s*\{',
    re.MULTILINE,
)
m = pat.search(src)
if not m:
    sys.exit(f"error: could not find a definition of {func}() in {path}")
start = m.start()
depth = 0
i = m.end() - 1  # at the opening brace
while True:
    if src[i] == '{':
        depth += 1
    elif src[i] == '}':
        depth -= 1
        if depth == 0:
            break
    i += 1
end = i + 1
guess_path = f'asm/nonmatchings/GUESS/{func}.s'
pragma = f'#pragma GLOBAL_ASM("{guess_path}")\n'
open(path, 'w').write(src[:start] + pragma + src[end:])
PYEOF

    gmake extract >/dev/null

    after=$(find asm/nonmatchings -type f -name '*.s' | sort)
    new_file=$(comm -13 <(echo "$before") <(echo "$after") | head -1)

    if [ -z "$new_file" ]; then
        echo "$0: splat did not emit a new .s for '$func' -- is it really" \
             "the whole body of a function, with no earlier code in the" \
             "same translation unit that also changed status? Check" \
             "$c_file by hand." >&2
        exit 1
    fi
    echo "Regenerated target: $new_file"
    asmfile=$OUT/target.s
    cp "$new_file" "$asmfile"

    # Put the tree back before running import.py/permuter.py, so those see
    # the real (matching or not) C, not the stub -- only the captured target
    # .s is needed from here on.
    mv "$backup" "$c_file"
    restore_c=""
    gmake extract >/dev/null
fi
echo "Target asm: $asmfile"

# --- Import into the permuter scratch dir -------------------------------
rm -rf "$OUT/scratch"
"$PYTHON" "$IMPORT" "$c_file" "$asmfile" 2>&1 | tee "$OUT/import.log"
imported=$(grep -oE 'Imported into \S+' "$OUT/import.log" | awk '{print $3}')
if [ -z "$imported" ] || [ ! -d "$imported" ]; then
    echo "$0: import.py did not report a scratch directory; see $OUT/import.log" >&2
    exit 1
fi
mv "$imported" "$OUT/scratch"
echo "Scratch: $OUT/scratch"

# --- Correct the scratch compile flags to the project's real per-file flags ---
# decomp-permuter's import.py infers a default (-mips1, no per-file overrides);
# the real build uses -mips2 plus per-file CFLAGS. Compiling the search at the
# wrong ISA searches the wrong instruction space, so rewrite compile.sh's cc
# flags to exactly what `gmake` uses for this object.
obj="build/${c_file}.o"
csh="$OUT/scratch/compile.sh"
# `gmake -n "$obj"` prints nothing when the object is already up to date, which
# would silently leave the scratch at the importer's wrong -mips1 (the search
# then explores the wrong instruction space and never matches). Touch the source
# so the object is always stale and gmake emits its real compile command.
touch "$c_file" 2>/dev/null || true
dryrun=$(gmake -n "$obj" 2>/dev/null)
realflags=$(printf '%s\n' "$dryrun" | grep -oE '\-mips[0-9]|\-O[0-9]|\-Wo,[^ ]*|\-Wab,[^ ]*|\-g[0-9]?' | sort -u | tr '\n' ' ')
if printf '%s' "$realflags" | grep -q mips; then
  # drop the importer's -O/-mips/-Wo/-Wab flags, splice in the real ones before the input
  sed -i '' -E 's/ -O[0-9]//g; s/ -mips[0-9]//g; s/ -Wo,[^ ]*//g; s/ -Wab,[^ ]*//g' "$csh"
  sed -i '' -E "s#(tools/ido/cc )#\1$realflags #" "$csh"
  echo "Corrected compile flags -> $realflags"
else
  echo "WARNING: could not recover real compile flags for $obj; scratch stays at import defaults (-mips1). The search may explore the wrong ISA." >&2
fi

# --- Replicate the TU's post-compile objcopy steps ----------------------
# Some TUs apply an `objcopy --redefine-sym A=B` (or --add-symbol) to the object
# AFTER cc, via the Makefile's per-file POSTPROCESS -- e.g. src/main/track.c gets
#   objcopy --redefine-sym trackCamPosTrap=TrapDanglingJump build/src/main/track.c.o
# (the func_8000D018 TrapDanglingJump fix). import.py's scratch runs cc only, so
# the scratch object differs from the real per-TU object and a score-0 in the
# scratch does NOT transfer to the real build -- a "false ceiling" that wastes
# ~an hour per function (func_80012574 in track.c scored 0 in scratch yet was
# 2-4 words off in the real build). Recover the same post-compile step from the
# `gmake -n` output already captured above and append it to compile.sh, after
# cc, retargeted from the real object path to the scratch's "$OUTPUT".
#
# The compile line names $obj but invokes cc/as (never objcopy), so selecting
# the dry-run lines that reference $obj AND invoke objcopy isolates the
# POSTPROCESS recipe. (Note $obj is a superstring of $c_file, so the source name
# cannot be used to exclude the compile line.) gmake prints the whole (possibly
# `&&`-chained) POSTPROCESS on one physical line. Only pure objcopy chains are
# safe to replicate: symbol renames survive permutation, whereas digest-guarded
# ELF surgery (add_elf_relocations.py's text-size/SHA guard, trim_elf_section.py)
# is tied to the *matched* bytes and would abort on a permuted object. If such a
# step is present we skip it and warn the scratch may not be bit-identical.
postproc=$(printf '%s\n' "$dryrun" | grep -F "$obj" | grep -i objcopy || true)
if [ -n "$postproc" ]; then
  if printf '%s\n' "$postproc" | grep -qE 'add_elf_relocations|trim_elf_section|rebind_elf_relocations|\.py\b'; then
    echo "WARNING: $obj has a digest-guarded post-compile ELF pass (not objcopy-only)." \
         "The scratch object may differ from the real object; a score-0 might not transfer." >&2
  else
    # Retarget every occurrence of the real object path to the scratch's $OUTPUT.
    remapped=$(printf '%s\n' "$postproc" | sed "s#${obj}#\"\$OUTPUT\"#g")
    # compile.sh's final cc line has no trailing newline; prepend one.
    printf '\n%s\n' "$remapped" >> "$csh"
    echo "Replicated post-compile objcopy -> $remapped"
  fi
fi

# --- Run the permuter, capped, non-interactive --------------------------
ncpu=$(sysctl -n hw.ncpu 2>/dev/null || nproc)
jobs=$(( ncpu > 6 ? 4 : (ncpu > 2 ? ncpu - 2 : 1) ))  # cap at 4: many -j workers freeze the machine

has_j=0
has_stackdiffs=0
for a in "$@"; do
    [ "$a" = "-j" ] && has_j=1
    [ "$a" = "--stack-diffs" ] && has_stackdiffs=1
done

# --stack-diffs is ESSENTIAL for exact-match decomp. decomp-permuter's scorer
# defaults to stack_differences=False (src/scorer.py / src/objdump.py), which
# NORMALIZES stack-relative offsets before diffing -- so a candidate whose only
# residual is a spill/local at the wrong sp offset (e.g. sw v1,0x18(sp) vs
# 0x1C(sp)) scores 0 even though those bytes differ and `gmake verify` fails.
# That is a false ceiling: the permuter reports a match that does not transfer
# (observed on track.c func_80012574 -- its winning candidate was 4 stack-home
# words off yet scored 0 without this flag). For a byte-identical rebuild the
# stack offsets ARE part of the match, so always score them.
args=(--stop-on-zero --quiet)
[ "$has_stackdiffs" -eq 0 ] && args+=(--stack-diffs)
[ "$has_j" -eq 0 ] && args+=(-j "$jobs")
args+=("$@")

echo "Running permuter.py ${args[*]} for up to ${MINUTES} minutes..."
set +e
nice -n 15 timeout "${MINUTES}m" "$PYTHON" "$PERMUTER" "${args[@]}" "$OUT/scratch" \
    2>&1 | tee "$OUT/permuter.log"
status=$?
set -e
if [ "$status" -eq 124 ]; then
    echo "(stopped at the ${MINUTES}-minute cap)"
fi

echo
echo "=== $func: summary ==="
base=$(grep -oE 'base score = [0-9]+' "$OUT/permuter.log" | head -1 | grep -oE '[0-9]+')
echo "base score: ${base:-unknown}"

# Every improvement permuter.py finds gets written to
# $OUT/scratch/output-<score>-<n>/{source.c,score.txt,diff.txt} (write_candidate()
# in decomp-permuter's src/main.py). The lowest <score> among these is the
# best result of this run; take its own diff.txt (already computed by the
# permuter against the base candidate) rather than re-diffing here.
best_dir=$(find "$OUT/scratch" -maxdepth 1 -type d -name 'output-*' 2>/dev/null \
    | awk -F- '{print $2, $0}' | sort -n | head -1 | cut -d' ' -f2-)
if [ -n "$best_dir" ]; then
    best_score=$(cat "$best_dir/score.txt")
    echo "best score: $best_score  (from $best_dir)"
    echo
    echo "--- diff: winning candidate vs base ---"
    cat "$best_dir/diff.txt"
else
    echo "best score: no improvement over the base found in this run"
fi
