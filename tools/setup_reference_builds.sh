#!/usr/bin/env bash
# Rebuilds the reference-decomp farm the tier-A names in this tree were mined
# from: clones each title at the commit tools/reference-builds.lock pins, stages
# its baserom out of your own ROM archive after checking the SHA1 the lock
# records, applies the macOS fixes each build needs, builds, and finishes by
# running tools/verify_reference_builds.sh, so the result is compared against
# the lock rather than merely declared done.
#
#   tools/setup_reference_builds.sh                    every locked title
#   tools/setup_reference_builds.sh jfg                one title
#   tools/setup_reference_builds.sh --root DIR         build the farm elsewhere
#   tools/setup_reference_builds.sh --rom-dir DIR      ROM archive elsewhere
#   tools/setup_reference_builds.sh --jobs 8           parallelism
#
# Idempotent: re-running fetches nothing it already has, applies no patch
# twice, and re-uses whatever make still considers current.
#
# Nothing this produces belongs in this repository and nothing is written into
# it -- see docs/CLEANROOM.md.  The per-title notes below are the transcript of
# getting these four to build on macOS, and are the reason this is a script
# rather than a paragraph.  Read docs/references.md for what each one yielded.
#
# Exits 0 when every requested title matches the lock, 1 when one does not, 2
# on a usage, prerequisite or plumbing error.

set -u

die() {
	echo "setup-refs: $*" >&2
	exit 2
}

# Progress goes to stderr: several of the helpers below return a value on
# stdout, and a stray line of narration inside a command substitution is a bug
# that takes an hour to find.
step() { echo "==> $*" >&2; }

run() {
	echo "    \$ $*" >&2
	"$@"
}

repo=$(cd "$(dirname "$0")/.." && pwd) || exit 2
lock=$repo/tools/reference-builds.lock
root=${REFS_ROOT:-$HOME/Desktop/dev/decomp-refs}
roms=${REFS_ROM_DIR:-$HOME/Documents/Minerva_Myrient/No-Intro/Nintendo - Nintendo 64 (BigEndian)}
jobs=4
titles=

# Jet Force Gemini's US dump is not in the No-Intro BigEndian set above, which
# carries only the Europe and Kiosk builds.  Point this at any archive or raw
# ROM holding the US one; the SHA1 in the lock is what decides.
jfg_rom=${REFS_JFG_ROM:-$HOME/Desktop/dev/Diddy-Kong-Racing/Jet Force Gemini (USA).zip}

# The IDO compilers.  Each of these four repos ships or builds its own IDO
# static recompilation and each of them assumes Linux; decompals publishes
# macOS builds of the same release, and using those is the single fix that
# makes this farm possible at all.  Pinned and checksummed because a truncated
# download of exactly this tarball once produced a `cc` that was the right size
# and SIGKILLed on every invocation, including --version.
IDO_RELEASE=https://github.com/decompals/ido-static-recomp/releases/download/v1.2
IDO_53_SHA256=5b1ca006ee4b158ffba0422fc3f00b9b330f9036f279bae2ea1b4703317ad9c0
IDO_71_SHA256=7f75570ed1ca14e6161b84b10743d440c685f27291bfbfaf87c6a82236553bc6

usage() {
	cat <<'EOF'
usage: tools/setup_reference_builds.sh [options] [title ...]

  --root DIR      where the farm lives (default $REFS_ROOT, else
                  ~/Desktop/dev/decomp-refs)
  --rom-dir DIR   your own ROM archive (default $REFS_ROM_DIR)
  --jobs N        make parallelism (default 4)
  title ...       section names from tools/reference-builds.lock; default all

Exits 0 when every requested title matches the lock, 1 when one does not, 2 on
a usage, prerequisite or plumbing error.
EOF
}

while [ $# -gt 0 ]; do
	case "$1" in
	--root)
		[ $# -ge 2 ] || die "--root needs a directory"
		root=$2
		shift 2
		;;
	--root=*)
		root=${1#--root=}
		shift
		;;
	--rom-dir)
		[ $# -ge 2 ] || die "--rom-dir needs a directory"
		roms=$2
		shift 2
		;;
	--rom-dir=*)
		roms=${1#--rom-dir=}
		shift
		;;
	--jobs)
		[ $# -ge 2 ] || die "--jobs needs a number"
		jobs=$2
		shift 2
		;;
	--jobs=*)
		jobs=${1#--jobs=}
		shift
		;;
	-h | --help)
		usage
		exit 0
		;;
	-*)
		echo "setup-refs: unknown argument '$1'" >&2
		usage >&2
		exit 2
		;;
	*)
		titles="$titles $1"
		shift
		;;
	esac
done

# ---------------------------------------------------------------------------
# Guards and prerequisites
# ---------------------------------------------------------------------------

# Reference material never enters this repository, and this script never writes
# into the two checkouts it does not own.  Both are cheap to assert and
# expensive to get wrong.
mkdir -p "$root" || die "cannot create $root"
root=$(cd "$root" && pwd) || die "cannot enter $root"
case "$root/" in
"$repo"/*) die "refusing to build the farm inside $repo (see docs/CLEANROOM.md)" ;;
"$HOME/Desktop/dev/Diddy-Kong-Racing"/*) die "refusing to build the farm inside the DKR checkout" ;;
esac

[ -f "$lock" ] || die "no lock at $lock"

PY=${PYTHON:-python3}
MAKE=${MAKE:-gmake}
command -v "$PY" >/dev/null 2>&1 || die "$PY not found"
command -v "$MAKE" >/dev/null 2>&1 || die "$MAKE not found (brew install make)"
command -v brew >/dev/null 2>&1 || die "Homebrew not found; these builds are wired to its prefixes"
for tool in git curl tar unzip shasum clang; do
	command -v "$tool" >/dev/null 2>&1 || die "$tool not found"
done
for tool in mips-linux-gnu-as mips-linux-gnu-ld mips-linux-gnu-objcopy; do
	command -v "$tool" >/dev/null 2>&1 ||
		die "$tool not found (brew install mips-linux-gnu-binutils)"
done
# macOS ships a sha1sum that cannot --check, which two of these repos' own
# verify steps use.  GNU coreutils goes in front of it, for this script only.
gnubin=$(brew --prefix coreutils 2>/dev/null)/libexec/gnubin
[ -d "$gnubin" ] || die "GNU coreutils not found (brew install coreutils)"
PATH=$gnubin:$PATH
export PATH

cache=$root/.cache
mkdir -p "$cache/bin" || die "cannot create $cache"
PATH=$cache/bin:$PATH
export PATH

# Same fixed-shape reader as tools/verify_reference_builds.sh.
field() {
	awk -v want="[$1]" -v key="$2" '
		/^\[/ { in_section = ($0 == want); next }
		!in_section || /^#/ { next }
		$1 == key && $2 == "=" { $1 = ""; $2 = ""; sub(/^  */, ""); print; exit }
	' "$lock"
}

if [ -z "$titles" ]; then
	titles=$(sed -n 's/^\[\(.*\)\]$/\1/p' "$lock" | tr '\n' ' ')
fi
[ -n "${titles// /}" ] || die "lock has no titles"
for title in $titles; do
	[ -n "$(field "$title" repo)" ] || die "'$title' is not a section of $lock"
done

# ---------------------------------------------------------------------------
# Shared plumbing
# ---------------------------------------------------------------------------

# Clone or update one title to the commit the lock pins, submodules included.
# The submodules are not optional and their absence is silent: a shallow clone
# of Jet Force Gemini leaves tools/asm-processor empty, asm-processor then
# fails inside a pipeline whose exit status make never sees, and every
# GLOBAL_ASM-wrapped file compiles to a valid, empty object.
checkout() {
	local title=$1 dir=$root/$1 url want
	url=$(field "$title" repo)
	want=$(field "$title" commit)
	[ -n "$url" ] && [ -n "$want" ] || die "$title has no repo/commit in the lock"

	if [ ! -d "$dir/.git" ]; then
		step "$title: cloning $url"
		run git clone "$url" "$dir" || die "$title: clone failed"
	fi
	if [ "$(git -C "$dir" rev-parse HEAD)" != "$want" ]; then
		step "$title: checking out $want"
		git -C "$dir" cat-file -e "$want^{commit}" 2>/dev/null ||
			run git -C "$dir" fetch --tags origin || die "$title: fetch failed"
		run git -C "$dir" checkout --detach "$want" || die "$title: checkout failed"
	fi
	run git -C "$dir" submodule update --init --recursive ||
		die "$title: submodule update failed"
}

# Stage a baserom from an archive or a raw dump.  The SHA1 in the lock is the
# only thing that decides; the source is read, never moved and never modified.
stage_rom() {
	local title=$1 src=$2 dest=$root/$1/$3 want member tmp got
	want=$(field "$title" baserom_sha1)
	member=$(field "$title" baserom)
	[ -n "$want" ] || die "$title has no baserom_sha1 in the lock"

	if [ -f "$dest" ] && [ "$(shasum -a 1 "$dest" | cut -d' ' -f1)" = "$want" ]; then
		return 0
	fi
	[ -f "$src" ] || die "$title: no ROM at '$src' -- supply your own dump (--rom-dir)"

	tmp=$(mktemp -d) || die "mktemp failed"
	case "$src" in
	*.zip) run unzip -o -q -j "$src" "$member" -d "$tmp" || die "$title: cannot unzip '$src'" ;;
	*) cp "$src" "$tmp/$member" || die "$title: cannot copy '$src'" ;;
	esac
	got=$(shasum -a 1 "$tmp/$member" | cut -d' ' -f1)
	[ "$got" = "$want" ] || die "$title: baserom SHA1 is $got, lock wants $want"
	mkdir -p "$(dirname "$dest")"
	mv "$tmp/$member" "$dest" || die "$title: cannot install baserom"
	rm -rf "$tmp"
	step "$title: baserom staged, SHA1 $want"
}

# The macOS IDO binaries, fetched once into the farm's cache and shared by
# every title that needs them.  Checksummed on download, smoke-tested on use.
ido_dir() {
	local version=$1 want=$2 dir=$cache/ido-$version tarball=$cache/ido-$version.tar.gz got
	if [ ! -d "$dir" ]; then
		step "fetching IDO $version (macOS)"
		curl -fsSL -o "$tarball" "$IDO_RELEASE/ido-$version-recomp-macos.tar.gz" ||
			die "cannot download IDO $version"
		got=$(shasum -a 256 "$tarball" | cut -d' ' -f1)
		[ "$got" = "$want" ] || die "IDO $version tarball sha256 is $got, expected $want"
		mkdir -p "$dir"
		tar xf "$tarball" -C "$dir" || die "cannot unpack IDO $version"
		rm -f "$tarball"
	fi
	# Not hypothetical: a bad copy of this exact binary ran to SIGKILL on every
	# invocation while being byte-for-byte the right size.
	"$dir/cc" -v >/dev/null 2>&1
	[ $? -lt 128 ] || die "IDO $version's cc is not runnable; delete $dir and re-run"
	echo "$dir"
}

# Copy the IDO binaries over a repo's own Linux copies, leaving whatever the
# release does not carry alone.
seed_ido() {
	local from=$1 to=$2 name
	mkdir -p "$to" || die "cannot create $to"
	for name in "$from"/*; do
		cp -f "$name" "$to/$(basename "$name")" || die "cannot seed $to"
	done
	chmod +x "$to"/* 2>/dev/null
	return 0
}

# Exact-string edits, idempotent by construction: already-applied is success,
# and neither-string-present is a hard error rather than a silent no-op -- which
# is how a patch survives a repo that has moved on, and fails when it has not.
subst() {
	"$PY" - "$@" <<'EOF' || die "cannot patch $1"
import sys
path, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
text = open(path).read()
if new in text:
    sys.exit(0)
if old not in text:
    sys.stderr.write("setup-refs: %s: the text to patch is not there\n" % path)
    sys.exit(1)
open(path, "w").write(text.replace(old, new))
EOF
}

# Append a block once, keyed on a marker inside it.
append_once() {
	local path=$1 marker=$2 block=$3
	[ -f "$path" ] || die "cannot append to $path: no such file"
	grep -q -- "$marker" "$path" && return 0
	printf '%s\n' "$block" >>"$path" || die "cannot append to $path"
}

# splat's disassembler emits `nonmatching`, `enddlabel` and `endjlabel`; the
# macro.inc files these repos pin predate them, so every non-matching .s file
# fails to assemble with "unrecognized opcode" until they are defined away.
MACRO_STUBS='
.macro nonmatching name, size
.endm

.macro enddlabel label
.endm

.macro endjlabel label
.endm'

# Perfect Dark assembles RSP microcode with armips, which Homebrew does not
# package.  Built into the farm's cache, not onto the system.
armips_from_source() {
	local dir=$cache/armips
	[ -x "$cache/bin/armips" ] && return 0
	command -v cmake >/dev/null 2>&1 || die "armips needs cmake (brew install cmake)"
	command -v ninja >/dev/null 2>&1 || die "armips needs ninja (brew install ninja)"
	step "building armips"
	[ -d "$dir" ] || run git clone --recursive https://github.com/Kingcom/armips "$dir" ||
		die "cannot clone armips"
	run cmake -G Ninja -S "$dir" -B "$dir/build" || die "armips: cmake failed"
	run ninja -C "$dir/build" || die "armips: build failed"
	cp "$dir/build/armips" "$cache/bin/armips" || die "armips: cannot install"
}

# ---------------------------------------------------------------------------
# Per-title builds
# ---------------------------------------------------------------------------

build_jfg() {
	local dir=$root/jfg
	checkout jfg
	stage_rom jfg "$jfg_rom" baseroms/baserom.us.z64
	command -v wget >/dev/null 2>&1 || die "jfg: wget not found (brew install wget)"
	# JFG needs no patching: its own `make setup` detects macOS and fetches the
	# right IDO build itself.  It is the only one of the four that does.
	(cd "$dir" && run "$MAKE" setup) || die "jfg: make setup failed"
	(cd "$dir" && run "$MAKE" extract) || die "jfg: make extract failed"
	(cd "$dir" && run "$MAKE" "-j$jobs") || die "jfg: build failed"
}

build_perfect_dark() {
	local dir=$root/perfect_dark gzip_bin capstone ido53 ido71
	checkout perfect_dark
	stage_rom perfect_dark "$roms/Perfect Dark (USA) (Rev 1).zip" pd.ntsc-final.z64

	gzip_bin=$(brew --prefix gzip 2>/dev/null)/bin/gzip
	[ -x "$gzip_bin" ] || die "perfect_dark: GNU gzip not found (brew install gzip)"
	capstone=$(brew --prefix capstone 2>/dev/null)
	[ -d "$capstone" ] || die "perfect_dark: capstone not found (brew install capstone)"
	command -v armips >/dev/null 2>&1 || armips_from_source

	step "perfect_dark: patching"
	# tools/gzip is a committed Linux x86-64 binary and the asset packer runs it.
	printf '#!/bin/sh\nexec %s "$@"\n' "$gzip_bin" >"$dir/tools/gzip" ||
		die "perfect_dark: cannot write the gzip shim"
	chmod +x "$dir/tools/gzip"
	# gzip_bits.c includes a header that classic gzip trees have and this repo
	# does not; nothing here is built with CRYPT defined, so an empty one does.
	[ -f "$dir/tools/mkrom/crypt.h" ] ||
		printf '/* stub: upstream gzip crypt.h, unused here (nothing defines CRYPT) */\n' \
			>"$dir/tools/mkrom/crypt.h"

	# Apple's standalone cpp is broken for `cpp -P in -o out`, and worse for
	# `cpp -P in | as -o out`: there the failure is invisible, because as
	# succeeds on empty stdin and the object is valid, 932 bytes and has no
	# code in it.  clang -E is a drop-in and -x is mandatory -- clang guesses
	# the language from the extension, and neither .ld nor .s guesses right.
	# pipefail makes the next one of these an error rather than an artifact.
	subst "$dir/Makefile" \
		'	cpp -DROMID=$(ROMID)' \
		'	clang -E -P -x c -DROMID=$(ROMID)'
	subst "$dir/Makefile" \
		' -DROM_SIZE=$(ROM_SIZE) -P ld/pd.ld -o $(B_DIR)/pd.ld' \
		' -DROM_SIZE=$(ROM_SIZE) ld/pd.ld > $(B_DIR)/pd.ld'
	subst "$dir/Makefile" \
		'	cpp -P -Wno-trigraphs -I include' \
		'	clang -E -P -x assembler-with-cpp -Wno-trigraphs -I include'
	subst "$dir/Makefile" \
		'################################################################################
# Options' \
		'SHELL := /bin/bash
.SHELLFLAGS := -o pipefail -c

################################################################################
# Options'

	# PD builds its own IDO recomp from source, which cannot work on Apple
	# silicon: that recompiler wants -no-pie, Mach-O/arm64 has no such thing,
	# and the translated binary's hardcoded addresses land on nothing -- it
	# reports its own argv back as "no such file or directory: 'c'".  Drop the
	# prebuilt macOS binaries in instead, and leave a placeholder where make
	# expects the recompiler it will now never run.
	ido53=$(ido_dir 5.3 "$IDO_53_SHA256") || exit 2
	ido71=$(ido_dir 7.1 "$IDO_71_SHA256") || exit 2
	seed_ido "$ido53" "$dir/build/recomp/5.3"
	seed_ido "$ido71" "$dir/build/recomp/7.1"
	: >>"$dir/build/recomp/recomp"
	chmod +x "$dir/build/recomp/recomp"

	export CPATH="$capstone/include" LIBRARY_PATH="$capstone/lib"
	(cd "$dir" && run "$MAKE" extract) || die "perfect_dark: make extract failed"
	# Recorded outcome is `nearfull`: this links clean and produces a ROM, but
	# `make test` reports packed asset blobs as byte-mismatched, because the
	# host's deflate stream is not the one retail was packed with.  The digest
	# check at the end covers code objects, which are unaffected.
	(cd "$dir" && run "$MAKE" "-j$jobs") || die "perfect_dark: build failed"
}

build_banjo_kazooie() {
	local dir=$root/banjo-kazooie ido53
	checkout banjo-kazooie
	stage_rom banjo-kazooie "$roms/Banjo-Kazooie (USA).zip" baserom.us.v10.z64
	command -v cargo >/dev/null 2>&1 ||
		die "banjo-kazooie: cargo not found (brew install rust); bk_rom_decompress is Rust and extraction needs it"

	step "banjo-kazooie: patching"
	append_once "$dir/include/macro.inc" '.macro nonmatching' "$MACRO_STUBS"
	ido53=$(ido_dir 5.3 "$IDO_53_SHA256") || exit 2
	seed_ido "$ido53" "$dir/ido/ido5.3_recomp"
	# ultralib downloads its own copy of the same release, and it is that
	# download which once arrived corrupt; give it the copy already smoke-tested.
	seed_ido "$ido53" "$dir/lib/ultralib/tools/ido"

	# Eight hand-written .s files are assembled with a real mips-linux-gnu-gcc,
	# which does not exist for macOS.  Only the -x assembler-with-cpp path is
	# ever used, so preprocessing with clang and assembling with the real GNU
	# as covers it -- and the real as is required, because clang's integrated
	# MIPS assembler rejects the `.set gp=64` those files use.
	cat >"$cache/bin/mips-linux-gnu-gcc" <<'EOF'
#!/bin/bash
# Written by tools/setup_reference_builds.sh.  Implements only
# `gcc -c -x assembler-with-cpp ... -o out.o in.s`, which is all this needs.
args=("$@"); out=; input=; defs=(); incs=()
i=0
while [ $i -lt ${#args[@]} ]; do
	a="${args[$i]}"
	case "$a" in
	-o) i=$((i + 1)); out="${args[$i]}" ;;
	-I) i=$((i + 1)); incs+=("-I" "${args[$i]}") ;;
	-I*) incs+=("$a") ;;
	-D*) defs+=("$a") ;;
	*.s) input="$a" ;;
	esac
	i=$((i + 1))
done
if [ -z "$input" ] || [ -z "$out" ]; then
	echo "mips-linux-gnu-gcc wrapper: unsupported invocation: ${args[*]}" >&2
	exit 1
fi
set -o pipefail
clang -E -x assembler-with-cpp -P "${defs[@]}" "${incs[@]}" "$input" |
	mips-linux-gnu-as -EB -mabi=32 -march=vr4300 -mtune=vr4300 "${incs[@]}" -o "$out"
EOF
	chmod +x "$cache/bin/mips-linux-gnu-gcc"

	[ -d "$dir/.venv" ] || run "$PY" -m venv "$dir/.venv" || die "banjo-kazooie: venv failed"
	run "$dir/.venv/bin/python3" -m pip install -q -r "$dir/requirements.txt" ||
		die "banjo-kazooie: pip install failed"
	PATH=$dir/.venv/bin:$PATH
	export PATH

	run "$MAKE" -C "$dir/lib/ultralib" VERSION=I TARGET=libultra_rom COMPARE=0 MODERN_LD=1 setup ||
		die "banjo-kazooie: ultralib setup failed"
	# The first pass is deliberately serial.  splat's extraction is not a
	# prerequisite of the per-file compile rules, so parallel workers reach for
	# generated .c files no worker has written yet and make stops with "No rule
	# to make target".  Once extracted, -j is safe.
	(cd "$dir" && run "$MAKE" --jobs=1) || die "banjo-kazooie: serial build failed"
	(cd "$dir" && run "$MAKE" "--jobs=$jobs") || die "banjo-kazooie: build failed"
}

build_conker() {
	local dir=$root/conker ido53
	checkout conker
	stage_rom conker "$roms/Conker's Bad Fur Day (USA).zip" baserom.us.z64

	step "conker: patching"
	subst "$dir/Makefile" 'CPP     = cpp' 'CPP     = clang -E -P -x c'
	subst "$dir/conker/Makefile" 'CPP     = cpp' 'CPP     = clang -E -P -x c'
	append_once "$dir/conker/include/macro.inc" '.macro nonmatching' "$MACRO_STUBS"
	append_once "$dir/tools/asm-processor/prelude.inc" '.macro nonmatching' "$MACRO_STUBS"
	# asm-processor's own line scanner has to learn the same three directives.
	# `nonmatching` precedes the first glabel in every generated file, and a
	# line it does not recognise there is treated as an instruction, so it
	# aborts with ".text block without an initial glabel".
	subst "$dir/tools/asm-processor/asm_processor.py" \
		"line.startswith('endlabel ') or (' ' not in line" \
		"line.startswith('endlabel ') or line.startswith('enddlabel ') or line.startswith('endjlabel ') or (' ' not in line"
	subst "$dir/tools/asm-processor/asm_processor.py" \
		"        elif line.startswith('.section') or line in" \
		"        elif line.startswith('nonmatching '):
            pass # splat metadata (name, size), not an instruction
        elif line.startswith('.section') or line in"

	ido53=$(ido_dir 5.3 "$IDO_53_SHA256") || exit 2
	seed_ido "$ido53" "$dir/ido/ido5.3_recomp"

	[ -d "$dir/.venv" ] || run "$PY" -m venv "$dir/.venv" || die "conker: venv failed"
	run "$dir/.venv/bin/python3" -m pip install -q \
		-r "$dir/requirements.txt" -r "$dir/tools/n64splat/requirements.txt" ||
		die "conker: pip install failed"
	PATH=$dir/.venv/bin:$PATH
	export PATH

	(cd "$dir" && run "$MAKE" check) || die "conker: baserom check failed"
	(cd "$dir" && run "$MAKE" extract) || die "conker: top-level extract failed"
	# The README calls this one optional.  It is not: without it there are no
	# asm/nonmatchings/**/*.s files and nothing referencing GLOBAL_ASM builds.
	(cd "$dir/conker" && run "$MAKE" extract) || die "conker: per-function extract failed"
	# Recorded outcome is `partial`.  Every source file compiles; the final
	# link does not, on _asmpp_funcN collisions across a few debugger
	# translation units and on references into the ~77% of this game that is
	# not decompiled yet.  Mining compares whole .text sections and never
	# links, so the objects are the deliverable and they are complete -- which
	# leaves the digest check below as the only thing standing between a real
	# compile failure and this comment.  It is not optional for this title.
	(cd "$dir/conker" && run "$MAKE" "--jobs=$jobs") ||
		step "conker: link failed, as recorded; checking the objects"
}

# ---------------------------------------------------------------------------
# Drive
# ---------------------------------------------------------------------------

for title in $titles; do
	echo >&2
	step "$title ($(field "$title" name)) -- expected outcome: $(field "$title" outcome)"
	case "$title" in
	jfg) build_jfg ;;
	perfect_dark) build_perfect_dark ;;
	banjo-kazooie) build_banjo_kazooie ;;
	conker) build_conker ;;
	*) die "no build recipe for '$title'" ;;
	esac
done

echo >&2
step "checking the farm against $lock"
# shellcheck disable=SC2086
exec bash "$repo/tools/verify_reference_builds.sh" --root "$root" $titles
