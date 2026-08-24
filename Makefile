# Mickey's Speedway USA (US) — clean-room decompilation build
#
# Phase 0: the ROM is rebuilt entirely from splat's disassembly + extracted
# binaries. No C is compiled yet; the IDO variables below are kept wired up so
# that later phases only have to add source files, not re-derive the toolchain.
#
#   gmake            build/mickey.us.z64
#   gmake verify     build + SHA1 compare against the baserom hash
#   gmake setup      venv + toolchain + baserom check + splat extraction + git hooks
#   gmake hooks      point git at .githooks (clean-room commit/push gates)
#   gmake cleanroom  clean-room sweep (CLEANROOM_ARGS=--staged, --range A..B)
#   gmake audit-decoders  assert no clean-room decoder is inventing words
#   gmake overlay-tables  decode the four overlay ROM blocks and check the layout
#   gmake overlay-atlas   check the generated overlay manifest and yaml block
#   gmake overlay-atlas-write  refresh those two tracked generated artifacts
#   gmake overlay-donors  validate the exhaustive DKR/JFG donor ledger
#   gmake overlay-donors-write  rescan the out-of-tree donor builds
#   gmake prune-asm  delete asm/ files splat orphaned (also run by every split)
#   gmake reference-builds        rebuild the out-of-tree reference decomp farm
#   gmake check-reference-builds  prove that farm is the one the names came from
#   gmake scoreboard        regenerate README.md's progress block from the tree
#   gmake check-scoreboard  fail if that block has gone stale
#   gmake clean      remove build/
#   gmake distclean  also remove splat's generated output

BASENAME := mickey
VERSION  := us

# ---------------------------------------------------------------------------
# Directories
# ---------------------------------------------------------------------------

# Compile-only escape hatch for the NON_MATCHING/GLOBAL_ASM functions (see
# docs/acceleration-survey.md sec.13.2): `gmake NON_MATCHING=1` takes every
# converted TU's real C body instead of its GLOBAL_ASM fallback, so the
# functions queued for source restructuring keep compiling under IDO without
# being claimed as matched. It never produces a byte-identical ROM -- `verify`
# refuses to run under it, exactly DKR's guard.
NON_MATCHING ?= 0

# Separate build tree for NON_MATCHING=1: objects compiled with -DNON_MATCHING
# are never byte-identical, so they must never sit next to (or be mistaken
# for, via stale timestamps) the objects `verify` checks.
ifeq ($(NON_MATCHING),0)
BUILD_DIR := build
else
BUILD_DIR := build_non_matching
endif
SRC_DIR   := src
# Every directory under src/ that holds .c files. Discovered rather than listed
# so adding a new source subdirectory (src/libultra, src/main, ...) needs no
# Makefile edit -- splat decides the layout via the yaml's subsegment names.
SRC_DIRS  := $(shell find $(SRC_DIR) -type d 2>/dev/null)
# Same idea as SRC_DIRS: naming a subsegment `libultra/foo` makes splat write
# asm/libultra/foo.s, and a hardcoded list would silently drop it from the link
# (no error -- just a wrong ROM). `asm` stays literal so `distclean` still has
# something to remove on a tree that was never split.
#
# asm/nonmatchings is deliberately excluded: those files are assembled *into* C
# objects by asm-processor via `#pragma GLOBAL_ASM`, so assembling them again
# here would define every one of their symbols twice.
ASM_DIRS  := asm $(shell find asm -mindepth 1 -type d -not -path 'asm/nonmatchings*' 2>/dev/null)
# Overlay data/relocation tails are deliberately nested by module. Discover
# binary directories just like asm/source directories so adding a generated
# module never requires a second, hand-maintained object list.
BIN_DIRS  := assets $(shell find assets -mindepth 1 -type d 2>/dev/null)
TOOLS_DIR := tools

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------

CROSS   := $(TOOLS_DIR)/binutils/mips64-elf-
AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
STRIP   := $(CROSS)strip

HOST_CC := cc
HOST_PYTHON := python3
VENV    := .venv
PYTHON  := $(VENV)/bin/python
SHA1    := shasum -a 1

# IDO 5.3. These are the project-default C flags; per-file overrides live in the
# "Per-file compiler flags" block further down and must be justified in the
# file's header comment (see docs/CLEANROOM.md's sibling rule in the plan).
CC      := $(TOOLS_DIR)/ido/cc
OPT_FLAGS := -O2
MIPSISET  := -mips1 -32
DEFINES   := -D_LANGUAGE_C -D_FINALROM -DTARGET_N64 -DVERSION_$(VERSION) \
             -D_MIPS_SZLONG=32
ifneq ($(NON_MATCHING),0)
DEFINES += -DNON_MATCHING
endif
INCLUDE_CFLAGS := -I . -I include -I include/libc -I include/PR -I assets
CFLAGS  := -non_shared -G 0 -Xcpluscomm -fullwarn -woff 649,838 -nostdinc \
           $(DEFINES) $(INCLUDE_CFLAGS)
POSTPROCESS := @:

# asm-processor (simonlindholm) is what makes `#pragma GLOBAL_ASM("...")` work
# with IDO: it strips the pragmas out, compiles the remaining real C, assembles
# the referenced .s files with $(AS), and splices the result back into the
# object so hand-written asm and compiled C share one translation unit in the
# right order. Invocation shape is
#   build.py <compiler...> -- <assembler...> -- <compile args...> <input.c>
# i.e. the compiler and assembler command lines are passed through verbatim.
ASM_PROCESSOR := $(PYTHON) $(TOOLS_DIR)/asm-processor/build.py

CRC := $(TOOLS_DIR)/n64crc

# ---------------------------------------------------------------------------
# Flags
# ---------------------------------------------------------------------------

# Verified working assembler invocation (Task 4).
ASFLAGS := -march=vr4300 -32 -mabi=32 -G0 -I include

# asm-processor's GLOBAL_ASM path needs a couple of macros that its own prelude
# doesn't define; gas takes several input files, so the extra prelude is simply
# handed to it ahead of asm-processor's temporary .s. See the file's comment.
ASM_PROC_ASFLAGS := $(ASFLAGS) include/asm_processor_prelude.inc

# splat's ld script names every input object explicitly, so nothing is passed
# on the command line; --no-check-sections because segments deliberately share
# VRAM ranges (overlays / assets all follow main's vram).
LD_SCRIPT := $(BASENAME).$(VERSION).ld
LDFLAGS   := -T $(LD_SCRIPT) \
             -T undefined_funcs_auto.$(VERSION).txt \
             -T undefined_syms_auto.$(VERSION).txt \
             -T libultra_undefined_syms.$(VERSION).txt \
             -T overlay_undefined_syms.$(VERSION).txt \
             -Map $(BUILD_DIR)/$(BASENAME).$(VERSION).map --no-check-sections

# rom_fill already carries the trailing 0xFF region, so --pad-to is a belt-and
# -braces guarantee of the 32MiB image size rather than a real fill step.
ROM_SIZE     := 0x2000000
OBJCOPYFLAGS := -O binary --pad-to=$(ROM_SIZE) --gap-fill=0xFF

# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

S_FILES   := $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))
BIN_FILES := $(foreach dir,$(BIN_DIRS),$(wildcard $(dir)/*.bin))
C_FILES   := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

# Every header, as a blunt prerequisite for every object: there are only a
# handful of them and IDO's dependency output is awkward to wire in, so
# "recompile all C when any header changes" is the cheap correct answer.
H_FILES   := $(shell find include -name '*.h' 2>/dev/null)

O_FILES := $(foreach f,$(S_FILES),$(BUILD_DIR)/$(f).o) \
           $(foreach f,$(C_FILES),$(BUILD_DIR)/$(f).o) \
           $(foreach f,$(BIN_FILES),$(BUILD_DIR)/$(f).o)

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(ASM_DIRS) $(BIN_DIRS) $(SRC_DIRS))

TARGET   := $(BUILD_DIR)/$(BASENAME).$(VERSION)
BASEROM  := baseroms/$(BASENAME).$(VERSION).z64
EXPECTED_SHA1 := $(firstword $(shell cat $(BASENAME).$(VERSION).sha1))

# splat.yaml is the single source of truth for what gets extracted/split from
# the baserom (asm/, assets/, the generated .ld script...). This stamp makes
# that split step a real Make dependency: anything that reads split output
# (the .s/.bin -> .o pattern rules, the final link) order-depends on it, so
# an edited yaml re-splits before those steps run instead of silently
# building against stale disassembly. See the .o pattern rules and
# $(TARGET).elf below for how the ordering is wired up.
SPLAT_STAMP := $(BUILD_DIR)/.splat-stamp

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

default: all

# Two-phase build, driven by a fresh recursive $(MAKE) per phase.
#
# Why: a single `make` invocation decides whether build/asm/FOO.s.o needs
# rebuilding by stat()-ing its prerequisites once, in the order they're
# listed on the rule -- `%.s` (normal) comes before `$(SPLAT_STAMP)`
# (order-only), so that stat happens *before* the split recipe (which is
# what actually rewrites %.s) has run, even though make correctly runs the
# order-only prereq's recipe first. Net effect, verified empirically: within
# one invocation, order-only alone does NOT retrigger .o rebuilds after a
# same-invocation re-split -- `gmake` would silently link against
# pre-re-split objects. (Minimal repro kept in the Task 2 fix notes.)
#
# The reliable fix is the classic two-pass recursive-make idiom: run the
# split to completion as its own `make` invocation (phase 1), then start a
# genuinely *new* `make` invocation for the real build (phase 2) -- its
# dependency scan stats files from disk fresh, after phase 1's writes, so
# object files correctly see their .s as newer and rebuild. The .o/.elf
# pattern rules below still order-depend on $(SPLAT_STAMP) too, as a
# best-effort guard for anyone invoking a build target directly instead of
# via `all`/`verify`.
all:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(TARGET).z64

setup: $(PYTHON) hooks
	$(PYTHON) -m pip install -q -r requirements.txt
	git submodule update --init $(TOOLS_DIR)/asm-processor \
		$(TOOLS_DIR)/asm-differ $(TOOLS_DIR)/m2c
	$(TOOLS_DIR)/setup_toolchain.sh
	$(TOOLS_DIR)/verify_baseroms.sh
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@$(MAKE) --no-print-directory prune-asm
	@mkdir -p $(BUILD_DIR)
	@touch $(SPLAT_STAMP)

# Unconditional re-split, e.g. after hand-editing yaml and wanting the result
# immediately without going through the dependency graph. Keeps the stamp in
# sync so a following `gmake` doesn't redundantly split again.
extract:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@$(MAKE) --no-print-directory prune-asm
	@mkdir -p $(BUILD_DIR)
	@touch $(SPLAT_STAMP)

# splat writes asm/ and never prunes it, so converting a subsegment from `asm`
# to `c`, or naming a function, leaves a file behind that nothing produces any
# more. Neither breaks the build -- the linker script simply stops naming the
# object -- but tools/progress.py counts a function as unmatched while any
# glabel for it survives anywhere under asm/, so the matched count silently
# under-reports until the file is deleted. It ran after every split rather than
# being remembered.
prune-asm:
	@$(PYTHON) $(TOOLS_DIR)/prune_stale_asm.py $(BASENAME).$(VERSION).yaml

verify:
ifneq ($(NON_MATCHING),0)
	$(error verify does not run under NON_MATCHING=1 -- it never produces a byte-identical ROM; unset NON_MATCHING and rebuild)
endif
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(TARGET).z64
	@got=$$($(SHA1) $(TARGET).z64 | cut -d' ' -f1); \
	echo "expected $(EXPECTED_SHA1)"; \
	echo "built    $$got"; \
	if [ "$$got" = "$(EXPECTED_SHA1)" ]; then \
		echo "OK  $(TARGET).z64 matches the expected US ROM hash"; \
	else \
		echo "FAIL $(TARGET).z64 does not match the expected US ROM hash"; \
		exit 1; \
	fi

# Points git at the tracked hook directory. Separate from `setup` so it can be
# re-run on its own, and so a clone that only wants the gates does not have to
# build a toolchain to get them. core.hooksPath is per-clone config, not a
# tracked file, so every fresh clone needs this once.
hooks:
	git config core.hooksPath .githooks
	@echo "hooks active: .githooks/pre-commit, .githooks/pre-push"

# Clean-room sweep. Extra arguments pass through, so the same target serves
# every mode:
#   gmake cleanroom                                  worktree
#   gmake cleanroom CLEANROOM_ARGS=--staged          the index
#   gmake cleanroom CLEANROOM_ARGS="--range A..B"    a commit range
cleanroom:
	bash $(TOOLS_DIR)/cleanroom_check.sh $(CLEANROOM_ARGS)

# Asserts that no clean-room decoder is inventing words -- that every stage
# which exists to DECODE something contributes nothing to a tree whose content
# is not encoded. Five false-decode defects were found by hand before this was
# automated, two of them only by re-running the audit after a "fix" had
# re-routed the phantom to a different decoder. Read the header of
# tools/audit_decoders.py before changing a threshold it complains about.
#
#   gmake audit-decoders                       tracked files (~0.2s)
#   gmake audit-decoders AUDIT_ARGS=--all      every blob in history (~4s)
#   gmake audit-decoders AUDIT_ARGS=--verbose  print the per-stage totals
#
# Deliberately NOT folded into `cleanroom`. The hooks run `cleanroom` on every
# commit and it must stay fast and must fail only for clean-room reasons; this
# check answers a different question -- "is the detector still measuring
# reality?" -- and it is aimed at whoever edits a decoder, not at whoever
# writes a commit. Folding it in would make an unrelated red bar block commits,
# which is precisely how a gate gets bypassed with --no-verify.
audit-decoders:
	$(PYTHON) $(TOOLS_DIR)/audit_decoders.py $(AUDIT_ARGS)

# Decodes the four overlay blocks the yaml splits (main relocation table, ROM
# table, header table, module images) out of the baserom and re-asserts the
# layout: the count word, 370 of 375 reloc entries landing on a real
# `jal TrapDanglingJump`, the per-module byte-gap arithmetic across all 107
# headers, the final module's end at 0x18F1FE0, and clone.c's strings falling
# inside overlay 43's .data. Reads the ROM at run time and writes nothing, so
# like `verify` it needs a baserom and cannot run in CI.
#
#   gmake overlay-tables                       human-readable module map
#   gmake overlay-tables OVERLAY_ARGS=--json   machine-readable, same fields
overlay-tables:
	$(PYTHON) $(TOOLS_DIR)/overlay_tables.py $(OVERLAY_ARGS)

# The atlas is the canonical, reviewable projection of all 107 module headers,
# 18,542 relocation records, exports, resident references, and cross-overlay
# edges. `overlay-atlas` is read-only and fails on drift. Updating the tracked
# JSON and generated yaml block is explicit so an ordinary build never edits
# source files as a side effect.
overlay-atlas:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check

overlay-atlas-write:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --write

# Every overlay decomp pass starts with the DKR v77/v80 and JFG object scans in
# this ledger. The ordinary target validates the committed 107-row-per-donor
# report without needing the out-of-tree builds. Refresh and reproducibility
# checks are explicit because they do need the pinned reference farm.
overlay-donors:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --check

overlay-donors-write:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --write

overlay-donors-scan-check:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --scan-check

# The other direction, and the one `audit-decoders` is structurally blind to: a
# decoder that quietly STOPS producing words looks exactly like a decoder
# behaving, so every number in the audit stays green while the gate becomes a
# no-op. This synthesizes real-ROM fixtures in every encoding at every wrap
# width from baseroms/mickey.us.z64 AT RUN TIME, asserts each is still caught,
# and writes nothing to disk -- a committed fixture that proves the detectors
# catch ROM data would itself be ROM data, and `cleanroom` would rightly reject
# it. Needs the baserom, so like `verify` it cannot run in CI; run it after
# touching tools/cleanroom_detectors.py, alongside `audit-decoders`, not
# instead of it.
check-fixtures:
	$(PYTHON) $(TOOLS_DIR)/false_negative_fixtures.py $(FIXTURE_ARGS)

# The reference decompilations 190 of this tree's tier-A names were mined from.
# `reference-builds` rebuilds that farm from the commits and baserom checksums
# tools/reference-builds.lock pins; `check-reference-builds` proves a farm on
# this machine is the one those names came from, by re-deriving each title's
# aggregate digest and comparing it. Neither can run in CI: the farm needs
# retail ROMs, which this project does not ship and never will, so this is the
# same honest split as `verify` and `check-fixtures`.
#
#   gmake reference-builds REFS_ARGS=jfg     one title
#   gmake reference-builds REFS_ARGS="--root DIR --jobs 2"
#
# Reference material stays out of the tree (docs/CLEANROOM.md); what is
# committed is the recipe, the pins and the digests. docs/references.md says
# what each build yielded and what remains uncheckable.
reference-builds:
	bash $(TOOLS_DIR)/setup_reference_builds.sh $(REFS_ARGS)

check-reference-builds:
	bash $(TOOLS_DIR)/verify_reference_builds.sh $(REFS_ARGS)

# Re-derives the arithmetic the docs claim -- VRAM/ROM conversions, segment
# size subtractions, the MiB column of the top-level map, the jump-table count
# -- and fails on a mismatch. A count audit found several of these stale at
# once; they are all recomputable, so they are recomputed here rather than at
# the next review. The jump-table count needs asm/, so it is skipped (not
# failed) before `gmake extract`.
check-docs:
	$(PYTHON) $(TOOLS_DIR)/check_derived_numbers.py
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --check

# Builds just far enough to have a linked ELF (no crc/z64 round-trip needed --
# tools/progress.py only reads the ELF's symbol table plus the current asm/
# and symbol_addrs.$(VERSION).txt state), then reports the derived progress
# numbers. Same two-phase split-then-build shape as `all`/`verify`, for the
# same reason (see the big comment on `all` above).
progress:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(TARGET).elf
	$(PYTHON) $(TOOLS_DIR)/progress.py --version $(VERSION)

# Rewrites README.md's scoreboard block, between its SCOREBOARD_BEGIN /
# SCOREBOARD_END markers, from the same derivation `progress` prints. The
# README is the most-read page in the repo and therefore the worst place for a
# hand-maintained count: this project has already caught summary numbers that
# had drifted from the tree they described, and the block this replaces was
# itself stale by two functions and a hundred and ninety symbols. So it is
# generated, and `check-scoreboard` below proves it stayed generated.
scoreboard:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(TARGET).elf
	$(PYTHON) $(TOOLS_DIR)/progress.py --version $(VERSION) --update-readme

# Fails if README.md's scoreboard block is not what the tree generates right
# now, printing the diff. Deliberately a separate target from `check-docs`:
# that one re-derives arithmetic *stated in prose* and needs nothing built,
# while this one needs a linked ELF, so folding them together would make
# `check-docs` require a toolchain and a build to answer a question that has
# nothing to do with one.
check-scoreboard:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(TARGET).elf
	$(PYTHON) $(TOOLS_DIR)/progress.py --version $(VERSION) --check-readme

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(ASM_DIRS) $(BIN_DIRS)
	rm -f $(LD_SCRIPT) undefined_funcs_auto.$(VERSION).txt undefined_syms_auto.$(VERSION).txt
	rm -f $(CRC)

# ---------------------------------------------------------------------------
# Recipes
# ---------------------------------------------------------------------------

$(ALL_DIRS):
	@mkdir -p $@

# Bootstrap the venv on a fresh clone. Only runs when .venv/bin/python is
# missing, so an existing environment is never rebuilt from under you.
$(PYTHON):
	$(HOST_PYTHON) -m venv $(VENV)

$(CRC): $(TOOLS_DIR)/n64crc.c
	$(HOST_CC) -O2 -w -o $@ $<

# $(SPLAT_STAMP) is an order-only prereq here: it guarantees the split has
# run at least once before this rule's recipe executes. It is *not* enough
# on its own to retrigger a rebuild after a same-invocation re-split (see
# the big comment on the `all` target above for why, and why the real
# correctness mechanism is the two-phase recursive `make` in `all`/`verify`)
# -- but it's a harmless, cheap safety net for anyone building a specific
# .o/.elf target directly instead of going through `all`/`verify`.
$(BUILD_DIR)/%.s.o: %.s | $(ALL_DIRS) $(SPLAT_STAMP)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.bin.o: %.bin | $(ALL_DIRS) $(SPLAT_STAMP)
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips $< $@

# The C rule. Everything goes through asm-processor unconditionally -- a file
# with no GLOBAL_ASM pragmas is passed through to IDO untouched, so there is no
# reason to maintain two recipes.
#
# $(SPLAT_STAMP) is a *normal* (not order-only) prerequisite here, unlike the
# .s.o/.bin.o rules below: a #pragma GLOBAL_ASM in a .c file names a .s under
# asm/nonmatchings/ that splat regenerates on every re-split, and that .s is
# not itself listed as a prerequisite of this object (there is no per-TU
# dependency file naming exactly which GLOBAL_ASM paths one .c references).
# Without a real dependency on the stamp, a re-split that renames a symbol
# (splat re-numbers auto names, or a friendly name changes) leaves an
# already-built .c.o holding a stale reference and an incremental build can
# link a mismatched object, or fail outright, while a clean build passes --
# this happened for real (overlay1GetEntry's pilot conversion sits right next
# to this comment; runlink.c.o hit exactly this when another lane's re-split
# renamed func_8002B768). Depending on the stamp for real means *every*
# .c.o rebuilds after any re-split, not just the ones that reference a moved
# name -- coarser than necessary, but a full rebuild is ~17s and correctness
# beats precision here.
$(BUILD_DIR)/%.c.o: %.c $(H_FILES) $(TOOLS_DIR)/normalize_elf_instructions.py $(SPLAT_STAMP) | $(ALL_DIRS)
	$(ASM_PROCESSOR) $(CC) -- $(AS) $(ASM_PROC_ASFLAGS) -- \
		-c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
	$(POSTPROCESS)

# ---------------------------------------------------------------------------
# Per-file compiler flags
#
# Deviations from the project defaults above. Each one must be forced by
# evidence (the file does not match otherwise) and explained in the source
# file's header comment, never guessed at.
# ---------------------------------------------------------------------------

# libultra's libc string TU needs branch-likely instructions (bnel/beql), which
# IDO only emits at -mips2; -mips1 produces a 0x90-byte .text instead of the
# ROM's 0xA0. Consistent with how the DKR decomp builds its libultra tree.
$(BUILD_DIR)/$(SRC_DIR)/libultra/string.c.o: MIPSISET := -mips2 -32

# These libultra TUs are built at -O1 -mips2, not the project default -O2 -mips1.
# Measured, not assumed: at -O2 IDO folds their stack frames away entirely and
# the .text comes out the wrong size, while -O1 -mips2 reproduces every one of
# them byte for byte.
#
# The list is NOT "libultra's io/ directory", despite most of it coming from
# there: getactivequeue is os/getactivequeue.c and needs the same flags. DKR
# happens to build its io/ tree at -O1 and its os/ tree at -O1 as well, so the
# per-directory framing would have been a coincidence rather than a rule -- the
# grouping here is simply "TUs measured to need -O1 -mips2", which is the only
# thing the evidence actually supports. Add a file when you have measured it.
#
# splat writes every libultra subsegment into one flat src/libultra/, so this
# cannot be a directory-scoped pattern rule and each file is named. Keep the
# list sorted.
LIBULTRA_O1_TUS := ai aigetlen aisetfreq aisetnextbuf crc createmesgqueue destroythread \
                   dp dpsetnextbuf dpsetstat getactivequeue getthreadpri \
                   gettime jammesg recvmesg resetglobalintmask sendmesg \
                   seteventmesg setthreadpri settimer settime si siacs \
                   sirawdma sirawread sirawwrite sp sprawdma spgetstat \
                   spsetstat sptask sptaskyield \
                   sptaskyielded startthread stopthread thread timerintr vi viblack \
                   vigetcurrcontext vigetcurrframebuf vigetnextframebuf \
                   visetevent visetmode visetspecial viswapbuf \
                   viswapcontext virtualtophysical yieldthread
$(foreach f,$(LIBULTRA_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O1))
$(foreach f,$(LIBULTRA_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))

# This old SDK source uses `__GNUC__` as a version-path selector even when IDO
# compiles it. JFG's matching object defines it for this TU only.
$(BUILD_DIR)/$(SRC_DIR)/libultra/destroythread.c.o: CFLAGS += -D__GNUC__

# -g3 -mips2: a THIRD libultra flag group, found in Phase 2 Task 3.
#
# WHAT IS MEASURED AND WHAT IS NOT -- read the two apart, because they are not
# the same strength of claim.
#
#   MEASURED: `-g3` and `-mips2` are both forced by Mickey's bytes. All twelve
#   combinations of {-O0,-O1,-O2} x {with,without -g3} x {-mips1,-mips2} were
#   compiled and compared against ROM 0x730A0 and 0x730F0. Exactly two produce
#   the ROM's 0x48-byte function: `-O1 -g3 -mips2` and `-O2 -g3 -mips2`. Drop
#   -g3 and the function is 0x44; drop -mips2 and it is 0x50 or 0x4C; -O0 gives
#   0x60. (-O3 could not be tested: this IDO recomp build dies in `uld` on any
#   -O3 invocation, an environment failure rather than a fact about the code.)
#
#   ALSO MEASURED, on a later TU: `-O2` rather than `-O1`. epiread and epiwrite
#   are byte-identical at both, so those two files never discriminated the
#   optimisation level and -O2 was borrowed from Jet Force Gemini's published
#   Makefile. `pidma` (ROM 0x6FB90) settles it from Mickey's own bytes: 48
#   instructions at -O2 -g3 -mips2, matching word for word, against 68 at
#   -O1 -g3 -mips2 and 78 at -O0.
#
# What -g3 changes, structurally, confirmed in disassembly: without it IDO
# hoists the third argument's spill (`sw a2,0x28(sp)`) into the first jal's
# delay slot and leaves that slot empty in the ROM's version, and it emits
# `lw ra` before `lw v0` in the epilogue. The ROM does neither.
#
# Reading a permitted public decompilation's build configuration is the same
# permission as reading its source (docs/CLEANROOM.md). The part of this that
# is a fact about MICKEY is the part that was measured here.
#
# Every TU in the list below was measured before it was added, one at a time.
# The list is not "libultra's io/ and pfs/ trees"; it is the set that has been
# measured, and nothing joins it on a neighbour's evidence.
LIBULTRA_O2_G3_TUS := contpfs epidma epilinkhandle epirawdma epirawread \
                      epirawwrite epiread epiwrite pfsallocatefile pfschecker \
                      pfsfilestate pfsfreeblocks pfsgetstatus pfsinit \
                      pfsisplug pfsnumfiles pfsreadwritefile pfssearchfile \
                      pfsselectbank piacs pidma pigetcmdq pirawdma
$(foreach f,$(LIBULTRA_O2_G3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O2 -g3))
$(foreach f,$(LIBULTRA_O2_G3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))

# -Xphase,uopt,+ -Xphase,uopt,-O1: a FOURTH libultra flag group, and the only
# one that does not go through the `cc` driver.
#
# WHY A WRAPPER AT ALL. `tools/ido/cc` runs `uopt` only at -O2 and above --
# checked at -O1 x {-g0,-g1,-g2,-g3,none}, where `cc -v` never lists a uopt
# stage. So the configuration "uopt ran, at -O1" cannot be produced by the
# driver. It cannot be faked with -Wo,-O1 either: -W<pass>,-O<n> is inserted
# BEFORE the driver's own -O and the last -O wins. (ISA options are appended
# after, which is why -Wc,-mips3 below does work and -Wo,-O1 does not.)
#
# ido-phases.py drives cfe/uopt/ugen/as1 itself, taking each phase's command
# line from `cc -v` rather than reimplementing the driver's flag translation,
# so it cannot drift from it. With no -Xphase, option it is byte-identical to
# `cc`: verified on string.c (-O2 -mips2), epidma.c (-O2 -g3 -mips2), si.c
# (-O1 -mips2, where the driver skips uopt), pfsreadwritefile.c (-O2 -g3
# -mips2), and on a GLOBAL_ASM TU under asm-processor.
#
# MEASURED, on setglobalintmask: __osSetGlobalIntMask is byte-identical to ROM
# 0x75080 once uopt has run at -O1 -- 19 of 19 words, with only the six the
# linker fills in differing. Without the uopt stage the same source gives 20
# instructions with the first jal's delay slot empty, which is what the file's
# header used to describe. -g3 is NOT needed (identical with and without);
# -mips2 IS (-mips1 gives 9 of 19); -O2 destroys it (the frame drops to 0x40).
#
# HOST_PYTHON, not $(PYTHON): the wrapper is stdlib-only, and using the venv
# would make every C object depend on `gmake setup`.
IDO_PHASES := $(HOST_PYTHON) $(TOOLS_DIR)/ido-phases.py

LIBULTRA_UOPT_O1_TUS := setglobalintmask
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CC := $(IDO_PHASES)))
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O1))
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CFLAGS += -Xphase,uopt,+ -Xphase,uopt,-O1))

# libultra's compiler-runtime 64-bit helpers use MIPS III instructions and
# were built at -O1. JFG's whole `libc/ll.c` object fixes both choices against
# Mickey's bytes; this is not a directory-wide libc assumption.
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: $(TOOLS_DIR)/set_elf_flags.py
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: MIPSISET := -mips3 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: POSTPROCESS = $(HOST_PYTHON) \
	$(TOOLS_DIR)/set_elf_flags.py $@ 0x10000000

# The old audio-library object was built unoptimised with debug codegen.  The
# ROM's 0x50-byte body is byte-identical to this exact flag pair.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_cspsetvol.c.o: OPT_FLAGS := -g
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_cspsetvol.c.o: MIPSISET := -mips2 -32

# Remaining SDK C groups measured against the pinned JFG objects.
$(BUILD_DIR)/$(SRC_DIR)/libultra/cents2ratio.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/cents2ratio.c.o: OPT_FLAGS := -g
$(BUILD_DIR)/$(SRC_DIR)/libultra/sinf.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/sinf.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/libultra/vimgr.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/vimgr.c.o: OPT_FLAGS := -O2
$(BUILD_DIR)/$(SRC_DIR)/libultra/aisetnextbuf.c.o: CFLAGS += -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/sptask.c.o: CFLAGS += -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/vi.c.o: CFLAGS += -DRAREDIFFS

LIBULTRA_O3_TUS := xlitob
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CC := $(IDO_PHASES)))
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O2))
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CFLAGS += -Xphase,cfe,-O3 \
	-Xphase,uopt,-O3 -Xphase,ugen,-O3 -Xphase,as1,-O3))

# IDO's driver cannot complete an -O3 build in this environment, but the phase
# wrapper can drive the same cfe/uopt/ugen/as1 pipeline directly.  The base -O2
# invocation supplies all four stages, then each phase is promoted to the -O3
# pass that produced libultra's ldiv object (including its function ordering).
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: CC := $(IDO_PHASES)
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: OPT_FLAGS := -O2
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: CFLAGS += -Xphase,cfe,-O3 \
	-Xphase,uopt,-O3 -Xphase,ugen,-O3 -Xphase,as1,-O3

# GAME code (as opposed to libultra) is -mips2, not the project default -mips1.
# Measured on the first game TU decompiled, main/runlink.c: the ROM's code does
# `lw t7,0(a3)` immediately followed by `addu v1,v1,t7`, i.e. it uses the loaded
# register in the very next instruction. That is only legal without a nop from
# -mips2 onwards; at -mips1 IDO's assembler inserts a load-delay nop after every
# such pair, and main/runlink.c came out 0x14 bytes long over five functions
# before this line existed.
#
# Scoped to src/main/ rather than made the global default so the libultra
# findings above stay untouched and so the claim stays exactly as wide as the
# evidence. Widen it when the next game module is measured, not before.
$(BUILD_DIR)/$(SRC_DIR)/main/%.c.o: MIPSISET := -mips2 -32

# Overlay game code is likewise MIPS II. Every adopted tranche-A object was
# compared instruction-for-instruction at this ISA level before joining this
# rule; MIPS I inserts load-delay nops in several of them.
$(BUILD_DIR)/$(SRC_DIR)/overlays/%.c.o: MIPSISET := -mips2 -32

# Rare's audio-bank patcher is an -O3 object in DKR and Mickey. Keeping each
# helper in its measured source boundary prevents the interprocedural inliner
# from folding calls that remain present in Mickey's bytes.
OVERLAY5_O3_TUS := alSeqFileNew alBnkfNew _bnkfPatchBank _bnkfPatchInst \
                   _bnkfPatchSound _bnkfPatchWaveTable
OVERLAY5_O3_OBJECTS := $(addprefix $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/, \
                       $(addsuffix .c.o,$(OVERLAY5_O3_TUS)))
$(OVERLAY5_O3_OBJECTS): $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/%.c.o: \
                        $(SRC_DIR)/overlays/o005/%.c $(H_FILES) | $(ALL_DIRS) $(SPLAT_STAMP)
	$(CC) -c $(CFLAGS) -O3 -mips2 -32 -o $@ $<
	$(POSTPROCESS)

# IDO aligns standalone .text sections to 16 bytes, while these reviewed
# overlay functions continue at four-byte boundaries inside a larger module.
# The trimmer only reduces the ELF section header and refuses nonzero tails.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o006/overlay_006.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o076/overlay_076.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114
$(BUILD_DIR)/$(SRC_DIR)/overlays/o078/overlay_078.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o102/overlay_102.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o103/overlay_103.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o106/overlay_106.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o107/osRamTest4_6105.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchBank.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchInst.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x98
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchSound.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchWaveTable.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay5CreatePlayer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
# This typed bootstrap naturally recovers every call, branch, memory access,
# and the exact 0x3a4-byte topology. Guardedly select the shipped register and
# schedule web while retaining all 42 runtime-relocated data halves; distinct
# call proxies preserve the runtime relocation-table zero payloads.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay5InitializeAudio.c.o: \
	config/normalizations/overlay5InitializeAudio.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay5InitializeAudio.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3A4 50ebbba59344c59f548cadd88bb390694e965d7dff44edbb07013cb87b84c9d9 \
		@config/normalizations/overlay5InitializeAudio.ops && \
	$(OBJCOPY) \
		--redefine-sym alHeapInit=overlay5HeapInitReloc \
		--redefine-sym func_8002E148=overlay5GetResourceReloc \
		--redefine-sym func_8002B280=overlay5AllocateResourceReloc \
		--redefine-sym func_8002E2E0=overlay5CopyResourceReloc \
		--redefine-sym func_8002E35C=overlay5ResolveResourceReloc \
		--redefine-sym alHeapDBAlloc=overlay5HeapAllocateReloc \
		--redefine-sym func_80001740=overlay5InitializeSoundReloc \
		--redefine-sym gsSndpNew=overlay5InitializeSequencePlayerReloc \
		--redefine-sym func_80001BA0=overlay5StartAudioReloc \
		--redefine-sym func_80000450=overlay5ConfigureAudioReloc \
		--redefine-sym func_8002B768=overlay5ReleaseResourceReloc \
		--redefine-sym func_800039F0=overlay5FinalizeAudioReloc \
		--redefine-sym alSurround_OutputType=overlay5SetOutputTypeReloc \
		--redefine-sym alSurround_ReverbSetup=overlay5SetupReverbReloc \
		--redefine-sym osCreateMesgQueue=overlay5CreateMessageQueueReloc \
		--redefine-sym n_alCSPSetMessageQ=overlay5SetPlayerMessageQueueReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1WrapOffset.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SignedOffset.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
# Two independent operations straddle the same source-line scheduling points
# in the shipped object. Assert IDO's natural order before restoring them.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindNextAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xc8 5b1579bb3bce00bb21b7fd7267281e471c33ad764b30ab09c110f57346cb1ae0 \
		fields:0x3c:op=4@0,rs=a0@zero,rd=zero@s3,fn=23@37 \
		fields:0x40:op=0@4,rs=zero@a0,rd=s3@zero,fn=37@22 \
		fields:0x6c:op=9@0,rs=s2@s1,rt=s2@zero,rd=ra@v0,sa=31@0,fn=60@37 \
		fields:0x70:op=0@9,rs=s1@s2,rt=zero@s2,rd=v0@ra,sa=0@31,fn=37@60 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindPreviousAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xc8 367cdc3f0f5de6c3e1dc773091046af57dddd67e8f1bb5d39188a6b7f81867a9 \
		fields:0x3c:op=4@0,rs=a0@zero,rd=zero@s3,fn=23@37 \
		fields:0x40:op=0@4,rs=zero@a0,rd=s3@zero,fn=37@22 \
		fields:0x6c:op=9@0,rs=s2@s1,rt=s2@zero,rd=ra@v0,sa=31@0,fn=60@37 \
		fields:0x70:op=0@9,rs=s1@s2,rt=zero@s2,rd=v0@ra,sa=0@31,fn=37@60 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1RefreshMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
# The source produces the shipped control flow and every memory operation, but
# IDO assigns two interchangeable integer webs to a1/a3 in the opposite order.
# Assert that bounded natural output before restoring the original coloring.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindBestRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x78 809f5debe1d6b9c0b303a57b1dee50b7e2845f4de16d6a1407d0c52b098901b4 \
		fields:0x4:rt=a1@a3 \
		fields:0x14:rs=a1@a3,rt=a1@a3 \
		fields:0x28:rs=a1@a3 \
		fields:0x2c:rd=a3@a1 \
		fields:0x30:rt=a3@a1 \
		fields:0x34:rs=a3@a1 \
		fields:0x38:rs=a3@a1 \
		fields:0x40:rs=a3@a1 \
		fields:0x4c:rd=a3@a1 \
		fields:0x54:rd=a3@a1 \
		fields:0x58:rs=a3@a1 \
		fields:0x60:rd=a3@a1 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ReleaseRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xBC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7EntryPool.c.o: \
	config/normalizations/overlay7EntryPool.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7EntryPool.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x228 d6abdb59d4da45cdde13fe7b0c5af949f6d35705270c836ce227406fa827e2f1 \
		@config/normalizations/overlay7EntryPool.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x1ac:gOverlay7ActiveTail:gOverlay7FreeHead \
		0x1b0:gOverlay7ActiveTail:gOverlay7FreeHead \
		0x1b8:gOverlay7FreeHead:gOverlay7ActiveTail \
		0x1bc:gOverlay7FreeHead:gOverlay7ActiveTail && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0ac:5:gOverlay7ActiveHead 0x0b0:6:gOverlay7ActiveHead \
		0x134:5:gOverlay7PriorityThresholdReloc \
		0x138:6:gOverlay7PriorityThresholdReloc \
		0x1e8:5:gOverlay7ActiveHead 0x1ec:6:gOverlay7ActiveHead \
		0x200:5:gOverlay7ActiveTail 0x1f4:6:gOverlay7ActiveTail && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x228
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7FillValues.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7AppendEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7CreateEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7DispatchModes.c.o: \
	config/normalizations/overlay7DispatchModes.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7DispatchModes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x20C 435a35e71a8bf9f44f5e23f27eb48150b77270f547f7878f58663264c0cba965 \
		@config/normalizations/overlay7DispatchModes.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x09c:5:.rodata 0x0a4:6:.rodata && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7UpdateOwnerMode.c.o: \
	config/normalizations/overlay7UpdateOwnerMode.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7UpdateOwnerMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x22C 3b59c6808cc676c820bba98bb452e58b6e877a5d4a6196406b9d3109b14601f0 \
		@config/normalizations/overlay7UpdateOwnerMode.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x22C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7DispatchSelection.c.o: \
	config/normalizations/overlay7DispatchSelection.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7DispatchSelection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xF0 0e69ee14c341330e203924dff3bb4ed4891331b9c2af33c38f32e97f40550320 \
		@config/normalizations/overlay7DispatchSelection.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7CommitSelection.c.o: \
	config/normalizations/overlay7CommitSelection.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7CommitSelection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x120 cd485cae05f6cfd07bb6e7ed43504d6f752eb58a44c109f52699ba808a85c6b9 \
		@config/normalizations/overlay7CommitSelection.ops && \
	$(OBJCOPY) --redefine-sym \
		func_overlay_007_F0000CCC_185CB54=overlay7DispatchSelectionReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x120
# This pool initializer is naturally instruction-exact. Its ten local-BSS
# records are already owned by overlay 7's shipped runtime relocation table,
# so retain their exact zero-base addends without static-link adjustment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7InitPool.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7InitPool.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x004:5:.bss 0x00c:6:.bss \
		0x000:5:.bss 0x008:6:.bss \
		0x094:5:.bss 0x098:6:.bss \
		0x09c:5:.bss 0x0a0:6:.bss \
		0x0a4:5:.bss 0x0ac:6:.bss
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3UpdateTimedEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectTarget.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x150
# The typed body naturally reproduces the complete call/CFG/FP inventory.
# Select its one relocation-aware carrier cycle and complete private owner webs.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3RunCachedModeAction.c.o: \
	config/normalizations/overlay3RunCachedModeAction.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3RunCachedModeAction.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1C4 f7109164aeadaedc78ab0434fec6adef226d03d8407816203386b6b516931ffd \
		@config/normalizations/overlay3RunCachedModeAction.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C4
# The natural source has the exact 77-word operation/CFG topology. Select the
# one complete four-use temporary allocation web with field-only guards.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3FindClosestObject.c.o: \
	config/normalizations/overlay3FindClosestObject.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3FindClosestObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x134 658b592060c9c7207af61b558ce3b23cf64599cc57d8d440774e0d491d09b24e \
		@config/normalizations/overlay3FindClosestObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
# The measured R4300 multiply-hazard flag supplies the target FP spacing nop.
# Then select the complete carrier/schedule and two stack-owner webs.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectScoredObject.c.o: \
	config/normalizations/overlay3SelectScoredObject.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectScoredObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectScoredObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1D8 5b163c78f21a5a4959f67b415663a42c1288e189e676f64809982e65d4e30495 \
		@config/normalizations/overlay3SelectScoredObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1D8
# The natural source has the exact 34-word operation/CFG topology. Select the
# complete caller-saved allocation web with register-field-only guards.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3TouchObject.c.o: \
	config/normalizations/overlay3TouchObject.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3TouchObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x88 b53d77d751d4d2bbd441808759afa529f10e142c5309b2df5c88b627b2f65837 \
		@config/normalizations/overlay3TouchObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19Dispatch.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xAC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildOutput.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
# The typed two-pass plane builder owns the exact 532-instruction CFG, frame,
# memory/call topology, and FP behavior. Select its complete guarded private
# schedule/allocation representation, preserve the four runtime call roles,
# then trim only the compiler's independent section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildPlanes.c.o: \
	config/normalizations/overlay19BuildPlanes.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildPlanes.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildPlanes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x850 4ca7cde8eac58b704c340b7f803d43533f98b78cbafc2242f47f6a1f8920b2a6 \
		@config/normalizations/overlay19BuildPlanes.ops && \
	$(OBJCOPY) --redefine-sym \
		o19AllocateReloc=func_overlay_019_F0000000_1875258 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x250:sqrtf:func_overlay_019_F0000000_1875258 \
		0x5E4:sqrtf:func_overlay_019_F0000000_1875258 \
		0x7FC:o19FreeReloc:func_overlay_019_F0000000_1875258 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x850
# IDO reproduces A30's complete CFG, memory topology, call site, frame, and
# size; only coherent interchangeable integer allocation webs differ. Assert
# every natural word before selecting the shipped register assignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildAdjacency.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1ec b034e0d88b0aabd4acb719a4955a98ba6317e7dc140dcea531edb9b5495e2a82 \
		fields:0x7c:rt=t4@t2 \
		fields:0x80:rd=t5@t3 \
		fields:0x84:rt=t5@t3 \
		fields:0x88:rt=t4@t2 \
		fields:0x8c:rt=t6@t4 \
		fields:0x90:rt=t8@t6 \
		fields:0x98:rs=t6@t4 \
		fields:0xa0:rt=t7@t5 \
		fields:0xa8:rs=t7@t5,rt=t8@t6,rd=t9@t7 \
		fields:0xac:rs=t9@t7 \
		fields:0xb0:rt=v0@t8 \
		fields:0xb4:rd=t1@t9 \
		fields:0xb8:rs=v0@t8,rt=t1@t9 \
		fields:0xd0:rt=t6@t7 \
		fields:0xd8:rd=t0@t1 \
		fields:0xe8:rs=t0@t1 \
		fields:0xf4:rd=v1@t0 \
		fields:0xf8:rd=v1@t0 \
		fields:0x10c:rt=v1@t0,rd=t2@t3 \
		fields:0x110:rs=t2@t3,rt=t3@t4 \
		fields:0x114:rt=t0@t1 \
		fields:0x118:rt=t1@t2 \
		fields:0x11c:rs=t3@t4,rd=t4@v1 \
		fields:0x120:rt=t4@v1 \
		fields:0x130:rs=t1@t2 \
		fields:0x13c:rd=t0@t1 \
		fields:0x15c:rt=t1@t2 \
		fields:0x160:rd=t2@t3 \
		fields:0x164:rs=t1@t2,rt=t2@t3,rd=t3@t4 \
		fields:0x168:rs=t3@t4,rd=t4@t6 \
		fields:0x16c:rs=t4@t6 \
		fields:0x17c:rt=t6@t7 \
		fields:0x180:rt=t8@t5 \
		fields:0x188:rs=t6@t7,rt=t7@t8 \
		fields:0x18c:rt=t8@t5 \
		fields:0x194:rt=t7@t8 \
		fields:0x19c:rt=t1@t9 \
		fields:0x1a0:rt=t5@t3 \
		fields:0x1a4:rs=t1@t9 \
		fields:0x1ac:rs=t5@t3,rt=t9@t4 \
		fields:0x1b0:rt=t9@t4 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19FindAdjacent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
# The edge classifier is naturally exact in size and all 120 opcodes. Four
# independent load pairs and one complete six-use temporary web differ only in
# schedule/register choice; assert the full natural residual before restoring
# the shipped representation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19ClassifyEdge.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1e0 4b047b0a8619b25039a9ad60af753b3d786c83015149335da9a7bb13c1e7dd18 \
		fields:0x68:rs=t0@v0,rt=a1@a3 \
		fields:0x6c:rs=v0@t0,rt=a3@a1 \
		fields:0x84:rs=t0@v0,rt=a1@a3 \
		fields:0x88:rs=v0@t0,rt=a3@a1 \
		fields:0x138:rt=v1@t3 \
		fields:0x140:rt=v1@t3 \
		fields:0x148:rs=v1@t3 \
		fields:0x154:rt=v1@t3 \
		fields:0x15c:rt=v1@t3 \
		fields:0x164:rs=v1@t3 \
		fields:0x194:rs=a1@v0,rt=v1@a3 \
		fields:0x198:rs=v0@a1,rt=a3@v1 \
		fields:0x1b0:rs=a1@v0,rt=v1@a3 \
		fields:0x1b4:rs=v0@a1,rt=a3@v1
# IDO naturally reproduces F58's complete CFG, stack frame, memory effects,
# loops, and exact size. The natural residual is exactly two complete register
# webs plus four independent schedule pairs; assert every natural word before
# restoring the shipped representation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildSpatialMasks.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x38c 5d048949588b86d990482e96dd0604243512f7091f5ad2b207809166ab921040 \
		fields:0x58:rd=v1@a0 \
		fields:0x5c:rs=v1@a0,rt=a0@v1 \
		fields:0x60:rs=v1@a0 \
		fields:0x64:rs=v1@a0 \
		fields:0x68:rs=v1@a0 \
		fields:0x6c:rs=a0@v1 \
		fields:0x74:rs=a0@v1 \
		fields:0x80:rt=a0@v1 \
		fields:0x88:rt=t2@t1 \
		fields:0xb4:rt=t1@t2 \
		fields:0xc0:rd=t0@a2 \
		fields:0xc4:rd=a2@t0 \
		fields:0xf4:rs=t2@t1 \
		fields:0xfc:rt=t1@t2 \
		fields:0x100:rd=t2@t1 \
		fields:0x104:rt=t2@t1 \
		fields:0x108:rd=t2@t1 \
		fields:0x10c:rt=t1@t2 \
		fields:0x118:rd=t1@t2 \
		fields:0x11c:rt=t1@t2 \
		fields:0x120:rd=t1@t2 \
		fields:0x198:rd=v1@a0 \
		fields:0x1b8:rt=v1@a0 \
		fields:0x1c0:rs=zero@t9 \
		fields:0x1c4:rs=t9@zero \
		fields:0x1c8:rt=t1@t2 \
		fields:0x1d4:rs=t2@t1,rt=v1@a0 \
		fields:0x1dc:rs=a0@v1,rt=a0@v1 \
		fields:0x1e4:rs=a0@v1,rt=a0@v1 \
		fields:0x1e8:rt=a0@v1 \
		fields:0x1f0:rt=t6@t7,rd=v0@v1 \
		fields:0x1f4:rt=t7@t6,rd=a0@v0 \
		fields:0x1f8:rs=v1@a0,rd=v1@a0 \
		fields:0x1fc:rs=a0@v1 \
		fields:0x200:rt=v1@a0 \
		fields:0x208:rs=t6@zero,rt=zero@t8,rd=a1@a0,sa=0@16,fn=37@3 \
		fields:0x210:rs=zero@t6,rt=t8@zero,rd=v1@a1,sa=16@0,fn=3@37 \
		fields:0x21c:rd=a0@v1 \
		fields:0x220:rd=v1@a0 \
		fields:0x240:rt=v1@a0 \
		fields:0x248:rd=v1@a0 \
		fields:0x258:rt=v1@a0 \
		fields:0x260:rs=a0@v1,rt=a0@v1 \
		fields:0x268:rs=a0@v1,rt=a0@v1 \
		fields:0x26c:rt=a0@v1 \
		fields:0x274:rd=a0@v1 \
		fields:0x27c:rs=v1@a0,rd=v1@a0 \
		fields:0x280:rs=a0@v1 \
		fields:0x284:rt=v1@a0 \
		fields:0x28c:rd=v1@a0 \
		fields:0x2a0:rd=a0@v1 \
		fields:0x2a4:rd=v1@a0 \
		fields:0x2c4:rt=v1@a0 \
		fields:0x2cc:rd=v1@a0 \
		fields:0x2dc:rt=v1@a0 \
		fields:0x2e4:rs=a0@v1,rt=a0@v1 \
		fields:0x2ec:rs=a0@v1,rt=a0@v1 \
		fields:0x2f0:rt=a0@v1 \
		fields:0x2f8:rd=a0@v1 \
		fields:0x300:rs=v1@a0,rd=v1@a0 \
		fields:0x304:rs=a0@v1 \
		fields:0x308:rt=v1@a0 \
		fields:0x310:rd=v1@a0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4GroupCount.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
# This initializer is naturally exact. Its data address pair is already owned
# by the shipped runtime relocation table; retain its zero addends and trim
# only compiler section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4InitializeObjectMotion.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4InitializeObjectMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x128:5:gOverlay4InitStatus 0x12c:6:gOverlay4InitStatus && \
	$(OBJCOPY) --redefine-sym \
		func_8005AD64=overlay4RuntimeCallReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
# The typed object naturally recovers the full CFG, eleven calls, frame, and
# operation stream. Select two complete GPR webs and the complete stack packet
# base web, then discard only compiler section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateObjectMotion.c.o: \
	config/normalizations/overlay4UpdateObjectMotion.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateObjectMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateObjectMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x398 0acbb9a638a75499a6e2f541da1ff95bf42e3a92f878bcc0761ad107f110dcf0 \
		@config/normalizations/overlay4UpdateObjectMotion.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x088:func_8002997C:func_8005ABA8 \
		0x13c:func_8002AA0C:func_8005ABA8 \
		0x168:func_8002997C:func_8005ABA8 \
		0x184:func_8002AA0C:func_8005ABA8 \
		0x1a8:func_80029274:func_8005ABA8 \
		0x20c:func_8002A8C0:func_8005ABA8 \
		0x27c:func_80004590:func_8005ABA8 \
		0x310:func_8000590C:func_8005ABA8 \
		0x328:func_8002997C:func_8005ABA8 \
		0x354:func_overlay_036_F00007B0:func_8005ABA8 && \
	$(OBJCOPY) --redefine-sym \
		func_8005ABA8=overlay4RuntimeCallReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x398
# The typed chain update naturally recovers its complete FP/CFG/call topology.
# Select the sole complete entry-allocation web and preserve runtime call zero.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateGroupSpacing.c.o: \
	config/normalizations/overlay4UpdateGroupSpacing.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateGroupSpacing.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateGroupSpacing.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x140 424b5e1ec9c342f91c36ca9f3a96132c68ee5502ef46701ecbf7a3d61f8462a6 \
		@config/normalizations/overlay4UpdateGroupSpacing.ops && \
	$(OBJCOPY) --redefine-sym sqrtf=overlay4RuntimeCallReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x140
# The natural four-way unrolled source has exact operation/CFG topology.
# Select its complete 37-word register-allocation web with field-only guards.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4FindCategory2Object.c.o: \
	config/normalizations/overlay4FindCategory2Object.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4FindCategory2Object.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1C0 ce564e6659518f49475cb959909161b686e125be7570c77be31c9655d35e6677 \
		@config/normalizations/overlay4FindCategory2Object.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C0
# The measured multiply-hazard mode supplies all fourteen target FP nops.
# Select the one complete six-use start/end stack-home allocation web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4FindSearchPosition.c.o: \
	config/normalizations/overlay4FindSearchPosition.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4FindSearchPosition.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4FindSearchPosition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3B8 5dae864d45fdcc2a13114b94e3dbb5b7cedafa75f1c06d392610871691f04e76 \
		@config/normalizations/overlay4FindSearchPosition.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4AttachObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4RemoveObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8Ignore.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8GetIndexed.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
# The natural IDO body owns all 381 opcodes, control flow, stack homes, and
# memory effects. Select its exhaustive private register/addend carrier, then
# retain exactly the retail R26 surface and bind its asserted call carriers.
O8_0894_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/func_overlay_008_F0000894_185E5EC.c.o
$(O8_0894_OBJ): \
	config/normalizations/func_overlay_008_F0000894_185E5EC.ops \
	config/normalizations/func_overlay_008_F0000894_185E5EC.filter.spec \
	config/normalizations/func_overlay_008_F0000894_185E5EC.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(O8_0894_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x5F4 8f51967ca5b6470c5c2cb896dab3a5fc98b388e71057a5fdd2fb8220c0077a8f \
		@config/normalizations/func_overlay_008_F0000894_185E5EC.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_008_F0000894_185E5EC.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_008_F0000894_185E5EC.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8StartMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x94
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8Activate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE4
# The natural phase machine owns its arithmetic, FP behavior, two calls,
# seven likely branches, memory effects and 161/165 opcode roles. Select one
# complete private state/scheduler carrier, then bind the five loader-local
# float-table addends and remove only the 18 runtime-owned relocation rows.
O8_1000_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/func_overlay_008_F0001000_185ED58.c.o
$(O8_1000_OBJ): \
	config/normalizations/func_overlay_008_F0001000_185ED58.normalize.py \
	config/normalizations/func_overlay_008_F0001000_185ED58.ops \
	config/normalizations/func_overlay_008_F0001000_185ED58.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(O8_1000_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/func_overlay_008_F0001000_185ED58.normalize.py $@ $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x294 447d8e3db05fa052018717059ce0077be2ff0c27993ad9a15d215fc10966f44c \
		@config/normalizations/func_overlay_008_F0001000_185ED58.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_008_F0001000_185ED58.filter.spec
# Natural IDO with the measured R4300 multiply-hazard mode owns the exact
# frame, size, CFG, calls, and opcode population. Select the complete bounded
# private schedule/home/allocation carrier, then remove only the 14
# loader-represented relocation rows.
O8_2640_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/func_overlay_008_F0002640_1860398.c.o
$(O8_2640_OBJ): \
	config/normalizations/func_overlay_008_F0002640_1860398.schedule.ops \
	config/normalizations/func_overlay_008_F0002640_1860398.fields.ops \
	config/normalizations/func_overlay_008_F0002640_1860398.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(O8_2640_OBJ): CFLAGS += -Wab,-r4300_mul
$(O8_2640_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2DC fb18de90797ce78d2a21e50989ea4f4ea08a78b9f49152c0ed1d83c225fbefb9 \
		@config/normalizations/func_overlay_008_F0002640_1860398.schedule.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2DC 51f108dbaa1859a5cee365cdd041f8f29722cd21910bc242d1381fdc0edbb103 \
		@config/normalizations/func_overlay_008_F0002640_1860398.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2DC && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_008_F0002640_1860398.filter.spec
# The measured hazard mode naturally owns all 361 opcodes, calls, branches,
# delay slots, and memory effects. Select the exhaustive private FP/home/addend
# representation web, then retain and bind the exact shipped R26 table.
O8_291C_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/func_overlay_008_F000291C_1860674.c.o
$(O8_291C_OBJ): \
	config/normalizations/func_overlay_008_F000291C_1860674.ops \
	config/normalizations/func_overlay_008_F000291C_1860674.filter.spec \
	config/normalizations/func_overlay_008_F000291C_1860674.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(O8_291C_OBJ): CFLAGS += -Wab,-r4300_mul
$(O8_291C_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x5A4 d2a5b9095e2acf61b9384e36e065f0600e42f5ead304207a88a9b7acb89e85e8 \
		@config/normalizations/func_overlay_008_F000291C_1860674.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_008_F000291C_1860674.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_008_F000291C_1860674.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8UpdateChild.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x158
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8UpdateChannels.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x260 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3dcccccdbdcccccdbf2b851f3f7333333d4ccccd000000000000000000000000 \
		0x1BC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8ApplyColors.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF0
# IDO reverses two independent threshold FP webs while reproducing every
# GPR, temporary, opcode, branch, and delay slot. Assert the complete natural
# four-word f0/f2 web before restoring the shipped allocation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8ScaleOutputs.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x138 b862f7ff2adc508b8af1ba159e5d0908fa343bfff5af1457ad4fa428905031ca \
		fields:0x34:rt=zero@v0 \
		fields:0x3c:rt=v0@zero \
		fields:0x80:rd=2@0 \
		fields:0xb4:rt=0@2 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8SetBuffer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8WriteCommand.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8SetValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8UpdateMotionOutput.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8UpdateMotionOutput.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x308 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3d23d70a000000000000000000000000 0x28C
# Natural IDO owns the exact 270-word frame/opcode/CFG/call/GPR topology.
# Select the complete private FP and normal-home web, externalize the asserted
# retained O8 literal pool, and leave the following two zero words as padding.
O8_4CF0_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o008/func_overlay_008_F0004CF0_1862A48.c.o
$(O8_4CF0_OBJ): \
	config/normalizations/func_overlay_008_F0004CF0_1862A48.ops \
	config/normalizations/func_overlay_008_F0004CF0_1862A48.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(O8_4CF0_OBJ): CFLAGS += -Wab,-r4300_mul
$(O8_4CF0_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x438 c7b816c168f4d59e28f2fc77bb9616f09e50ef2baab80d0a241264194f425f28 \
		@config/normalizations/func_overlay_008_F0004CF0_1862A48.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3f6666663f6666663ecccccd3e4ccccd3d4ccccd3cccccc03f79999a00000000 \
		0x290 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x438 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_008_F0004CF0_1862A48.filter.spec
# Natural IDO owns the exact size/frame/CFG/call topology. A complete guarded
# private schedule/register/scalar-address web selects retail's representation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateObjectState.c.o: \
	config/normalizations/overlay9UpdateObjectState.ops \
	config/normalizations/overlay9UpdateObjectState.filter.spec \
	config/normalizations/overlay9UpdateObjectState.rebind.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateObjectState.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateObjectState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x540 fe575274565167da66b29c063560060334cc1d8eed83fc077026449f32a4f1ca \
		@config/normalizations/overlay9UpdateObjectState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateObjectState.filter.spec && \
	$(HOST_PYTHON) config/normalizations/overlay9UpdateObjectState.rebind.py $@
# Natural IDO owns the exact 129-word frame/opcode/CFG/call/GPR topology. The
# complete guarded FP-carrier web selects retail's equivalent private FP
# allocation and three local addends; loader LOCAL roles remain authoritative.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateAngle.c.o: \
	config/normalizations/overlay9UpdateAngle.ops \
	config/normalizations/overlay9UpdateAngle.filter.spec \
	config/normalizations/overlay9UpdateAngle.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateAngle.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x204 ec43b6c99211636dfc52333543b14041af785b00ead777ee407a16f760213718 \
		@config/normalizations/overlay9UpdateAngle.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateAngle.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_2a5bc=func_overlay_009_F0000000_1866678 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateAngle.calls.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x204 \
		000000000000000000000000
# Natural IDO owns the exact frame/opcode/CFG/call/FP topology. The complete
# guarded web selects two loader-local addends and one private handle carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateOutput.c.o: \
	config/normalizations/overlay9UpdateOutput.ops \
	config/normalizations/overlay9UpdateOutput.filter.spec \
	config/normalizations/overlay9UpdateOutput.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateOutput.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateOutput.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x278 bb35c0cec7e14ae5de26d1c667efde61c629f365203ec2c8ba234ea2565425e5 \
		@config/normalizations/overlay9UpdateOutput.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateOutput.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_2b90=func_overlay_009_F0000000_1866678 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateOutput.calls.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x278 \
		0000000000000000
# Natural IDO reproduces every instruction class, CFG edge, delay slot, GPR,
# FP register, operand, and store. Twelve guarded compiler-pool LO addends bind
# the retained O9 literal pool; the loader remains sole owner of all 24 LOCAL
# HILO records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateInputState.c.o: \
	config/normalizations/overlay9UpdateInputState.ops \
	config/normalizations/overlay9UpdateInputState.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateInputState.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateInputState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x328 515c89bbe137c55c764b3adff3ced5d1c95becc06dccbfdd52ea1d09b5fd33fa \
		@config/normalizations/overlay9UpdateInputState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateInputState.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3ca3d70a3d99999a3ccccccd3d4ccccd3dcccccd43b680003f733333bc23d70a3c23d70abecccccdbdcccccd00000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x328 \
		0000000000000000
# Natural IDO owns the exact 162-word opcode, CFG, call, GPR, FPR, and memory
# topology. A guarded complete private frame/home web plus one loader-local
# literal addend selects retail's representation; runtime roles stay external.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9IntegrateVelocity.c.o: \
	config/normalizations/overlay9IntegrateVelocity.ops \
	config/normalizations/overlay9IntegrateVelocity.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9IntegrateVelocity.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9IntegrateVelocity.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x288 f8b306d7537deaa2cd9d0b319d4a8612738246783cfa2cfd252a43530fd1345a \
		@config/normalizations/overlay9IntegrateVelocity.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9IntegrateVelocity.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3f266666000000000000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x288 \
		0000000000000000
# Natural IDO owns the exact frame, opcode/CFG/call/register/FP topology, and
# all 78 executable words. Three asserted private stack/local addends select
# retail's representation; the narrow filter removes only the two compiler
# HILO records whose runtime LOCAL roles remain in the shipped loader table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9ResolveHeight.c.o: \
	config/normalizations/overlay9ResolveHeight.ops \
	config/normalizations/overlay9ResolveHeight.filter.spec \
	config/normalizations/overlay9ResolveHeight.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9ResolveHeight.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9ResolveHeight.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x138 1df46f7e2f9bfed32779736821eff05a16a7978617a332a08ea1108c3c6aacc4 \
		@config/normalizations/overlay9ResolveHeight.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9ResolveHeight.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_1353c=func_overlay_009_F0000000_1866678 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay9ResolveHeight.calls.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138 \
		0000000000000000
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9Ignore.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10
# The natural owner reproduces the exact 282-word opcode/CFG/call topology.
# This complete decoded-field ledger selects retail's equivalent private
# integer/FP allocation and four stack-home roles; the narrow filter removes
# only 16 asserted compiler-local relocations whose addends are already encoded
# in the split target. Runtime loader relocations remain in the original table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateMotion.c.o: \
	config/normalizations/overlay9UpdateMotion.ops \
	config/normalizations/overlay9UpdateMotion.filter.spec \
	config/normalizations/overlay9UpdateMotion.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x468 0895fd970e232f2216173e09e04991e6b73aba5db5e4331d093b66a987c7b8f2 \
		@config/normalizations/overlay9UpdateMotion.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateMotion.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_210b4=func_overlay_009_F0000000_1866678 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay9UpdateMotion.calls.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x46C
# Mickey extends the DKR particle-asset initializer with state resets and
# palette construction. A hash-guarded equivalent index carrier consumes only
# natural alignment words; the complete private allocation web then selects
# retail. Runtime HILO roles stay in the loader and six raw call carriers stay
# in the split object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeParticleAssets.c.o: \
	config/normalizations/overlay31InitializeParticleAssets.prepare.py \
	config/normalizations/overlay31InitializeParticleAssets.ops \
	config/normalizations/overlay31InitializeParticleAssets.filter.spec \
	config/normalizations/overlay31InitializeParticleAssets.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeParticleAssets.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay31InitializeParticleAssets.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x210 9d5a474fa6a92f962fcefbb4e932349b08e08b90e43988537c0ede59ef490539 \
		@config/normalizations/overlay31InitializeParticleAssets.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay31InitializeParticleAssets.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay31InitializeParticleAssets.calls.spec && \
	$(OBJCOPY) --remove-section=.data --remove-section=.rel.data \
		--remove-section=.gptab.data $@
# IDO naturally reproduces the complete 688-byte schedule, all calls, and all
# address pairs. Assert five complete private frame/register/order webs before
# selecting the shipped allocation; any source or compiler drift fails loudly.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o010/overlay10Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2b0 318d46afd0ff85cc2f5f6c4e5efb4165524e2124f3f4a8f584942dbbac7e089a \
		fields:0x0:imm=65432@65448 \
		fields:0x20:imm=92@76 \
		fields:0x28:imm=88@72 \
		fields:0x2c:rt=v1@a1,imm=92@76 \
		fields:0x30:rt=a0@a2,imm=88@72 \
		fields:0x3c:rs=v0@a3,rt=v0@a3,imm=0@320 \
		fields:0x40:rs=a3@v0,rt=a3@v0,imm=320@0 \
		fields:0x48:rs=v1@a1,rt=a1@v1 \
		fields:0x4c:rs=a0@a2,rt=a2@a0 \
		fields:0x68:rt=a1@v1 \
		fields:0x6c:rt=a2@a0 \
		fields:0x70:rt=v1@a1 \
		fields:0x74:rt=a0@a2 \
		fields:0xcc:rs=v0@v1,rt=v0@v1,imm=0@1024 \
		fields:0xd0:rs=v1@v0,rt=v1@v0,imm=1024@0 \
		fields:0x138:rt=v1@a0 \
		fields:0x13c:rs=v1@a0,rt=v1@a0 \
		fields:0x144:rs=v1@a0 \
		fields:0x14c:rd=a0@s0 \
		fields:0x150:rd=s0@v1 \
		fields:0x154:rs=v1@a0 \
		fields:0x158:rt=s0@v1 \
		fields:0x160:rs=v1@a0 \
		fields:0x164:rt=s0@v1 \
		fields:0x16c:rs=v1@a0 \
		fields:0x170:rt=s0@v1 \
		fields:0x174:rt=a0@s0 \
		fields:0x178:rs=v1@a0 \
		fields:0x17c:rs=a0@s0,rt=a0@s0 \
		fields:0x180:rt=s0@v1 \
		fields:0x184:rs=s0@v1,rt=s0@v1 \
		fields:0x188:rs=s0@v1 \
		fields:0x1f4:rs=s3@s5,rt=s3@s5,imm=0@16 \
		fields:0x1f8:rs=s5@s3,rt=s5@s3,imm=16@0 \
		fields:0x1fc:op=35@9,rs=s0@zero,rt=s1@s4,imm=0@256 \
		fields:0x200:op=9@35,rs=zero@s0,rt=s4@s1,imm=256@0 \
		fields:0x2ac:imm=104@88 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12Initialize.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14Reset.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReturnOne.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReturnOneCallbacks.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReleaseOwner.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14FinalizeActiveHandle.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_800053D0=overlay14LookupObjectReloc \
		--redefine-sym func_8001EF1C=overlay14ApplyObjectPositionReloc \
		--redefine-sym func_800280FC=overlay14AcquireFirstReloc \
		--redefine-sym func_800389C0=overlay14AcquireSecondReloc \
		--redefine-sym func_80027F24=overlay14SubmitHandleReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14CallUpdate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
# The typed source has the shipped CFG, opcode inventory, frame, calls, and
# complete 29-role runtime relocation surface. A guarded position-preserving
# field web selects the private register allocation; the six input-flag HILO
# records are loader-owned and excluded only from the static split object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14PrepareInputState.c.o: \
	config/normalizations/overlay14PrepareInputState.ops \
	config/normalizations/overlay14PrepareInputState.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14PrepareInputState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x20C d7ed2a16725f6dc7c380926886e232747cf3c70d0954b202f32a41b9f0c1cc55 \
		@config/normalizations/overlay14PrepareInputState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14PrepareInputState.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14ReadInput=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym gOverlay14PulseC=D_C \
		--redefine-sym gOverlay14Pulse10=D_10 \
		--redefine-sym gOverlay14Pulse14=D_14 \
		--redefine-sym gOverlay14Pulse18=D_18 \
		--redefine-sym gOverlay14Timer1C=D_1C \
		--redefine-sym gOverlay14Timer20=D_20 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20C
# The natural object already has the exact 127-instruction CFG/register web,
# frame, calls, and 40-role runtime topology. Twelve guarded local immediates
# select the overlay loader's representation; only its asserted loader-owned
# records are removed from the exact 12-record static split surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14AdvanceCommand.c.o: \
	config/normalizations/overlay14AdvanceCommand.ops \
	config/normalizations/overlay14AdvanceCommand.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14AdvanceCommand.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1FC 4aaeadfbac26d219d580afe743a9bf43c0783c6bdccd75bb46c52b2c43c81309 \
		@config/normalizations/overlay14AdvanceCommand.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14AdvanceCommand.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14InitializeMode=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym overlay14ResetMode=func_overlay_014_F0000498_186FD70 \
		--redefine-sym overlay14ApplyValues=func_overlay_014_F0000328_186FC00 \
		--redefine-sym gOverlay14Transition=D_D8 \
		--redefine-sym gOverlay14Cursor=D_DC $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1FC
# Natural IDO output already has the shipped 49-instruction schedule, frame,
# and calls. Five guarded local addends select the runtime loader form; twelve
# asserted loader-owned HILO records are omitted from the exact static object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14StepCommand.c.o: \
	config/normalizations/overlay14StepCommand.ops \
	config/normalizations/overlay14StepCommand.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14StepCommand.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xC4 eef6b2dc9131f3ae85cade95fc61bdd46dabc1c21ebc45db4ae626f4ad91f6cb \
		@config/normalizations/overlay14StepCommand.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14StepCommand.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14ResetMode=func_overlay_014_F0000498_186FD70 \
		--redefine-sym overlay14DispatchCommand=func_overlay_014_F0001040_1870918 \
		--redefine-sym overlay14MoveCommandCursor=func_overlay_014_F0000578_186FE50 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
# Natural IDO output has the exact 83-instruction schedule, frame, registers,
# CFG, calls, and complete 32-role runtime topology. Ten guarded local addends
# select the loader's representation; 22 asserted loader-owned data records are
# excluded from the exact ten-record static split surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00013F4_1870CCC.c.o: \
	config/normalizations/func_overlay_014_F00013F4_1870CCC.ops \
	config/normalizations/func_overlay_014_F00013F4_1870CCC.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00013F4_1870CCC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x14C b78abd1cc60d771d4e83717d88158e572b0a994e094d8f8bf3dfa2b08a9d464b \
		@config/normalizations/func_overlay_014_F00013F4_1870CCC.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F00013F4_1870CCC.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14BuildPanel=func_overlay_014_F00012D8_1870BB0 \
		--redefine-sym overlay14CreateHandle=func_overlay_014_F0001830_1871108 \
		--redefine-sym overlay14DrawPrimitive=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym gOverlay14Args2C=D_2C \
		--redefine-sym gOverlay14Args30=D_30 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14C
# The typed command renderer naturally has the exact 201-instruction size,
# 0x90 frame, call set, CFG semantics, switch arity, and runtime relocation
# roles. Five complete scheduling permutations and a bounded field web restore
# the shipped private compiler representation. The compiler's exact jump table
# already lives in retained overlay data, so assert and externalize that section
# and filter its private relocations along with six loader-owned text records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001830_1871108.c.o: \
	config/normalizations/func_overlay_014_F0001830_1871108.reorder.ops \
	config/normalizations/func_overlay_014_F0001830_1871108.fields.ops \
	config/normalizations/func_overlay_014_F0001830_1871108.filter.spec \
	config/normalizations/func_overlay_014_F0001830_1871108.rodata.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001830_1871108.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x324 469a14f29e44d6fdba796ba6e3a6b295ac3fbc517ac28e9e68b11194ada1d3f9 \
		@config/normalizations/func_overlay_014_F0001830_1871108.reorder.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x324 8368bed8d109ce43efc59ddb983fbf1ee3c19359820c50637beb44b054017a46 \
		@config/normalizations/func_overlay_014_F0001830_1871108.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		000001280000014c0000016c00000180000001a0000001b8000001f400000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F0001830_1871108.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .rodata \
		@config/normalizations/func_overlay_014_F0001830_1871108.rodata.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14Dispatch=func_overlay_014_F0000000_186F8D8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x324
# This typed panel renderer has the exact 188-instruction semantic CFG and
# 26-role runtime relocation topology. A complete guarded permutation and
# private field web select the shipped loop/register representation; ten
# asserted loader-owned data records are excluded from the static split while
# all calls and runtime data carriers remain intact.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001540_1870E18.c.o: \
	config/normalizations/func_overlay_014_F0001540_1870E18.reorder.ops \
	config/normalizations/func_overlay_014_F0001540_1870E18.fields.ops \
	config/normalizations/func_overlay_014_F0001540_1870E18.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001540_1870E18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2F0 0c45f6757af5cb5544056db29cc46e4e71337359e1760bb8e749b2d8050563d0 \
		@config/normalizations/func_overlay_014_F0001540_1870E18.reorder.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2F0 c14acbf2a4e14d315e30d08ac24132d48f767e43228484c2f2c50d3e68a6110f \
		@config/normalizations/func_overlay_014_F0001540_1870E18.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F0001540_1870E18.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14ValueC0=D_C0 \
		--redefine-sym overlay14BuildPanel=func_overlay_014_F00012D8_1870BB0 \
		--redefine-sym overlay14Dispatch=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym overlay14ValidateEntry=func_overlay_014_F0001028_1870900 \
		--redefine-sym gOverlay14Args34=D_34 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F0
# Natural IDO output has the exact size, frame, CFG, and opcode inventory for
# this range/asset fixup loop. One guarded two-instruction permutation and a
# complete private field web restore the shipped allocation; only the asserted
# loader-owned range HILO pair is removed from the static split surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00009F4_18702CC.c.o: \
	config/normalizations/func_overlay_014_F00009F4_18702CC.reorder.ops \
	config/normalizations/func_overlay_014_F00009F4_18702CC.fields.ops \
	config/normalizations/func_overlay_014_F00009F4_18702CC.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00009F4_18702CC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD8 3e18be097567b43e41c1e7fc662d1cbdef8ffade1fdfeb3bd15649c5a7621e43 \
		@config/normalizations/func_overlay_014_F00009F4_18702CC.reorder.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD8 fd6b3ca7203d465eebb463fb8aed2ff6d4263b82452f297df8d784cb5f7ba40d \
		@config/normalizations/func_overlay_014_F00009F4_18702CC.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F00009F4_18702CC.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14AssetCall=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym gOverlay14DefaultA4=D_A4 \
		--redefine-sym gOverlay14DefaultB4=D_B4 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
# The natural state-pop loop has the exact 56-instruction size, 0x30 frame,
# CFG, registers, branches, calls, and complete 18-role runtime carrier. A
# seven-instruction prologue permutation plus four guarded loader-local low
# addends restores the shipped representation without semantic changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ResetMode.c.o: \
	config/normalizations/overlay14ResetMode.reorder.ops \
	config/normalizations/overlay14ResetMode.fields.ops \
	config/normalizations/overlay14ResetMode.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ResetMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xE0 c0b3f0a15eb32fbb555ee161064d9c02876d52d29d41a062c363eb5c00e8e273 \
		@config/normalizations/overlay14ResetMode.reorder.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xE0 cc30aa838e9abb8d65d865534dbf679eff06fef8b0d6d19c1b2e2499d1a1a22c \
		@config/normalizations/overlay14ResetMode.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14ResetMode.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14CountEC=D_EC \
		--redefine-sym gOverlay14Commands128=D_128 \
		--redefine-sym gOverlay14CurrentTypeE4=D_E4 \
		--redefine-sym gOverlay14CurrentValueFC=D_FC \
		--redefine-sym overlay14Dispatch=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym overlay14Advance=func_overlay_014_F0000578_186FE50 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE0
# The reset source naturally preserves the full state teardown, 32-slot free
# loop, and 48-role runtime relocation topology. Three guarded redundant
# rematerializations are deleted before the complete retained-value/register
# web is applied. The configured split retains the target's six semantic HILO
# records and four calls in the assembler-authored REL record order.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0000000_186F8D8.c.o: \
	config/normalizations/func_overlay_014_F0000000_186F8D8.fields.ops \
	config/normalizations/func_overlay_014_F0000000_186F8D8.drop.filter.spec \
	config/normalizations/func_overlay_014_F0000000_186F8D8.numeric.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/order_o14_reset_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0000000_186F8D8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F0000000_186F8D8.drop.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x148 e03ea85ee3445e38277d130a461edbbef8300c56b2af0e989cb961a1f30149ae \
		'fields:0xb4:op=0xf@0x9' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x144 91cbeffdb7f057898439da4a6bf40e9341fc036b1056f5c37ab2101fe798bbe1 \
		'drop-li:0xb4:at:0:func_overlay_014_F0000000_186F8D8' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x140 5129cc853e9cbe4a22317da0cc84e0ba487ab1992bfefa49b41f1e3e2a7c6b7c \
		'drop-li:0xdc:t7:-1:func_overlay_014_F0000000_186F8D8' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x13C b29503160ebcccb2bc0390905a273efd09a91ffb879fc9236a9137bbf0abf69e \
		'drop-li:0xe4:t8:-1:func_overlay_014_F0000000_186F8D8' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x13C ef10204242360d04f113e27c14e1bca35ad873907e2069d87d67c9b806ac2e8a \
		@config/normalizations/func_overlay_014_F0000000_186F8D8.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x70:gOverlay14ValueSlotsEnd128:gOverlay14ValueCountE8 \
		0x7C:gOverlay14ValueSlotsEnd128:gOverlay14ValueCountE8 \
		0x74:gOverlay14ValueCountE8:gOverlay14ValueSlotsEnd128 \
		0x78:gOverlay14ValueSlots28:gOverlay14ValueSlotsEnd128 \
		0x80:gOverlay14ValueCountE8:gOverlay14ValueSlots28 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F0000000_186F8D8.numeric.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14ValueSlots28=D_28 \
		--redefine-sym gOverlay14ValueCountE8=D_E8 \
		--redefine-sym gOverlay14ValueSlotsEnd128=D_128 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x14:overlay14InitializeReloc:func_overlay_014_F0000000_186F8D8 \
		0x4C:overlay14FreeReloc:func_overlay_014_F0000000_186F8D8 \
		0x64:overlay14FreeReloc:func_overlay_014_F0000000_186F8D8 \
		0x9C:overlay14FreeReloc:func_overlay_014_F0000000_186F8D8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/order_o14_reset_relocations.py $@
# The natural rule-loader source has the exact 120-instruction opcode schedule,
# frame, CFG, and complete 20-role runtime topology. A guarded private field
# web selects the shipped value/register representation without reordering any
# instruction. Ten asserted loader-owned HILO records are removed from the
# exact static split surface before the semantic symbols are rebound.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F000013C_186FA14.c.o: \
	config/normalizations/func_overlay_014_F000013C_186FA14.fields.ops \
	config/normalizations/func_overlay_014_F000013C_186FA14.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F000013C_186FA14.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1E0 328fb50a8397684c94dff3bf16e66b9e178bd7ff55b02d835266df687ef599f4 \
		@config/normalizations/func_overlay_014_F000013C_186FA14.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F000013C_186FA14.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14AssetF0=D_F0 \
		--redefine-sym overlay14ResetReloc=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym overlay14TestRule=func_overlay_014_F000031C_186FBF4 \
		--redefine-sym overlay14ApplyValues=func_overlay_014_F0000328_186FC00 \
		--redefine-sym overlay14FinishLoad=func_overlay_014_F0000B40_1870418 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x30:overlay14AllocateReloc:func_overlay_014_F0000000_186F8D8 \
		0x40:overlay14GetVariantReloc:func_overlay_014_F0000000_186F8D8 \
		0x8C:overlay14AllocateReloc:func_overlay_014_F0000000_186F8D8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E0
# The pending-value and queue-append path naturally emits every shipped
# opcode, register, stack home, branch, delay slot, call, and relocation site.
# Twelve guarded loader-local low addends select the retained runtime carrier;
# the corresponding 24 HILO records are excluded from the static split.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ApplyValues.c.o: \
	config/normalizations/overlay14ApplyValues.fields.ops \
	config/normalizations/overlay14ApplyValues.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ApplyValues.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x170 eee54263588a9dbccb15d32cbe633d315943c32169590fb6c0561db61a050fa8 \
		@config/normalizations/overlay14ApplyValues.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14ApplyValues.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14StateC8=D_C8 \
		--redefine-sym gOverlay14CommandCountEC=D_EC \
		--redefine-sym gOverlay14ResultF8=D_F8 \
		--redefine-sym gOverlay14QueuedCommands128=D_128 \
		--redefine-sym overlay14CreateValue=func_overlay_014_F00006FC_186FFD4 \
		--redefine-sym overlay14MoveCommandCursor=func_overlay_014_F0000578_186FE50 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x170
# The typed selection scan has the exact size, frame, saved-register ABI,
# return shape, six calls, and eight static relocation identities. A guarded
# complete instruction permutation and private field web select the shipped
# loop schedule without adding, dropping, or duplicating instructions.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14MoveCommandCursor.c.o: \
	config/normalizations/overlay14MoveCommandCursor.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14MoveCommandCursor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x184 e8859d4d9dc4b913f126d2a316e577619c0fe9a05d27703ebd059f0d3844b8d4 \
		@config/normalizations/overlay14MoveCommandCursor.ops && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14SelectionList=D_FC \
		--redefine-sym overlay14IsSelectable=func_overlay_014_F0001028_1870900 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
# The exact-size table lookup/creation routine naturally has the shipped
# opcode census, call offsets, and all 15 relocations. Four asserted source
# aliases bound distinct slot lifetimes; after rebinding them to one retail
# identity, a guarded carrier permutation and private frame/register web
# select the shipped representation without changing any opcode.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14CreateValue.c.o: \
	config/normalizations/overlay14CreateValue.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14CreateValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x58:gOverlay14FreeSlots28:gOverlay14Slots28 \
		0x64:gOverlay14FreeSlots28:gOverlay14Slots28 \
		0x9C:gOverlay14ChosenSlots28:gOverlay14Slots28 \
		0xA0:gOverlay14ChosenSlots28:gOverlay14Slots28 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x180 376042d09f0f954cb0f041cc01b7b449627a0d4e499bd597c171f38ab4d189d7 \
		@config/normalizations/overlay14CreateValue.ops && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14Slots28=D_28 \
		--redefine-sym gOverlay14SlotsEnd128=D_128 \
		--redefine-sym gOverlay14SlotsActive2C=D_2C \
		--redefine-sym gOverlay14SlotCountE8=D_E8 \
		--redefine-sym overlay14SelectKind=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym overlay14LoadRelocatedValue=func_overlay_014_F000087C_1870154 \
		--redefine-sym overlay14LoadAsset=func_overlay_014_F00009F4_18702CC $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x180
# The natural switch routine has the exact 94-instruction opcode/CFG schedule,
# calls, delay slots, and six runtime identities. A guarded private frame and
# register-color web restores the shipped allocation, while the compiler jump
# table is asserted and externalized in favor of the overlay's retained owner.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14LoadRelocatedValue.c.o: \
	config/normalizations/overlay14LoadRelocatedValue.ops \
	config/normalizations/overlay14LoadRelocatedValue.filter.spec \
	config/normalizations/overlay14LoadRelocatedValue.rodata.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14LoadRelocatedValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x178 bfe49335a964de1e9ea9c2e261c18255b0593f6fb0a06ffd84f4dd39e905eba8 \
		@config/normalizations/overlay14LoadRelocatedValue.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14LoadRelocatedValue.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14AllocateReloc=func_overlay_014_F0000000_186F8D8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x50:overlay14LoadReloc:func_overlay_014_F0000000_186F8D8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		000000ac000000b8000000dc0000010000000124000001480000015400000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .rodata \
		@config/normalizations/overlay14LoadRelocatedValue.rodata.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x178
# The natural source has the exact 85-instruction CFG, opcode inventory, call
# sites, and runtime relocation topology. This guarded complete private field
# web selects the shipped register/home allocation without moving or changing
# any instruction. Loader-owned HILO and SYMBOL sites are then removed from the
# static ELF relocation surface; local JUMP calls remain linked normally.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14UpdateTransition.c.o: \
	config/normalizations/overlay14UpdateTransition.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14UpdateTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x154 663751d0cf55059866cba6b58992d57a9b40bbca4399761d8388b799cd87e3ba \
		@config/normalizations/overlay14UpdateTransition.ops && \
	$(OBJCOPY) \
		--redefine-sym overlay14PrepareReloc=func_overlay_014_F0000B5C_1870434 \
		--redefine-sym overlay14AdvanceReloc=func_overlay_014_F0000D68_1870640 \
		--redefine-sym overlay14RetreatReloc=func_overlay_014_F0000F64_187083C \
		--redefine-sym overlay14InitializeReloc=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym overlay14SetActiveReloc=func_8004B064 \
		--redefine-sym overlay14DrawPrimaryReloc=func_overlay_014_F00013F4_1870CCC \
		--redefine-sym overlay14DrawAlternateReloc=func_overlay_014_F0001540_1870E18 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x18:5:gOverlay14FlagC4 \
		0x1C:6:gOverlay14FlagC4 \
		0x20:5:gOverlay14TransitionValue \
		0x24:5:gOverlay14FlagC8 \
		0x2C:6:gOverlay14TransitionValue \
		0x30:6:gOverlay14FlagC8 \
		0x34:5:gOverlay14ModeE4 \
		0x40:6:gOverlay14ModeE4 \
		0x48:5:gOverlay14EnabledF8 \
		0x50:5:gOverlay14CommandHeader \
		0x54:6:gOverlay14EnabledF8 \
		0x70:6:gOverlay14CommandHeader \
		0x84:5:gOverlay14TransitionValue \
		0x8C:6:gOverlay14TransitionValue \
		0xD0:5:gOverlay14FlagC8 \
		0xD4:6:gOverlay14FlagC8 \
		0xE0:4:func_overlay_014_F0000000_186F8D8 \
		0xF4:4:func_8004B064 \
		0xFC:5:gOverlay14TransitionValue \
		0x100:6:gOverlay14TransitionValue \
		0x108:5:gOverlay14ModeE4 \
		0x114:6:gOverlay14ModeE4 \
		0x13C:4:func_8004B064 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x154
# The natural object has the shipped CFG, opcode multiset, frame, calls, and
# all 15 loader-relocated sites. A complete guarded permutation selects the
# private schedule/register web; runtime-only HILO relocations remain owned by
# the overlay tables, while the seven retained call carriers match raw asm.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14DispatchCommand.c.o: \
	config/normalizations/overlay14DispatchCommand.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14DispatchCommand.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x124 957988868f21ead10f784b77ae7d76b328f2668eacbaf3a64443ccadc05f34c0 \
		@config/normalizations/overlay14DispatchCommand.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x88:overlay14ApplyActionReloc:overlay14CallUpdate \
		0xA8:overlay14UpdateReloc:overlay14ApplyActionReloc \
		0xD0:overlay14CallUpdate:overlay14UpdateReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:gOverlay14CommandHeader \
		0x4:6:gOverlay14CommandHeader \
		0x7C:5:gOverlay14FlagC4 \
		0x84:6:gOverlay14FlagC4 \
		0xB8:5:gOverlay14ActiveHandle \
		0xBC:6:gOverlay14ActiveHandle \
		0xC0:5:gOverlay14FlagC8 \
		0xCC:6:gOverlay14FlagC8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124
# IDO places the local 24-entry rectangle array four bytes above the shipped
# slot while preserving the complete frame, access pattern, and semantics.
# Assert both natural base materializations before restoring the original slot.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14BuildRects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x11c 0c67c47f5baa9f69f9c158acacd7feca4a420d27729a3697a9adaf3920fc9be0 \
		fields:0x14:imm=40@36 \
		fields:0xfc:imm=40@36 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CallGlobal.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CallGlobal.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36InitObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27CanUse.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27Activate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41InterpolateAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
# The typed curve/object update naturally owns all 638 semantic instructions,
# 33 call sites, the exact 0xE0 frame, and all 49 runtime relocation roles.
# R4300 multiply-hazard scheduling recovers the shipped dependency nop. A
# complete pinned instruction/allocator web selects retail's private schedule;
# two pool HI16 carriers are restored to their runtime-table sites. Loader HILO
# roles stay in Overlay 41's ROM table, while the split object retains the exact
# 33 raw call carriers and no private compiler constant section.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateCurveObject.c.o: \
	config/normalizations/overlay41UpdateCurveObject.ops \
	config/normalizations/overlay41UpdateCurveObject.relocation_move.filter.spec \
	config/normalizations/overlay41UpdateCurveObject.static_hilo.filter.spec \
	config/normalizations/overlay41UpdateCurveObject.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/add_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateCurveObject.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateCurveObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x9F8 a3d4c36efd6fa24ceb9a02376ba0bfa83ef79d5496bbe45ae8aaa56a43af8f56 \
		@config/normalizations/overlay41UpdateCurveObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41UpdateCurveObject.relocation_move.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x9F8 a3d4c36efd6fa24ceb9a02376ba0bfa83ef79d5496bbe45ae8aaa56a43af8f56 \
		0x184:HI16:.rodata:0 0x190:HI16:.rodata:0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9F8 \
		0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41UpdateCurveObject.static_hilo.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		func_8003EDEC=func_overlay_041_F0000000_1887338 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay41UpdateCurveObject.rebind.spec && \
	$(OBJCOPY) --remove-section=.rodata --remove-section=.rel.rodata \
		--remove-section=.gptab.rodata --remove-section=.data \
		--remove-section=.rel.data --remove-section=.gptab.data $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41IsUnitScale.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
# Natural IDO owns the exact frame, CFG, opcodes, immediates, delay slots,
# call site, and fixed-point semantics. A complete guarded private temporary
# web selects retail's equivalent allocation; loader HILO roles remain in the
# runtime table while the raw call carrier is retained in the split object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AdvanceStepRecords.c.o: \
	config/normalizations/overlay41AdvanceStepRecords.ops \
	config/normalizations/overlay41AdvanceStepRecords.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AdvanceStepRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x124 314b0729f28df325aaed3db97cbd173f2e55b40d53894d3ef7acaa6d972fdbd2 \
		@config/normalizations/overlay41AdvanceStepRecords.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124 \
		000000000000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41AdvanceStepRecords.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay41EmitStep=func_overlay_041_F0000000_1887338 $@
# Natural IDO owns the exact 98-word CFG, calls, divide schedule, frame, and
# memory effects. Select only the complete private allocation web, retain the
# seven exact split-object relocations, and materialize proved runtime addends.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateColorRecords.c.o: \
	config/normalizations/overlay41UpdateColorRecords.ops \
	config/normalizations/overlay41UpdateColorRecords.filter.spec \
	config/normalizations/overlay41UpdateColorRecords.materialize.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/add_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateColorRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x188 68b49c4222ca84bfc9585394a234bab5241df8681bd816e0cb78c7d47d31f433 \
		@config/normalizations/overlay41UpdateColorRecords.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x188 \
		0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41UpdateColorRecords.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x128:overlay41SetColor:func_overlay_041_F0000000_1887338 \
		0x134:overlay41SetAlpha:func_overlay_041_F0000000_1887338 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x188 68b49c4222ca84bfc9585394a234bab5241df8681bd816e0cb78c7d47d31f433 \
		0x30:HI16:D_80000038:0x8000 \
		0x144:LO16:D_80000038:0x38 \
		0x148:LO16:D_80000039:0x39 \
		0x14c:LO16:D_8000003A:0x3a \
		0x150:LO16:D_8000003B:0x3b && \
	$(OBJCOPY) --remove-section=.data --remove-section=.rel.data \
		--remove-section=.gptab.data $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x188 aea876758946deb9eb9ca11f0d4c77029d5010a83d8a52ac0032c582b4054924 \
		@config/normalizations/overlay41UpdateColorRecords.materialize.ops
# Natural IDO owns the exact size, frame, CFG, switch, calls, constants, and
# first 295 instructions. A guarded private cubic schedule and complete FP web
# select retail; compiler-local pool relocations and sections are then removed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SampleCurve.c.o: \
	config/normalizations/overlay41SampleCurve.ops \
	config/normalizations/overlay41SampleCurve.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SampleCurve.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x550 e8a1e87b0748320ad7191431456751b39e9410f9409daf8083abd7dab2d1dcb9 \
		@config/normalizations/overlay41SampleCurve.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41SampleCurve.filter.spec && \
	$(OBJCOPY) --remove-section=.rodata --remove-section=.rel.rodata \
		--remove-section=.gptab.rodata --remove-section=.data \
		--remove-section=.rel.data --remove-section=.gptab.data $@
# Natural IDO owns all 115 semantic instructions and the complete CFG, frame,
# call, FP, memory, and branch topology. One guarded ten-instruction identity
# rotation plus its full register web selects retail's private allocation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateProgress.c.o: \
	config/normalizations/overlay41UpdateProgress.ops \
	config/normalizations/overlay41UpdateProgress.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateProgress.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1CC 5a2f73cfe46193543cc257aa6008b5fc6cf4d44318b511a41e2b93d7e3126885 \
		@config/normalizations/overlay41UpdateProgress.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1CC 00000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41UpdateProgress.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay41ApplyAmount=func_overlay_041_F0000000_1887338 $@
# The recovered body naturally owns the complete CFG, frame, call order,
# memory operations, and FP conversion. Guard and remove five proved input-home
# artifacts, then select the bounded private schedule/register web and restore
# the split object's raw static call carrier without collapsing runtime roles.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41ProcessEntry.c.o: \
	config/normalizations/overlay41ProcessEntry.prepare.py \
	config/normalizations/overlay41ProcessEntry.ops \
	config/normalizations/overlay41ProcessEntry.rebind.spec \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41ProcessEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay41ProcessEntry.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1EC \
		0000000000000000000000000000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1EC 112e13cb7343119f9400e841462be9e8a23c91fee05d1c6fd28ab4d67b2e3537 \
		@config/normalizations/overlay41ProcessEntry.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay41ProcessEntry.rebind.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay41StartEntry=func_overlay_041_F0000000_1887338 $@
# Natural codegen owns the exact 55-word CFG, seven-argument ABI, FP sequence,
# and four runtime relocation roles. The guarded bijection selects one private
# schedule/count-register web and materializes the proved LOCAL constant
# addend; the compiler's duplicate constant is asserted and externalized.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AddSlot.c.o: \
	config/normalizations/overlay41AddSlot.ops \
	config/normalizations/overlay41AddSlot.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AddSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xDC 1b4162322dcc7bb262a75dfa7f76c7adb3cfafdc0bb45f2d385eaa9ce0d36090 \
		@config/normalizations/overlay41AddSlot.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41AddSlot.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3f199999000000000000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC \
		00000000
# The typed switch/emit loop naturally reproduces all 135 instructions and
# every compiler relocation role. Assert the two loader-local initialized-data
# addends, retain the retail-owned table/constant island, and fold all five
# runtime-dispatched calls to the raw overlay carrier used by the split asm.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SpawnItems.c.o: \
	config/normalizations/overlay41SpawnItems.ops \
	config/normalizations/overlay41SpawnItems.filter.spec \
	config/normalizations/overlay41SpawnItems.rebind.spec \
	config/normalizations/overlay41SpawnItems.rodata.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SpawnItems.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x21C d05308a1a03b9f68c4e6b44409af6c9293977e929675b43db679ba179d2262ad \
		@config/normalizations/overlay41SpawnItems.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41SpawnItems.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		0000008c00000094000000a0000000b4000000c8bc23d70a0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .rodata \
		@config/normalizations/overlay41SpawnItems.rodata.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x21C \
		00000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay41SpawnItems.rebind.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay41RandomRange=func_overlay_041_F0000000_1887338 $@
# The natural seven-argument queue insertion owns every operation and runtime
# data role. Select retail's equivalent single-cursor schedule/register web,
# discard numeric runtime aliases from the static split object, and retain the
# one exact D_C pair used by the original owner.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41EnqueueTransition.c.o: \
	config/normalizations/overlay41EnqueueTransition.ops \
	config/normalizations/overlay41EnqueueTransition.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41EnqueueTransition.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41EnqueueTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A4 a5d6b02a7c4de089fb23d79ed51adc151a86c6fd71b556b9674f76cdea3b3b56 \
		@config/normalizations/overlay41EnqueueTransition.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay41EnqueueTransition.filter.spec && \
	$(OBJCOPY) --redefine-sym gOverlay41QueueEntries=D_C $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41TickTransitions.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41DrawItem.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41DrawItem.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8004B0A4=overlay41SetModeReloc \
		--redefine-sym func_8004B0DC=overlay41SetColorReloc \
		--redefine-sym func_8004BA8C=overlay41MeasureReloc \
		--redefine-sym func_8004B0B8=overlay41SetColorIntensityReloc \
		--redefine-sym func_8004B0F8=overlay41DrawResourceReloc \
		--redefine-sym gOverlay41Resources=gOverlay41ResourcesReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29BuildChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29UpdateRatio.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29UpdateRatio.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29Sample.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x198
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29HandleEffects.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29HandleEffects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x404
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29DrawGroups.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_029_F00014C8_187E778=overlay29DrawGroups $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x204
$(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay51Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay5InitSequence.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ConsumeTimer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1TestDirection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BuildPointRecord.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BuildPointRecord.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BuildPointRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x424
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ResetFlags.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagC4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagC8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReleaseCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15GetResource4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15GetResource10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15SetValueC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15ClearValue7C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawRain.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawRain.c.o: \
	config/normalizations/overlay15DrawRain.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawRain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x98:5:gOverlay15RainOffsetY && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD8 22794a3eb06c90ff76a04c471b1520590f9dfbcc70167818106ce34cdd6a715f \
		@config/normalizations/overlay15DrawRain.ops && \
	$(OBJCOPY) --redefine-sym \
		overlay15GetActiveCameraReloc=func_overlay_015_F0000000_1872398 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xc0:rainFastDraw:func_overlay_015_F0000000_1872398 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x04:5:gOverlay15RainEnabled 0x08:6:gOverlay15RainEnabled \
		0x24:5:gOverlay15RainCapacity 0x28:6:gOverlay15RainCapacity \
		0x30:5:gOverlay15RainPositions 0x38:6:gOverlay15RainPositions \
		0x64:5:gOverlay15RainPositions 0x70:6:gOverlay15RainPositions \
		0x68:5:gOverlay15RainColors 0x6c:6:gOverlay15RainColors \
		0x90:5:gOverlay15RainOffsetX 0x94:6:gOverlay15RainOffsetX \
		0x98:6:gOverlay15RainOffsetY \
		0x9c:5:gOverlay15RainOffsetZ 0xa0:6:gOverlay15RainOffsetZ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
# Exact-frame star/palette initializer. Remove only IDO's redundant second
# count-address pair, apply the complete 247-carrier relocation-anchored web,
# and claim the asserted final alignment nop as the owner's last instruction.
# Preserve the fourteen-role runtime proof; filter only the eight embedded
# loader-owned records for the exact split-object surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15InitStarsAndPalette.c.o: \
	config/normalizations/overlay15InitStarsAndPalette.ops \
	config/normalizations/overlay15InitStarsAndPalette.runtime.filter.spec \
	config/normalizations/overlay15InitStarsAndPalette.static.filter.spec \
	config/normalizations/overlay15InitStarsAndPalette.finalize.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15InitStarsAndPalette.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay15InitStarsAndPalette.runtime.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3DC c0d678e621f5a53e0a9b0fa385ea4c718acd6440dcc0a0760aadf65c08bc5f58 \
		@config/normalizations/overlay15InitStarsAndPalette.ops && \
	$(HOST_PYTHON) config/normalizations/overlay15InitStarsAndPalette.finalize.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay15InitStarsAndPalette.static.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay15Allocate=func_overlay_015_F0000000_1872398 $@ && \
	$(OBJCOPY) \
		--redefine-sym overlay15RandomRange=func_overlay_015_F0000000_1872398 $@ && \
	$(OBJCOPY) --redefine-sym gOverlay15Stars=D_4 $@
# Typed source supplies the complete ABI, effects, FP/register multiset, call,
# and all 21 runtime relocation roles. Four guarded drops remove only IDO's
# redundant odd-bound address anchors; the remaining guarded operations select
# shipped local addends and one bijective schedule of independent instructions.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15MoveStars.c.o: \
	config/normalizations/overlay15MoveStars.drop1.ops \
	config/normalizations/overlay15MoveStars.drop2.ops \
	config/normalizations/overlay15MoveStars.drop3.ops \
	config/normalizations/overlay15MoveStars.drop4.ops \
	config/normalizations/overlay15MoveStars.addends.ops \
	config/normalizations/overlay15MoveStars.schedule.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15MoveStars.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x4c:5:gOverlay15StarBound1 0x60:5:gOverlay15StarBound3 \
		0x7c:5:gOverlay15StarBound5 0xa4:5:gOverlay15StarBound7 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xE4 2413785409234123f60ba40391ffa00168807fded89a7af87131f43727905753 \
		@config/normalizations/overlay15MoveStars.drop1.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xE0 c1c1ec180e9e9ed81eb0c8f4ea1a002478c1195195850bebbe278778ac34a21d \
		@config/normalizations/overlay15MoveStars.drop2.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xDC 7169699270adcac03001b7565dedd2bc42d93dec7e7b417d64fb00056953d2be \
		@config/normalizations/overlay15MoveStars.drop3.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD8 9cebd8c8fb2a85b78aa36009b7b94df966c3224104726958335082e90ee75779 \
		@config/normalizations/overlay15MoveStars.drop4.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD8 d5e40706b4bf5f08c92651635fee8dcf0e61cc9cd9667725cbc5ca996e261bf6 \
		@config/normalizations/overlay15MoveStars.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD8 a51912f115546a60ae29b0b7ae36ad5ff3b2fbae783242f6bb5c76a645bfa90d \
		@config/normalizations/overlay15MoveStars.schedule.ops && \
	$(OBJCOPY) --redefine-sym \
		starfieldFastMove=func_overlay_015_F0000000_1872398 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x08:5:gOverlay15StarMovement 0x10:6:gOverlay15StarMovement \
		0x1c:5:gOverlay15Stars 0x24:6:gOverlay15Stars \
		0x40:5:gOverlay15StarBound0 0x48:6:gOverlay15StarBound0 \
		0x4c:6:gOverlay15StarBound1 \
		0x50:5:gOverlay15StarBound2 0x54:6:gOverlay15StarBound2 \
		0x58:6:gOverlay15StarBound3 \
		0x60:5:gOverlay15StarBound4 0x78:6:gOverlay15StarBound4 \
		0x98:6:gOverlay15StarBound5 \
		0x9c:5:gOverlay15StarBound6 0xa0:6:gOverlay15StarBound6 \
		0xa4:6:gOverlay15StarBound7 \
		0xa8:5:gOverlay15StarBound8 0xac:6:gOverlay15StarBound8 \
		0x80:5:gOverlay15StarCount 0x88:6:gOverlay15StarCount && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
# Typed source naturally supplies the exact R4300 hazard, frame, CFG, effects,
# calls, and full instruction multiset. Guarded decoded addends, one same-target
# branch displacement, and a bijective schedule select the shipped form.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawScreenStars.c.o: \
	config/normalizations/overlay15DrawScreenStars.addends.ops \
	config/normalizations/overlay15DrawScreenStars.branch.ops \
	config/normalizations/overlay15DrawScreenStars.schedule.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawScreenStars.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawScreenStars.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A4 a739cb4b1bcf13b0e3cf5f56b28855bd668bda486be647a6f3c6811efa274d90 \
		@config/normalizations/overlay15DrawScreenStars.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A4 7ed01dca2669863e5a77a554359ff5850bae9cb9c09b8052ddca1a150a05da71 \
		@config/normalizations/overlay15DrawScreenStars.branch.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A4 459d35d0fd4d236478ab6ba3b3b45727dd8ac2e1106282038578190f8e4b1eb0 \
		@config/normalizations/overlay15DrawScreenStars.schedule.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x24:5:gOverlay15StarCount 0x28:6:gOverlay15StarCount \
		0x30:5:gOverlay15Stars 0x34:6:gOverlay15Stars \
		0x3c:5:gOverlay15StarSetup 0x44:6:gOverlay15StarSetup \
		0x60:5:gOverlay15StarFadeScale 0x64:6:gOverlay15StarFadeScale && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A4
# Exact-frame star initializer. The fail-loud preparation assigns the natural
# alignment nop plus two explicit zero carriers to the owner; the complete
# address-lowering web expands three stores into three address addius plus the
# same stores. Preserve all 15 runtime roles and filter only the asserted
# second count HILO pair embedded by the split assembly.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15InitStars.c.o: \
	config/normalizations/overlay15InitStars.prepare.py \
	config/normalizations/overlay15InitStars.ops \
	config/normalizations/overlay15InitStars.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15InitStars.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay15InitStars.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2F8 6dd3d6d0bb33d73751c58f0aacdc1384297fa301b7409cdb3f6b1a2e90e7e044 \
		@config/normalizations/overlay15InitStars.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay15InitStars.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay15Allocate=func_overlay_015_F0000000_1872398 \
		--redefine-sym gOverlay15InitBounds=D_50 \
		--redefine-sym gOverlay15Stars=D_10 \
		--redefine-sym gOverlay15StarColors=D_98 \
		--redefine-sym gOverlay15StarCount=D_8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x1d8:overlay15RandomRange:func_overlay_015_F0000000_1872398 \
		0x1f4:overlay15RandomRange:func_overlay_015_F0000000_1872398 \
		0x210:overlay15RandomRange:func_overlay_015_F0000000_1872398 \
		0x22c:overlay15RandomRange:func_overlay_015_F0000000_1872398
# Typed source preserves the camera-delta/update ABI, all effects, exact call
# order, and all 39 runtime relocation roles. Seven fail-loud drops remove
# only redundant adjacent-field HI anchors; decoded addends and a bijective
# permutation select retail's equivalent independent schedule.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15UpdateMovingStars.c.o: \
	config/normalizations/overlay15UpdateMovingStars.drop1.ops \
	config/normalizations/overlay15UpdateMovingStars.drop2.ops \
	config/normalizations/overlay15UpdateMovingStars.drop3.ops \
	config/normalizations/overlay15UpdateMovingStars.drop4.ops \
	config/normalizations/overlay15UpdateMovingStars.drop5.ops \
	config/normalizations/overlay15UpdateMovingStars.drop6.ops \
	config/normalizations/overlay15UpdateMovingStars.drop7.ops \
	config/normalizations/overlay15UpdateMovingStars.addends.ops \
	config/normalizations/overlay15UpdateMovingStars.schedule.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15UpdateMovingStars.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x164:5:gOverlay15MovingBound7 0x148:5:gOverlay15MovingBound5 \
		0x12c:5:gOverlay15MovingBound3 0x11c:5:gOverlay15MovingBound1 \
		0xf8:5:gOverlay15CurrentPositionY 0xc8:5:gOverlay15PreviousCameraZ \
		0x6c:5:gOverlay15PreviousCameraZ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1B4 5646d6b173766b375e58e714526e0384d7f53271f66a483d2679a3066174fc00 \
		@config/normalizations/overlay15UpdateMovingStars.drop1.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1B0 d28cc59a2a4a4a3c02f1a67a6e8de19fa1ac284964ff006468f86b34a765ef65 \
		@config/normalizations/overlay15UpdateMovingStars.drop2.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1AC 1aa0172dadc837e4a02014254659b83a07a6108c0e64b5300af3218892252b67 \
		@config/normalizations/overlay15UpdateMovingStars.drop3.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A8 c3347c2638c186a9cea6791fc9d8edacf1fbbf3d704880e085ff28a8005d31f7 \
		@config/normalizations/overlay15UpdateMovingStars.drop4.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A4 96804816525ff3e0e6ca2fc5f1ed7a6f08347c4680e0bdd158b352613923a62c \
		@config/normalizations/overlay15UpdateMovingStars.drop5.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A0 e90d38731468cf53193027bd455fc1f1e93acaaa5360fa08d198c497470af231 \
		@config/normalizations/overlay15UpdateMovingStars.drop6.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x19C 4d92f3859b6458ddfebb2d2c6e0945749be6b0144948d3d5cfad0bcf0c24f224 \
		@config/normalizations/overlay15UpdateMovingStars.drop7.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x19C 3f3418076754414b054eaabdc354c23cfd2124fa71f5c450136011e1fc7ab8ed \
		@config/normalizations/overlay15UpdateMovingStars.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x19C 0b0c1d2c7c663444f10bf32a8ad23f40fa9c1af6250bd7296aa556d7d33f36fe \
		@config/normalizations/overlay15UpdateMovingStars.schedule.ops && \
	$(OBJCOPY) --redefine-sym \
		overlay15GetActiveCameraReloc=func_overlay_015_F0000000_1872398 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x180:starfieldFastMove:func_overlay_015_F0000000_1872398 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x20:5:gOverlay15CameraReadyRead 0x24:6:gOverlay15CameraReadyRead \
		0x50:5:gOverlay15PreviousCameraX 0x54:6:gOverlay15PreviousCameraX \
		0x5c:5:gOverlay15PreviousCameraY 0x68:6:gOverlay15PreviousCameraY \
		0x7c:6:gOverlay15PreviousCameraZ \
		0xa8:5:gOverlay15PreviousCameraX 0xac:6:gOverlay15PreviousCameraX \
		0xb4:5:gOverlay15PreviousCameraY 0xbc:6:gOverlay15PreviousCameraY \
		0xcc:6:gOverlay15PreviousCameraZ \
		0xd0:5:gOverlay15CameraReadyWrite 0xd8:6:gOverlay15CameraReadyWrite \
		0xdc:5:gOverlay15CurrentPositionX 0xec:6:gOverlay15CurrentPositionX \
		0xf0:6:gOverlay15CurrentPositionY \
		0xf4:5:gOverlay15CurrentPositionZ 0xfc:6:gOverlay15CurrentPositionZ \
		0xe0:5:gOverlay15MovingStars 0xe8:6:gOverlay15MovingStars \
		0x168:5:gOverlay15MovingStarCount 0x16c:6:gOverlay15MovingStarCount \
		0x104:5:gOverlay15MovingBound0 0x108:6:gOverlay15MovingBound0 \
		0x110:6:gOverlay15MovingBound1 \
		0x114:5:gOverlay15MovingBound2 0x118:6:gOverlay15MovingBound2 \
		0x11c:6:gOverlay15MovingBound3 \
		0x120:5:gOverlay15MovingBound4 0x138:6:gOverlay15MovingBound4 \
		0x134:6:gOverlay15MovingBound5 \
		0x140:5:gOverlay15MovingBound6 0x150:6:gOverlay15MovingBound6 \
		0x14c:6:gOverlay15MovingBound7 \
		0x154:5:gOverlay15MovingBound8 0x15c:6:gOverlay15MovingBound8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x19C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SetValue10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
# Natural source supplies the exact allocator calls, clear loops, complete
# register/schedule web, and all eight runtime relocations. Select retail's
# equivalent call-surviving size home and fold the true allocator identity to
# the module's pre-loader relocation carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34InitStorage.c.o: \
	config/normalizations/overlay34InitStorage.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34InitStorage.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xC8 f331cbe038338fb6cd06d63eaa4951de78791d505d92f7bebc8ab0906b193869 \
		@config/normalizations/overlay34InitStorage.ops && \
	$(OBJCOPY) --redefine-sym func_8002B280=overlay34AllocateReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34InterpolateColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
# Natural source owns the exact boundary, CFG, effects, and both calls. Restore
# retail's equivalent schedule/private register web and bind the two decoded
# resident call identities to their pre-loader relocation carriers.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34CreateRecord.c.o: \
	config/normalizations/overlay34CreateRecord.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34CreateRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1F4 20b039915264e68f35cb5da6acf54206f545ed329e36e856dfe21c9b8052621a \
		@config/normalizations/overlay34CreateRecord.ops && \
	$(OBJCOPY) --redefine-sym func_80034448=overlay34LoadTextureReloc \
		--redefine-sym func_80029FE4=overlay34DirectionReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F4
include config/normalizations/overlay34Records.mk
include config/normalizations/overlay1Epoch12.mk
include config/normalizations/overlay12Epoch12.mk
include config/normalizations/overlay22Epoch12.mk
include config/normalizations/overlay28Epoch12.mk
include config/normalizations/overlay46Epoch12.mk
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41Ignore.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66GetCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66Select.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
# The natural body has the exact boundary, frame, opcode multiset, CFG, and
# runtime-relocation topology. This complete fail-loud ledger selects the
# retail instruction schedule, private register web, and three proved local
# symbol addends before trimming the compiler's section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66SmoothAndDraw.c.o: \
	config/normalizations/overlay66SmoothAndDraw.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66SmoothAndDraw.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4A0 62e3d491baf2c7018584dc73ad22560dbb6e41c7ff6c05d84a7d17b6e69ba7c8 \
		@config/normalizations/overlay66SmoothAndDraw.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4A0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61RecordSize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79SetLink.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79UpdateTimers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79FindNearby.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79FindNearby.c.o: CFLAGS += -Wab,-r4300_mul
# Natural output has the exact 77-instruction schedule, frame, stack/GPR web,
# and seven-role runtime topology. A complete guarded COP1 color web restores
# the private FP allocation; the loader-owned scale pair is filtered only from
# the five-call static surface, whose resident SYMBOL calls use the raw overlay
# base carrier exactly as the original split object does.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000000_18CCFA0.c.o: \
	config/normalizations/func_overlay_079_F0000000_18CCFA0.ops \
	config/normalizations/func_overlay_079_F0000000_18CCFA0.filter.spec \
	config/normalizations/func_overlay_079_F0000000_18CCFA0.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000000_18CCFA0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x134 4b4bf65bde54182aafa7b48c3f707b290170da7e7f82839626927ddc4b8451ef \
		@config/normalizations/func_overlay_079_F0000000_18CCFA0.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_079_F0000000_18CCFA0.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_079_F0000000_18CCFA0.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
# Natural output has the exact 123-instruction schedule, frame, stack/GPR web,
# CFG, opcodes, immediates, and runtime topology. Three complete private color
# webs restore the compiler's register allocation. The loader-owned resident
# flag pair is filtered from the original 13-record static split surface, then
# calls and the overlay-local counter pair are rebound to their raw carriers.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0001290_18CE230.c.o: \
	config/normalizations/func_overlay_079_F0001290_18CE230.ops \
	config/normalizations/func_overlay_079_F0001290_18CE230.filter.spec \
	config/normalizations/func_overlay_079_F0001290_18CE230.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0001290_18CE230.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym overlay79RandomReloc=func_overlay_079_F0000000_18CCFA0 \
		--redefine-sym overlay79FindNearby=func_overlay_079_F0000EFC_18CDE9C \
		--redefine-sym gOverlay79CounterReloc=D_14 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1EC 26d0ccb3172d36afd56cb89b480e627296a9ab1740d9e002d21b0410aad26516 \
		@config/normalizations/func_overlay_079_F0001290_18CE230.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_079_F0001290_18CE230.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_079_F0001290_18CE230.rebind.spec && \
	$(OBJCOPY) \
		--strip-symbol overlay79SpawnReloc \
		--strip-symbol overlay79EmitAtReloc \
		--strip-symbol overlay79FinishReloc \
		--strip-symbol overlay79EmitReloc \
		--strip-symbol overlay79TriggerReloc \
		--strip-symbol gOverlay79FlagsReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay80InitializeContact.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay80InitializeContact.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_080_F0000000_18CE8C8=overlay80InitializeContact $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay80UpdateContact.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay80UpdateContact.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_080_F000011C_18CE9E4=overlay80UpdateContact $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84CopyPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
# Natural codegen owns the exact CFG, opcode/register/relocation multiset, and
# FP behavior. A complete guarded frame/spill update and twelve-word
# post-call permutation selects the shipped private representation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84AdvanceCurrent.c.o: \
	config/normalizations/overlay84AdvanceCurrent.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84AdvanceCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x148 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x148 b8992c05cacd382e56d243defadb07bed2fedb81452769f09b928d38e3ca7060 \
		@config/normalizations/overlay84AdvanceCurrent.ops
# The natural object has the exact frame, CFG, opcode/call/FP schedule, and all
# memory effects. This complete decoded ledger selects retail's equivalent
# private allocator webs and one unused state-pointer spill slot.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84LoadCurrent.c.o: \
	config/normalizations/overlay84LoadCurrent.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84LoadCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x120 b7de8811d38658d16be513ab475b25409988a0f32c2e2220a34637bfc8e100f7 \
		@config/normalizations/overlay84LoadCurrent.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SetBit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetValues.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
# IDO naturally reproduces all registers, FP lanes, control flow, and calls.
# Assert the complete private-frame/selected-value representation before
# restoring the shipped word-sized home and 0x30-byte frame.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ActivateCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x194 87ac5da55e8b23ea2a9f42a737c478716c2b762b1f792a3423dc6b255198ed24 \
		fields:0x8:imm=65472@65488 \
		fields:0x18:op=12@0,rt=a2@zero,rd=zero@a2,sa=3@0,fn=63@37 \
		fields:0x80:imm=52@40 \
		fields:0x88:op=40@43,imm=39@48 \
		fields:0x8c:imm=52@40 \
		fields:0x94:op=36@35,imm=39@48 \
		fields:0x15c:imm=44@32 \
		fields:0x168:imm=44@32 \
		fields:0x188:imm=64@48 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ClearActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ClearMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SetAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84Mark.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SelectCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xAC
# The natural O86 tail owns the exact 662-word semantic program and all 32
# call anchors. Four closed one-to-one schedule basins and the complete private
# register/frame web select retail codegen. The compiler literal/jump pool is
# asserted against the retained module data, while the loader remains sole
# owner of its six LOCAL HILO roles. All 32 static calls are folded to the raw
# overlay carrier only after their runtime identities have been independently
# censused.
O86_0474_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/func_overlay_086_F0000474_18D22AC.c.o
$(O86_0474_OBJ): \
	config/normalizations/func_overlay_086_F0000474_18D22AC.ops \
	config/normalizations/func_overlay_086_F0000474_18D22AC.filter.spec \
	config/normalizations/func_overlay_086_F0000474_18D22AC.rodata.filter.spec \
	config/normalizations/func_overlay_086_F0000474_18D22AC.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(O86_0474_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xA58 f83fb494746bab470bd1a69cc6b3d86f68c72af9b4fa5a142733ef839220ed72 \
		@config/normalizations/func_overlay_086_F0000474_18D22AC.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_086_F0000474_18D22AC.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3dcccccd3d4ccccd000002380000035000000518000005180000068000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .rodata \
		@config/normalizations/func_overlay_086_F0000474_18D22AC.rodata.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_53d0=func_overlay_086_F0000000_18D1E38 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_086_F0000474_18D22AC.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ProcessCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ScaledVectorPosition.c.o: \
	config/normalizations/overlay86ScaledVectorPosition.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ScaledVectorPosition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xDC fa210ac085d64c5986bb98bd4a22030d1a5f5953672e4fcbe440ab8b0e2e92fa \
		@config/normalizations/overlay86ScaledVectorPosition.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86SelectPosition.c.o: \
	config/normalizations/overlay86SelectPosition.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86SelectPosition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x160 9dcc96e336d27ae20f88c0becf4b687fc10dfc667a5de04af7633faabd7ed111 \
		@config/normalizations/overlay86SelectPosition.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86BuildTransform.c.o: \
	config/normalizations/overlay86BuildTransform.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86BuildTransform.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x18C 22aabebb4b6ab40a8c6e4c72bf86c6cf4883c909732a1c3d9af50a50e6c08339 \
		@config/normalizations/overlay86BuildTransform.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94UpdateController.c.o: \
	config/normalizations/overlay94UpdateController.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
# The semantic object naturally owns the exact 275-word CFG, twelve calls,
# twelve private address pairs, FP behavior, and frame. This complete fail-loud
# decoded-field ledger selects only the retail private allocation/schedule web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94UpdateController.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x44C fc43722fe449701bbdc9961e9fdcf43537f4f48fd0f9e2c105f7a7a32f1b4e11 \
		@config/normalizations/overlay94UpdateController.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x108:func_8002A878:func_800254FC \
		0x14c:func_8002565C:func_800254FC \
		0x1c0:func_8002A878:func_800254FC \
		0x1f4:func_8002A878:func_800254FC \
		0x224:func_8002A878:func_800254FC \
		0x28c:func_8005ABA8:func_800254FC \
		0x29c:func_8005AF14:func_800254FC \
		0x2bc:func_80019AB8:func_800254FC \
		0x304:func_8002B040:func_800254FC \
		0x310:func_8002A910:func_800254FC && \
	$(OBJCOPY) --redefine-sym \
		func_800254FC=func_overlay_094_F0000000_18D6BA0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x18:5:gO94Value 0x1c:6:gO94Value \
		0x54:5:gO94Value 0x58:6:gO94Value \
		0xf8:5:gO94Const0 0x10c:6:gO94Const0 \
		0x16c:5:gO94Const4 0x1ac:6:gO94Const4 \
		0x1b0:5:gO94Const8 0x1b4:6:gO94Const8 \
		0x148:5:gO94ConstC 0x1f0:6:gO94ConstC \
		0x6c:5:gO94Const10 0x228:6:gO94Const10 \
		0x234:5:gO94Const14 0x238:6:gO94Const14 \
		0x35c:5:gO94Const18 0x364:6:gO94Const18 \
		0x368:5:gO94Const1C 0x380:6:gO94Const1C \
		0x398:5:gO94Const20 0x3b0:6:gO94Const20 \
		0x3e8:5:gO94Const24 0x400:6:gO94Const24 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94SetValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
# IDO leaves an unused eight-byte tail out of this frame and uses an `or` for
# the same zero argument. Assert that exact natural basin before restoring the
# shipped frame/spill offsets and equivalent zero materialization.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94InitializeController.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x54 848fb34e909dea1f93b35119efdd24005799b1673a94e41d740cc87be1375b53 \
		fields:0x0:imm=65456@65448 \
		fields:0x5c:op=0@9,rt=zero@a2,rd=a2@zero,fn=37@0 \
		fields:0x68:imm=68@76 \
		fields:0x78:imm=68@76 \
		fields:0x10c:imm=80@88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101AllocateEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Reset.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101FindEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry12.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ActivateSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101AdvanceSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte17.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte16.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateFloat12.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateDelta16.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x118
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateGlobalPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x108
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x198
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildIntensityColors.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
# The natural 79-word object has the exact frame, calls, stack layout, CFG,
# and rectangle values.  This guarded permutation selects retail's equivalent
# straight-line private-stack schedule and complete temporary register web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildBorder.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x13C d4f8551e10c9aeb38f4491cfbf28473b85553f0e98fa66ff33ce98f6355af73c \
		reorder:0x7c=0x80,0x80=0x7c,0x84=0x84,0x88=0x90,0x8c=0x94,0x90=0x88,0x94=0x8c,0x98=0x98,0x9c=0xac,0xa0=0xa0,0xa4=0x9c,0xa8=0xbc,0xac=0xa4,0xb0=0xa8,0xb4=0xb0,0xb8=0xb4,0xbc=0xb8,0xc0=0xc0,0xc4=0xc4,0xc8=0xc8,0xcc=0xcc,0xd0=0xd0,0xd4=0xd4,0xd8=0xd8,0xdc=0xdc,0xe0=0xe0,0xe4=0xe4,0xe8=0xe8,0xec=0xec,0xf0=0xf0,0xf4=0xf4,0xf8=0xf8,0xfc=0xfc,0x100=0x100,0x104=0x104,0x108=0x108,0x10c=0x10c,0x110=0x110,0x114=0x118,0x118=0x128,0x11c=0x11c,0x120=0x120,0x124=0x124,0x128=0x114 \
		fields:0x44:rt=t1@a2 \
		fields:0x4c:rt=t2@t1 \
		fields:0x74:rt=t1@a2 \
		fields:0x78:rt=t2@t1 \
		fields:0x7c:rt=t4@t3 \
		fields:0x84:rt=t3@t2 \
		fields:0x88:rs=t2@a2,rt=t9@t3,rd=v1@a0 \
		fields:0x8c:rt=t4@t9,rd=a1@a3 \
		fields:0x90:rt=t5@t4 \
		fields:0x94:rt=t6@t5 \
		fields:0x98:rt=t7@t6 \
		fields:0x9c:rt=t1@a2 \
		fields:0xa0:rs=t1@a2,rt=a2@a1 \
		fields:0xa4:rs=v1@a3,rt=a0@v1 \
		fields:0xa8:rt=t1@a2 \
		fields:0xac:rs=t2@t1,rt=a3@a2 \
		fields:0xb0:rs=a1@a0 \
		fields:0xb4:rt=t2@t1 \
		fields:0xb8:rt=a2@a1 \
		fields:0xbc:rt=a0@v1 \
		fields:0xc0:rt=a0@v1 \
		fields:0xc4:rt=a1@a0 \
		fields:0xc8:rt=v1@a3 \
		fields:0xd0:rt=a3@a2 \
		fields:0xd4:rt=a1@a0 \
		fields:0xd8:rt=a0@v1 \
		fields:0xdc:rt=a2@a1 \
		fields:0xe0:rt=t2@t1 \
		fields:0xe4:rt=a1@a0 \
		fields:0xe8:rt=a3@a2 \
		fields:0xf0:rt=t3@t2 \
		fields:0xf4:rt=t5@t4 \
		fields:0xf8:rt=t6@t5 \
		fields:0xfc:rt=t7@t6 \
		fields:0x100:rt=a2@a1 \
		fields:0x104:rt=a3@a2 \
		fields:0x10c:rt=a0@v1 \
		fields:0x110:rt=t8@t7 \
		fields:0x11c:rt=t8@t7 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C
# Natural codegen owns the exact 268-word opcode/function census, frame, CFG,
# calls, branch shape, semantic immediates, and relocation sites. This guarded
# complete permutation selects retail's equivalent private construction and
# register web; its only immediate changes are six proved stack-home aliases.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawPanel.c.o: \
	config/normalizations/overlay101DrawPanel.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawPanel.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawPanel.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x430 e1468b587569543eacaf50ccbf70ded9e88f16ba854aead53203ff52584d6cef \
		@config/normalizations/overlay101DrawPanel.ops
# The typed presentation owner naturally recovers the full semantic CFG,
# calls, record writes, conversion diamonds, and frame. A whole-owner guarded
# retained-address preparation, exact trim, and relocation-aware bijection
# select retail's equivalent private schedule/register web. Two final bounded
# filters and carrier-specific rebinds preserve all 40 static and 48 runtime
# relocation roles.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailAB4C.c.o: \
	config/normalizations/overlay101TailAB4C.prepare.py \
	config/normalizations/overlay101TailAB4C.retained.filter.spec \
	config/normalizations/overlay101TailAB4C.ops \
	config/normalizations/overlay101TailAB4C.addends.ops \
	config/normalizations/overlay101TailAB4C.filter.spec \
	config/normalizations/overlay101TailAB4C.calls.spec \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailAB4C.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailAB4C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailAB4C.retained.filter.spec && \
	$(HOST_PYTHON) config/normalizations/overlay101TailAB4C.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9F8 \
		0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x9F8 7ffdc0e790371ae00d38dee6061af7713aeffa092ee6bc6c7c27a7ba15d472e2 \
		@config/normalizations/overlay101TailAB4C.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x9F8 9603f7a7d2335516a72296d87d51432d8e7f1a5d3eb9f15e723d1883b8c7c5eb \
		@config/normalizations/overlay101TailAB4C.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailAB4C.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gO101TailAB4CRoot=D_1C \
		--redefine-sym gO101TailAB4COrderCount=D_1C4 \
		--redefine-sym gO101TailAB4CAssetD90=D_D90 \
		--redefine-sym gO101TailAB4COrderSlots=D_38 \
		--redefine-sym gO101TailAB4CNode32Count=D_1CC \
		--redefine-sym gO101TailAB4CNodes32=D_340 \
		--redefine-sym gO101TailAB4CNode24Count=D_1D0 \
		--redefine-sym gO101TailAB4CNodes24=D_540 \
		--redefine-sym gO101TailAB4CFinalObject3F14=D_3F14 \
		--redefine-sym o101TailAB4CCreator1Reloc=func_overlay_101_F0000000_18DB820 \
		--redefine-sym overlay101ByteLength=func_overlay_101_F000CEA8_18E86C8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailAB4C.calls.spec
# The typed builder naturally owns every data/call role and the full semantic
# opcode inventory. Remove only IDO's asserted volatile-FPR home pair, then
# select the complete equivalent private schedule/register web and restore the
# three proved raw local addends. Every stage is guarded by a full-text digest.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailB544.c.o: \
	config/normalizations/overlay101TailB544.prepare.py \
	config/normalizations/overlay101TailB544.ops \
	config/normalizations/overlay101TailB544.filter.spec \
	config/normalizations/overlay101TailB544.addends.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailB544.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailB544.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay101TailB544.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4F0 faf19203d1594c3045b22e6a924f58e2ff0abe1ca0b866fd44006ea437bf0793 \
		@config/normalizations/overlay101TailB544.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailB544.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4F0 ab9be9ec9253c7454f7b0e0bae0864bfacb629b01d2e3cf8fc05ba802e9464fe \
		@config/normalizations/overlay101TailB544.addends.ops
# The typed presentation builder naturally owns all 45 runtime roles and the
# complete semantic instruction census. A guarded whole-owner preparation
# canonicalizes the two equivalent 1.0f webs, then a complete relocation-aware
# bijection selects retail's private schedule/register web. Exact local
# addends, split-object relocation filtering, carrier renames, and call/root
# rebinding finish the configured 452-word owner.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailBA34.c.o: \
	config/normalizations/overlay101TailBA34.prepare.py \
	config/normalizations/overlay101TailBA34.ops \
	config/normalizations/overlay101TailBA34.addends.ops \
	config/normalizations/overlay101TailBA34.filter.spec \
	config/normalizations/overlay101TailBA34.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailBA34.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailBA34.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay101TailBA34.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x710 7cfe98da170547834a5c11d936a7add83d950a9c32d0a9482c5ad1db34d9a76b \
		@config/normalizations/overlay101TailBA34.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x710 7d800dd16ef8cd7deb12cbcc76e13bdb87bad265e62433a525a69a6491e0aa84 \
		@config/normalizations/overlay101TailBA34.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailBA34.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gO101TailBA34Root=D_1C \
		--redefine-sym gO101TailBA34OrderSlots=D_38 \
		--redefine-sym gO101TailBA34Inputs=D_54 \
		--redefine-sym gO101TailBA34OrderCountCall0=D_1C4 \
		--redefine-sym gO101TailBA34AssetDB8=D_DB8 \
		--redefine-sym gO101TailBA34Node32Count=D_1CC \
		--redefine-sym gO101TailBA34Nodes32=D_340 \
		--redefine-sym gO101TailBA34Node24Count=D_1D0 \
		--redefine-sym gO101TailBA34Nodes24=D_540 \
		--redefine-sym gO101TailBA34FinalObject4630=D_4630 \
		--redefine-sym o101TailBA34RootCreatorReloc=func_overlay_101_F0000000_18DB820 \
		--redefine-sym overlay101ByteLength=func_overlay_101_F000CEA8_18E86C8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailBA34.calls.spec
# The typed presentation builder naturally owns the complete CFG/opcode
# inventory and all 39 runtime roles. A whole-owner guard rewrites two proved
# constant representation webs; the complete relocation-aware bijection then
# selects the shipped private schedule/register web before exact local addends,
# split-object relocation filtering, carrier renames, and call rebinding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC144.c.o: \
	config/normalizations/overlay101TailC144.prepare.py \
	config/normalizations/overlay101TailC144.ops \
	config/normalizations/overlay101TailC144.addends.ops \
	config/normalizations/overlay101TailC144.filter.spec \
	config/normalizations/overlay101TailC144.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC144.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC144.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay101TailC144.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x5A4 f7cbc1ea07209488de35fa5d835360275731d8360ac653b88ff224bb42af14cc \
		@config/normalizations/overlay101TailC144.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x5A4 079927f6382905edc140409c084f41df23cc98e76bb5adbdea9cab5e07a5c4cf \
		@config/normalizations/overlay101TailC144.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailC144.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gO101TailC144Root=D_1C \
		--redefine-sym gO101TailC144OrderSlots=D_38 \
		--redefine-sym gO101TailC144OrderCount=D_1C4 \
		--redefine-sym gO101TailC144AssetDCC=D_DCC \
		--redefine-sym gO101TailC144Node32Count=D_1CC \
		--redefine-sym gO101TailC144Nodes32=D_340 \
		--redefine-sym gO101TailC144Node24Count=D_1D0 \
		--redefine-sym gO101TailC144Nodes24=D_540 \
		--redefine-sym gO101TailC144FinalObject4A90=D_4A90 \
		--redefine-sym o101TailC144CreatorReloc=func_overlay_101_F0000000_18DB820 \
		--redefine-sym overlay101ByteLength=func_overlay_101_F000CEA8_18E86C8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailC144.calls.spec
# The typed dispatcher naturally preserves the complete call/data graph and
# opcode census. Remove its sole asserted scheduler NOP, select the complete
# bijective private schedule/register web, restore nine shipped local addends,
# and retain only the split object's exact static relocation surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC6E8.c.o: \
	config/normalizations/overlay101TailC6E8.prepare.py \
	config/normalizations/overlay101TailC6E8.ops \
	config/normalizations/overlay101TailC6E8.addends.ops \
	config/normalizations/overlay101TailC6E8.filter.spec \
	config/normalizations/overlay101TailC6E8.calls.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC6E8.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC6E8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay101TailC6E8.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4F4 05428d5a536202cc290237ff6d6f6e4b61f3d48e1639d86d11f6dd234fb60a3e \
		@config/normalizations/overlay101TailC6E8.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4F4 d4e289e6eb62ddb1f6994968b046b4e204b3a6e0d87b67894dc4db0a00b6c6b2 \
		@config/normalizations/overlay101TailC6E8.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailC6E8.filter.spec && \
	$(OBJCOPY) --remove-section=.rodata \
		--redefine-sym o101TailC6E8StateReloc=func_overlay_101_F0000000_18DB820 \
		--redefine-sym gO101TailC6E8OrderCount=D_1C4 \
		--redefine-sym gO101TailC6E8AssetDF0=D_DF0 \
		--redefine-sym gO101TailC6E8Node32Count=D_1CC \
		--redefine-sym gO101TailC6E8Nodes32=D_340 \
		--redefine-sym gO101TailC6E8Nodes20=D_200 \
		--redefine-sym gO101TailC6E8Node20Count=D_1C8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay101TailC6E8.calls.spec
# The natural clock renderer is exact in size, frame, CFG, calls, FP topology,
# and effects. Restore retail's equivalent ordering of two independent loop
# constants, three complete four-iteration count webs, and one commutative FP
# expression. The digest covers the entire owned function before alignment is
# trimmed, so any unreviewed compiler drift fails loudly.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawClock.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3b8 888f343870fd7de8df68995b60de561d6f5eb19b0c97b73226a0c52266721b7f \
		reorder:0x208=0x20c,0x20c=0x208 \
		fields:0x250:imm=0x21@0x22 \
		fields:0x254:rt=v0@v1 \
		fields:0x294:imm=0x10@0x11 \
		fields:0x298:rt=v0@v1 \
		fields:0x2d4:op=9@0,rs=s6@zero,rt=v0@zero,imm=4@0 \
		fields:0x2d8:op=0@9,rs=v0@zero,rt=zero@v1,imm=0x1825@4 \
		fields:0x2dc:rs=v0@v1 \
		fields:0x2e0:rs=v0@zero,imm=0xffff@3 \
		fields:0x324:rt=20@10,rd=10@20 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildFrame.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SetScissor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x198
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Cleanup.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x158
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdatePresentation.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x174
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateChains.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawSlots.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SchedulePair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SchedulePair12.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleByte17.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleByte16.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21RegisterPlane.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21RegisterPlane.c.o: CFLAGS += -Wab,-r4300_mul
# The natural source is exact except that IDO gives the three reloads of the
# overlay-local object count to v1 instead of the shipped a0. Assert the full
# bounded natural register web before restoring that interchangeable color.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21ApplyPriorities.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1c8 12ac460cb47e48fb75a6388707fa9ee1dc13fc569c444a66d563f406fbf5a285 \
		fields:0x24:rt=v1@a0 \
		fields:0x28:rs=v1@a0,rt=v1@a0 \
		fields:0x34:rs=v1@a0 \
		fields:0x58:rt=v1@a0 \
		fields:0x114:rt=v1@a0 \
		fields:0x11c:rs=v1@a0,rt=v1@a0 \
		fields:0x128:rt=v1@a0 \
		fields:0x19c:rt=v1@a0 \
		fields:0x1a0:rs=v1@a0,rt=v1@a0 \
		fields:0x1a4:rt=v1@a0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21ApplyPriorities.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o030/overlay30Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o030/overlay30TransposePixels.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_030_F00002B4_187F1AC=overlay30TransposePixels $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23SpawnAttachments.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x208
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x148
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Init.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x118
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Update.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23RenderEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x100 11925133075ef610a8d394c118dd65ee1a7e49c28534a66710005f27f4a9af92 \
		fields:0x18:rt=t0@t1 \
		fields:0x28:rt=t0@t1 \
		fields:0x2c:rt=t0@t1 \
		fields:0x40:rs=t0@t1 \
		fields:0x50:rs=t0@t1 \
		fields:0x54:rt=t3@t4 \
		fields:0x58:rt=t5@t7 \
		fields:0x60:rs=t0@t1 \
		fields:0x74:rs=zero@sp,imm=14@64 \
		fields:0x78:op=9@0,rs=sp@s1,rt=t8@zero,rd=zero@a0,sa=1@0,fn=0@37 \
		fields:0x84:rs=s1@s0,rd=a0@a3 \
		fields:0x88:op=0@57,rs=s0@sp,rt=zero@t2,rd=a3@zero,sa=0@1,fn=37@20 \
		fields:0x8c:op=57@49,rs=sp@s0,rt=t2@s0,imm=84@20 \
		fields:0x90:op=49@57,rs=s0@sp,imm=20@88 \
		fields:0x94:op=57@35,rs=sp@t1,rt=s0@t2,imm=88@32 \
		fields:0x98:op=35@43,rs=t0@sp,rt=t1@t2,imm=32@96 \
		fields:0x9c:op=43@35,rs=sp@s1,rt=t1@v0,imm=96@0 \
		fields:0xa0:op=35@9,rs=s1@zero,rt=v1@t2,imm=0@14 \
		fields:0xa4:rs=v1@v0,rt=t2@t3 \
		fields:0xa8:rt=t2@t3 \
		fields:0xac:rs=v1@v0 \
		fields:0xb0:rs=v1@v0,rt=t3@t4 \
		fields:0xb4:rt=v1@v0 \
		fields:0xb8:rs=v1@v0,rt=t4@t5 \
		fields:0xbc:rt=t4@t5 \
		fields:0xc0:rs=v1@v0,rt=t5@t7 \
		fields:0xc4:rs=v1@v0 \
		fields:0xc8:rt=t7@t8 \
		fields:0xcc:rt=t9@t2 \
		fields:0xd0:rt=t8@t9 \
		fields:0xd4:rt=t7@t8 \
		fields:0xd8:rt=t1@t3 \
		fields:0xe8:rt=t1@t3 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x100
$(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay24Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay24Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x268 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3e99999a000000000000000000000000
$(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay24RenderState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x190
# The typed initializer naturally owns 94 instructions plus one reusable IDO
# alignment word, with exact frame, five calls, CFG, and opcode/FP inventories.
# Adopt that proved nop and select the complete private schedule/register/home
# web without changing an opcode, constant, call identity, or memory effect.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25InitializeEffect.c.o: \
	config/normalizations/overlay25InitializeEffect.ops \
	$(TOOLS_DIR)/extend_elf_function_to_text.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25InitializeEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x17C && \
	$(HOST_PYTHON) $(TOOLS_DIR)/extend_elf_function_to_text.py $@ \
		overlay25InitializeEffect 0x178 0x17C \
		7291fa6c89c8261645103d5b3d2a880b1212771fe84e454a21293e167123dfd3 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x17C ef8e14dd36a3e857a4551303f7f8828580252e767f5ac1d25c5140499d3495a8 \
		@config/normalizations/overlay25InitializeEffect.ops
# The reconstructed update owns +0x17C..+0x588. IDO naturally recovers the
# exact operation inventory, call order, relocation surface, and six-word
# literal pool; the guarded ledger selects the shipped schedule/register/home
# web. The original overlay data at +0x630 remains authoritative, so prove and
# externalize the compiler's identical private pool before linking.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25UpdateEffect.c.o: \
	config/normalizations/overlay25UpdateEffect.ops \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25UpdateEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x40C 20c0b987007f00daa1b622e49087021cc827ca9ba10f0dd0ce68bb4b9d5b1a22 \
		@config/normalizations/overlay25UpdateEffect.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3f8d3dcb3f8d3dcb3ecccccd3ecccccd3dcccccd3ecccccd0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25SetVectorFlags.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27RenderEffect.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27RenderEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3F8
# NON_MATCHING/GLOBAL_ASM uses extracted retail instructions; keep only
# the metadata-only entry-symbol rename needed by the friendly split.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27UpdateCoordinates.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27UpdateCoordinates.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_027_F0000A1C_187C3F4=overlay27UpdateCoordinates $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x104
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56SplitTime.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56UnpackColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56AdjustCoordinates.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56SetMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56LoadResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56ReleaseResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay_039_tail.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay_039_tail.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay39Write.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay39Write.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x114 a2e8758f60d85c2c2ea927f5ead44995d71d903daaf0a97a5b997417fe4c12c8 \
		fields:0x8:op=0@43,rs=a0@sp,rt=zero@a1,rd=a2@zero,fn=37@44 \
		fields:0xc:op=43@35,rs=sp@a0,rt=a1@v0,imm=32@100 \
		fields:0x10:op=35@0,rs=a2@a0,rt=v0@zero,rd=zero@a2,sa=1@0,fn=36@37 \
		fields:0x20:rt=v1@a1 \
		fields:0x2c:rs=v1@a1 \
		fields:0x38:rs=a2@a0 \
		fields:0x3c:rs=v1@a1 \
		fields:0x44:rs=a2@a0 \
		fields:0x48:rs=v1@a1 \
		fields:0x50:rs=a2@a0 \
		fields:0x58:rs=a2@a0 \
		fields:0x5c:imm=32@44 \
		fields:0xac:rt=v1@a1,imm=24@28 \
		fields:0xd4:rt=v1@a1,imm=24@28 \
		fields:0xd8:rt=16@0,rd=0@16 \
		fields:0xec:rs=v1@a1 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114
# The typed reconstruction naturally owns 852 bytes plus one proved zero
# alignment word. Extend that word into the symbol, select the complete guarded
# frame/register/FP/schedule bijection, and bind resident calls to the overlay's
# stored-zero runtime proxy without collapsing the relocation sites.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Render.c.o: \
	config/normalizations/overlay37Render.ops \
	$(TOOLS_DIR)/extend_elf_function_to_text.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Render.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x358 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/extend_elf_function_to_text.py $@ \
		overlay37RenderEffect 0x354 0x358 \
		dbe9cb3411865c02c06efcae1dedd18f0b5a6c96ac37781ca9d292cfc1ac9412 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x358 812e309eb30b88554676a6b7a11b8cd823a7426d7666506654bef7db8b6119e5 \
		@config/normalizations/overlay37Render.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x01c:func_80021964:overlay37CallProxy \
		0x174:func_8002A250:overlay37CallProxy \
		0x248:func_800244EC:overlay37CallProxy \
		0x264:func_800349A4:overlay37CallProxy \
		0x33c:func_8002460C:overlay37CallProxy
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37RecordMinimum.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37RecordActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40AddEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x84 6f729465079278e0246a47c16c2e976f8d8e43a7ee89b5d300f687c8429f9840 \
		fields:0x20:rt=v1@a0 \
		fields:0x2c:rs=v1@a0,rd=a0@v1 \
		fields:0x74:rs=v1@a0 \
		fields:0x78:rs=v1@a0,rt=v1@a0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40DrawEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x164
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40RemoveEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40UpdateEntries.c.o: \
	config/normalizations/overlay40UpdateEntries.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40UpdateEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xC0 ab399f8641247fec884ebe3e17c73682057bb72c5d97b9734e6578accbfabd85 \
		@config/normalizations/overlay40UpdateEntries.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40BuildFrame.c.o: \
	config/normalizations/overlay40BuildFrame.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40BuildFrame.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x144 41cc334b35d88d7dc63ee3e2d0417af667013bd681ef4e30fcc5237723c79797 \
		@config/normalizations/overlay40BuildFrame.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x144
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40SetValues.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40Interpolate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40DrawTintRectangle.c.o: \
	config/normalizations/overlay40DrawTintRectangle.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40DrawTintRectangle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x15C 96fbd4fed92061b2674a29f910508d591437813ddc81cf5b0408aad296f23a58 \
		@config/normalizations/overlay40DrawTintRectangle.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
# The natural source owns the full color-fade algorithm and relocation roles.
# A guarded preparation reconstructs the retained saved-s0/countdown topology;
# the complete bijective ledger then selects the shipped private schedule.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40FadeRecords.c.o: \
	config/normalizations/overlay40FadeRecords.prepare.py \
	config/normalizations/overlay40FadeRecords.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40FadeRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay40FadeRecords.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x194 589bdc87fe62280529ee5a646c33b324b68ad9bbbf435e5c413b55951377106d \
		@config/normalizations/overlay40FadeRecords.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
$(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Resume.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42DrawCapturedBuffer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5B0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Present.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
# The source recovers the full offset-zero initializer. The guarded private
# register/schedule web selects retail's a2 reservation; the LOCAL D_0 records
# are loader-owned and the two adjacent O43 calls retain zero-addend symbols.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43InitializeState.c.o: \
	config/normalizations/overlay43InitializeState.ops \
	config/normalizations/overlay43InitializeState.filter.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43InitializeState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x194 745acc6bd0934b19a225a4092c744798a0c10ca982ce9b56eb04dbb58aaa950a \
		@config/normalizations/overlay43InitializeState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay43InitializeState.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
# Three LOCAL D_C8 accesses carry proved loader addends, while the runtime
# relocation table owns the discarded LOCAL/external HILO rows. Fold the two
# runtime resident calls through the offset-zero overlay carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FlushPending.c.o: \
	config/normalizations/overlay43FlushPending.ops \
	config/normalizations/overlay43FlushPending.filter.spec \
	config/normalizations/overlay43FlushPending.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FlushPending.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xF0 0c61a242beec3c2e7cbc2e4b493aa7c6ecba24987d71aa72d3b0fe7e2cdc90f8 \
		@config/normalizations/overlay43FlushPending.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay43FlushPending.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		func_8002E800=func_overlay_043_F0000000_1889FD0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay43FlushPending.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
# Six resident release roles are stored through one overlay carrier. The source
# is otherwise instruction-natural; rebind the exceptional release site and
# trim only compiler section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ReleaseResources.c.o: \
	config/normalizations/overlay43ReleaseResources.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ReleaseResources.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_8002B768=func_overlay_043_F0000000_1889FD0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay43ReleaseResources.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
# The natural function owns the exact motion calculation, frame, and CFG.
# Select the complete post-call FP schedule web, discard runtime-local D_24
# HILO rows, and fold the two resident calls through one stored carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ComputeMotion.c.o: \
	config/normalizations/overlay43ComputeMotion.ops \
	config/normalizations/overlay43ComputeMotion.filter.spec \
	config/normalizations/overlay43ComputeMotion.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ComputeMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xDC d9b2924ca4e62c12e8ab2a98837793da411b4684545155207893df3e87487a2d \
		@config/normalizations/overlay43ComputeMotion.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay43ComputeMotion.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		func_80029FE4=func_overlay_043_F0000000_1889FD0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay43ComputeMotion.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
# Retail reserves one extra ABI-aligned frame block. Assert that complete
# frame/home web, then fold the allocation and release runtime roles through
# the offset-zero overlay carrier without changing their live identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43AllocateResources.c.o: \
	config/normalizations/overlay43AllocateResources.ops \
	config/normalizations/overlay43AllocateResources.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43AllocateResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xE0 a803b95baeebe1d064796b269bfd606d4984e81f340586bec3f3b648a0528401 \
		@config/normalizations/overlay43AllocateResources.ops && \
	$(OBJCOPY) --redefine-sym \
		func_8002B280=func_overlay_043_F0000000_1889FD0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay43AllocateResources.rebind.spec
# IDO keeps the pre-gate owner in s2 while retail uses s1, whose later loop
# role begins only after the owner's final use. Assert the complete natural
# four-word owner web before restoring the shipped callee-saved selection.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43SubmitChildren.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x114 4470296356841da3d6a4a061a2abc03308bf35e9a3843ceba51154000a6a7b61 \
		fields:0x2c:rd=s2@s1 \
		fields:0x58:rs=s2@s1 \
		fields:0x68:rs=s2@s1 \
		fields:0x78:rs=s2@s1 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114
# The first two byte loads form the same sum either way; IDO's alternative
# coloring changes four encodings. Assert the natural output before restoring
# the shipped temporary registers.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FilterImage.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xac df46879804000cafbaa350f23c95efb44ecae6c3ce95923f8a1084853012490c \
		fields:0xc:rt=t6@a3 \
		fields:0x10:rt=t7@t6 \
		fields:0x20:rs=t6@a3,rt=t7@t6,rd=v1@t7 \
		fields:0x28:rs=t8@t7,rt=v1@t8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xAC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44CreateAnimationState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x224
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44ReleaseHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
# The typed four-slot frame-cache updater recovers the complete operation,
# CFG, loop, and relocation surfaces. Adopt one proved IDO padding nop and
# select the shipped complete schedule/register/stack web fail-loud.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44UpdateFrameCache.c.o: \
	config/normalizations/overlay44UpdateFrameCache.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44UpdateFrameCache.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2EC ed3e74a94e2745246ca218268c11e61dadc1fe5312d9a676c18fab7aa0293753 \
		@config/normalizations/overlay44UpdateFrameCache.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
# The source naturally owns the exact frame, schedule, and 73 instruction
# words. The shipped overlay table retains four distinct runtime callees; the
# split target normalizes those sites to its offset-zero proxy, so fold the
# role names through one carrier and preserve that established proxy binding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69UpdateAnchor.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69UpdateAnchor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x80:overlay69AngleReloc:overlay69RotateVectorReloc \
		0x94:overlay69SinReloc:overlay69RotateVectorReloc \
		0xc0:overlay69CosReloc:overlay69RotateVectorReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay69RotateVectorReloc=func_overlay_069_F0000000_18C8A68 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124
# Overlay 69 and overlay 88 ship the same reviewed renderer bytes, but each
# remains an independent object verdict. This rule asserts overlay 69's entire
# bounded natural basin before restoring the shipped stack-frame and complete
# allocator/scheduling webs; the adjacent alignment word remains separate.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69DrawSortedGeometry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x59c 3a5cfd50d01656368d68dca0dbb82437bb3bb11e8c14fa6856bf6b8c6ffa312c \
		@config/normalizations/overlay69DrawSortedGeometry.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x59C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o067/overlay67BuildVertices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/overlay71UpdateCoordinates.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
# The natural object owns 0x278 executable bytes and IDO adds two zero
# alignment words. Assert the complete compiler output before trimming only
# that outside-owner alignment; the two scale aliases are one runtime LOCAL
# object with shipped addends zero and four.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000000_18C9B20.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000000_18C9B20.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x280 e3a550e9d51770a0857a3243267f9097f00e971c8cf3bb60b2fd6b5c05caf42e \
		fields:0x278:op=0@0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x25c:func_overlay_071_F0000278_18C9D98:func_overlay_071_F0000000_18C9B20 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x278
# Natural IDO output owns the exact 332-instruction CFG, frame, call/FP
# topology, and relocation sites. Select the shipped complete private stack
# home and two independent schedule permutations, then fold the sqrtf call
# through O71's established stored-zero resident-call proxy.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000278_18C9D98.c.o: \
	config/normalizations/func_overlay_071_F0000278_18C9D98.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000278_18C9D98.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000278_18C9D98.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x530 f3cde4c0acc455d724f786b7f5b38f93770d7ada6f33168b7f7a91010115b2a2 \
		@config/normalizations/func_overlay_071_F0000278_18C9D98.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x1b8:sqrtf:func_80032BF0 && \
	$(OBJCOPY) --redefine-sym \
		func_80032BF0=func_overlay_071_F0000000_18C9B20 $@
# Natural IDO output owns the exact 182-instruction CFG, frame, calls,
# branch-likely behavior, memory effects, and relocation sites. Select the
# retail private spill/register/schedule web, then fold five resident-call
# roles through the overlay's stored-zero relocation proxy.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000870_18CA390.c.o: \
	config/normalizations/func_overlay_071_F0000870_18CA390.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000870_18CA390.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000870_18CA390.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2D8 ade7ab0f6288f866c805c58967c5e325a65666e6c4ba176da36604e958b0c2ef \
		@config/normalizations/func_overlay_071_F0000870_18CA390.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x054:func_8002409C:func_80032BF0 \
		0x0b0:func_80034554:func_80032BF0 \
		0x198:func_80034554:func_80032BF0 \
		0x2bc:func_800241BC:func_80032BF0 && \
	$(OBJCOPY) --redefine-sym \
		func_80032BF0=func_overlay_071_F0000000_18C9B20 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2D8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o072/overlay72Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o072/overlay72Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB4
# The source naturally owns all operations and exact instruction order. Select
# the complete private GPR/local-base web, bind the proved +4/+8 LOCAL addends
# to the module's D_0 carrier, and enforce the exact owner boundary.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay73Initialize.c.o: \
	config/normalizations/overlay73Initialize.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay73Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x190 180556a4efc4527dcec3b3b101b09adab1e0b51ea8abad7994e632d6b449ca01 \
		@config/normalizations/overlay73Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x14c:D_4:D_0 0x160:D_4:D_0 \
		0x164:D_8:D_0 0x170:D_8:D_0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x190
# The source recovers the complete renderer and its three calls. Apply the
# audited private allocator/home/schedule web and trim the compiler's two
# alignment NOPs so +EA8..+EB0 remains separately owned target padding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay73Draw.c.o: \
	config/normalizations/overlay73Draw.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay73Draw.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x138 84df0d134f6a8ec3653dca3618e6f784b286a59b9ad207510d10dedd0fdda26f \
		@config/normalizations/overlay73Draw.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x090:func_80034554:func_8002409C \
		0x120:func_800241BC:func_8002409C && \
	$(OBJCOPY) --redefine-sym \
		func_8002409C=func_overlay_073_F0000000_18CAAC0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
$(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay74Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay74Init.c.o: CFLAGS += -Wab,-r4300_mul
# The natural C reproduces the complete routine and schedule, but IDO colors
# two non-overlapping temporary webs oppositely. Every replacement below is a
# register-only or commutative-operand encoding; fail if compiler output moves.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay74Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x190 2ae806f5065442817457af295f249b42c9659f5952d52d20e38b2eb87511b58d \
		fields:0xc:rt=v0@t3 \
		fields:0x68:rt=v0@t3 \
		fields:0x84:rt=t3@v0 \
		fields:0x8c:rs=t3@v0 \
		fields:0x90:rs=t3@v0 \
		fields:0x124:rs=t0@t8,rt=t8@t0 \
		fields:0x13c:rt=v0@v1 \
		fields:0x140:rs=v0@v1,rt=v0@v1 \
		fields:0x148:rt=v1@v0 \
		fields:0x14c:rt=v0@v1 \
		fields:0x150:rd=v0@v1 \
		fields:0x154:rs=v0@v1,rt=v1@v0 \
		fields:0x15c:rt=v1@v0 \
		fields:0x168:rd=v1@v0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x190
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75MarkSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x214
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75Init.c.o: CFLAGS += -Wab,-r4300_mul
# The semantic source naturally owns the exact 304-instruction boundary,
# frame, CFG, 16 calls, and FP expression tree.  This guarded ledger selects
# retail's equivalent private schedule/local-home web; its sole opcode change
# replaces a redundant unescaped-stack initialization with the stable active
# halfword reload already governing the same join.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75UpdateMovingObject.c.o: \
	config/normalizations/overlay75UpdateMovingObject.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75UpdateMovingObject.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75UpdateMovingObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4C0 dd4a4fa928227da6a8df5e2b7a6af736a328a95a4c0d85cb493557fb09531768 \
		@config/normalizations/overlay75UpdateMovingObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay_077_tail.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay77Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay77Init.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay77Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x288
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay77Update.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay_081_leafs.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xCC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x154
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81Update.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81CheckNearby.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81CheckNearby.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay82Accessors.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay82Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay82Init.c.o: CFLAGS += -Wo,-loopunroll,2
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay82Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x458
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
# Source naturally reproduces every instruction word and the seven retail
# runtime relocation sites. The split target aliases those sites to its
# offset-zero static proxy, so fold only the role names and trim alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88UpdateAnchor.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88UpdateAnchor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x60:overlay88UpdateNodeReloc:overlay88PrepareNodeReloc \
		0x80:overlay88RefreshNodeReloc:overlay88PrepareNodeReloc \
		0xc4:overlay88ForwardVectorReloc:overlay88PrepareNodeReloc \
		0xd0:overlay88AngleReloc:overlay88PrepareNodeReloc \
		0xec:overlay88SinReloc:overlay88PrepareNodeReloc \
		0x118:overlay88CosReloc:overlay88PrepareNodeReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay88PrepareNodeReloc=func_overlay_088_F0000000_18D3A88 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x158
# The o69/o88 retail renderer bodies and independently compiled natural basins
# are byte-identical.  Assert o88's own configured object and target digest;
# its predecessor remains assembly-owned and there is no following padding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88DrawSortedGeometry.c.o: \
	$(SRC_DIR)/overlays/o069/overlay69DrawSortedGeometry.c
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88DrawSortedGeometry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x59c 3a5cfd50d01656368d68dca0dbb82437bb3bb11e8c14fa6856bf6b8c6ffa312c \
		@config/normalizations/overlay69DrawSortedGeometry.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x59C
# The semantic source naturally reproduces the exact boundary, frame, opcode
# inventory, CFG, FP topology, and runtime relocation sites. Select retail's
# complete local-allocation web, fold the six statically aliased call sites to
# the offset-zero proxy, and remove only the local-data relocations retained by
# the shipped overlay runtime table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateEffect.c.o: \
	config/normalizations/overlay89UpdateEffect.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateEffect.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x138 1b44a19d916bbad634607e5fa7d2d2b6266b5607c1c4f0234669e29b5af0145a \
		@config/normalizations/overlay89UpdateEffect.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x30:overlay89TrigBReloc:overlay89TrigAReloc \
		0x48:overlay89TrigBReloc:overlay89TrigAReloc \
		0xb8:overlay89MoveEffectReloc:overlay89TrigAReloc \
		0x118:overlay89CreateEffectReloc:overlay89TrigAReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay89TrigAReloc=func_overlay_089_F0000000_18D4230 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x54:5:gOverlay89EffectScale \
		0x58:6:gOverlay89EffectScale && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89Evaluate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
# Natural source supplies the exact boundary, frame, CFG, calls, and five
# relocation-bearing instructions. The complete guarded allocation/schedule
# ledger selects retail's equivalent state-pointer spill and register web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89InitializeEffect.c.o: \
	config/normalizations/overlay89InitializeEffect.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89InitializeEffect.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89InitializeEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x334 91a336e39261e09b3690e088760c6bdb0bf39854393b7d119442b9db64239a70 \
		@config/normalizations/overlay89InitializeEffect.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x288:overlay89MaintainReloc:overlay89CreatePrimaryReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay89Evaluate=func_overlay_089_F0000138_18D4368 \
		--redefine-sym overlay89CreatePrimaryReloc=func_overlay_089_F0000000_18D4230 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x50:5:gOverlay89InitScale \
		0x54:6:gOverlay89InitScale && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x334
# Natural source supplies the exact 136-instruction semantic body, calls, FP
# inventory, constants, structure accesses, and fourteen-record runtime
# relocation surface. The complete reviewed permutation/field ledger selects
# retail's parameter-spill, frame, saved-register, and schedule web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateStateAndParticles.c.o: \
	config/normalizations/overlay89UpdateStateAndParticles.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateStateAndParticles.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateStateAndParticles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x220 d4ba88922a47913a7f6f139165be1d0b9351e60de4f60832af5366177082a6df \
		@config/normalizations/overlay89UpdateStateAndParticles.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xa8:overlay89SetColorReloc:overlay89SetPrimaryReloc \
		0x12c:overlay89RandomReloc:overlay89SetPrimaryReloc \
		0x144:overlay89RandomReloc:overlay89SetPrimaryReloc \
		0x15c:overlay89RandomReloc:overlay89SetPrimaryReloc \
		0x178:overlay89RandomReloc:overlay89SetPrimaryReloc \
		0x190:overlay34SpawnReloc:overlay89SetPrimaryReloc \
		0x1ac:overlay89RandomReloc:overlay89SetPrimaryReloc \
		0x1e0:overlay89SetColorReloc:overlay89SetPrimaryReloc \
		0x1f8:overlay89MaintainReloc:overlay89SetPrimaryReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay89UpdateReloc=func_overlay_089_F00001A8_18D43D8 \
		--redefine-sym overlay89SetPrimaryReloc=func_overlay_089_F0000000_18D4230 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x120:5:gOverlay89RandomScale \
		0x124:6:gOverlay89RandomScale
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay92Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
# Natural source owns the exact semantic topology, frame and arithmetic. Four
# bounded scheduler permutations and the complete reviewed private allocation
# web select retail's equivalent schedule; local float pairs are materialized
# while all three calls retain the raw stored overlay carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay92FindNearestCourse.c.o: \
	config/normalizations/overlay92FindNearestCourse.ops \
	config/normalizations/overlay92FindNearestCourse.rebind.spec \
	config/normalizations/overlay92FindNearestCourse.filter.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay92FindNearestCourse.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2A0 942ca05cebcedd2066b98a19667b50c23cf044c4a79d28e0a67e61e85e798d42 \
		@config/normalizations/overlay92FindNearestCourse.ops && \
	$(OBJCOPY) --redefine-sym \
		overlay92GetObjectRange=func_overlay_092_F0000000_18D5F20 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay92FindNearestCourse.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay92FindNearestCourse.filter.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o093/overlay_093.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o093/overlay_093.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o095/overlay95NoOp.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o095/overlay95Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitRadius.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitBounds.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1FC
# The source naturally owns the exact duplicate-guarded registry algorithm.
# Guard three equivalent copy producers and remove one compiler-only duplicate
# address pair before selecting the retained counter-loop register schedule.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96Register.c.o: \
	config/normalizations/overlay96Register.prepare.py \
	config/normalizations/overlay96Register.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96Register.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay96Register.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x24:5:gO96EntriesReloc 0x28:6:gO96EntriesReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x70 90f04fdcb9ccdcdbdc73219931c64e24a325ab0135de7e1610854eed0386025f \
		@config/normalizations/overlay96Register.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
# The adjacent removal helper naturally owns the reverse scan and compaction
# effects. Its guarded preparation adds the retained second array-base pair
# and two counter producers before a complete decoded schedule bijection.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96Unregister.c.o: \
	config/normalizations/overlay96Unregister.prepare.py \
	config/normalizations/overlay96Unregister.ops \
	$(TOOLS_DIR)/add_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96Unregister.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay96Unregister.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x88 2d7baa93466128d0875d5de3ce7a2743496e26ec9f422662bec7c98f054aba1d \
		0x30:HI16:gO96EntriesReloc 0x68:LO16:gO96EntriesReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x88 550bc696bbdf789b252efe93409c4c67d0760e90cc13dab05a77070d8fb11b5a \
		@config/normalizations/overlay96Unregister.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
# The semantic source naturally owns all 241 instructions, the 0x138 frame,
# three calls, both loops, and the complete FP expression tree.  The R4300
# scheduler flag supplies the retail multiply latencies.  Select only two
# bounded local-home translations and one four-site center-coordinate web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96BuildVolume.c.o: \
	config/normalizations/overlay96BuildVolume.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96BuildVolume.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96BuildVolume.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3C4 8a2da0efd540fe9f38097ba8790c2c600f9c50aeb10f6cd61e79d8dd2a4e63b3 \
		@config/normalizations/overlay96BuildVolume.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C4
# Natural source owns the exact 48-word ABI, frame, countdown loops, plane
# test, and four runtime address sites. Swap only two independent FP producers.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96FindVolume.c.o: \
	config/normalizations/overlay96FindVolume.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96FindVolume.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xC0 ce175c624f28dd4507b1a4dc3fe330fda3a4be5b0964e5906d268af4ebb80758 \
		@config/normalizations/overlay96FindVolume.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96TestBit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96DrawObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10C
# Natural source supplies the complete 81-instruction semantic owner. Assert
# and select the target's equivalent three-base allocation/address schedule,
# then restore the two independently proved BSS relocation aliases and pair.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CollectUniqueY.c.o: \
	config/normalizations/overlay98CollectUniqueY.ops \
	$(TOOLS_DIR)/add_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CollectUniqueY.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x144 0a699852811a0f3be8f74fbfeabc0aa068412b2f8b3553cb2a987f905ed56a4c \
		@config/normalizations/overlay98CollectUniqueY.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x94:overlay98UniqueCountReloc:overlay98UniqueYReloc \
		0x98:overlay98UniqueCountReloc:overlay98UniqueYReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x144 0a699852811a0f3be8f74fbfeabc0aa068412b2f8b3553cb2a987f905ed56a4c \
		0xa4:HI16:overlay98UniqueYReloc \
		0xac:LO16:overlay98UniqueYReloc && \
	$(OBJCOPY) --redefine-sym overlay98UniqueCountReloc=D_80 \
		--redefine-sym overlay98UniqueYReloc=D_308 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x144
# These two exact edge owners surround O98's remaining assembly core. Their
# private fail-loud normalizers assert the natural source/relocation hashes and
# select the shipped frame, schedule, and allocation representations.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CollectAccepted.c.o: \
	config/normalizations/overlay98CollectAccepted.prepare.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CollectAccepted.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay98CollectAccepted.prepare.py $@ $@ && \
	$(OBJCOPY) \
		--redefine-sym overlay98AcquireContextReloc=func_overlay_098_F0000000_18D89C0 \
		--redefine-sym gOverlay98AcceptedCount=D_84 \
		--redefine-sym gOverlay98AcceptedEntries=D_88 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF0
# The typed reflection renderer naturally owns the complete 389-instruction
# CFG and all 36 runtime semantic relocation roles. A complete fail-loud
# private representation selects retail's frame/schedule/register web, then
# the split-object stage converts only two proved local addends, removes the
# 20 runtime-local records absent from the static proxy, and rebinds the exact
# 16 retained relocation sites to their shipped static identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98RenderReflections.c.o: \
	config/normalizations/overlay98RenderReflections.prepare.py \
	config/normalizations/overlay98RenderReflections.ops.json \
	config/normalizations/overlay98RenderReflections.prelink.ops \
	config/normalizations/overlay98RenderReflections.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98RenderReflections.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay98RenderReflections.prepare.py $@ $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x614 06675ef5b4e51fa9b8668746c4fd55ffa6495a95ff238599a71604746a0ab580 \
		@config/normalizations/overlay98RenderReflections.prelink.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay98RenderReflections.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym o98AcquireRenderContextReloc=func_overlay_098_F0000000_18D89C0 \
		--redefine-sym gO98Globals=D_88 \
		--redefine-sym gO98SpecialVertices=D_80000000 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x070:o98SetupRenderContextReloc:func_overlay_098_F0000000_18D89C0 \
		0x1e4:o98BuildMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x1f4:o98CombineMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x200:o98LoadMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x2b4:o98BuildInverseMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x30c:o98BuildMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x31c:o98CombineMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x32c:o98CombineMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x338:o98LoadMatrixReloc:func_overlay_098_F0000000_18D89C0 \
		0x4a0:o98RestoreStateReloc:func_overlay_098_F0000000_18D89C0 \
		0x558:o98EmitObjectReloc:func_overlay_098_F0000000_18D89C0 && \
	$(OBJCOPY) --strip-symbol gO98Toggle \
		--strip-symbol o98SetupRenderContextReloc \
		--strip-symbol o98BuildMatrixReloc \
		--strip-symbol o98CombineMatrixReloc \
		--strip-symbol o98LoadMatrixReloc \
		--strip-symbol o98BuildInverseMatrixReloc \
		--strip-symbol o98RestoreStateReloc \
		--strip-symbol o98EmitObjectReloc $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CheckObject.c.o: \
	config/normalizations/overlay98CheckObject.prepare.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CheckObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay98CheckObject.prepare.py $@ $@ && \
	$(OBJCOPY) \
		--redefine-sym overlay98CheckInitialReloc=func_overlay_098_F0000000_18D89C0 \
		--redefine-sym overlay98UniqueCountReloc=D_80 \
		--redefine-sym overlay98UniqueYReloc=D_308 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x154:overlay98CheckCandidateReloc:func_overlay_098_F0000000_18D89C0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1BC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitRadius.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97CopyAngles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitTransform.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitSelection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitPlane.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x110
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitPlane.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97CreateDescriptor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97AssignState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitDirection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitDirection.c.o: CFLAGS += -Wab,-r4300_mul
# IDO canonicalizes values=model+0x3E even though the shipped instruction
# deliberately reuses the already-live bounds=model+0x3C base. Both compute
# the same address; assert the exact compiler word before restoring that base.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitScale.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x240 72713fb9de2dd1e74cab0f174df2d35f22735a5209148498cfb2d3d295668797 \
		fields:0xd0:rs=a1@a3,imm=62@2 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x240
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitScale.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100RemoveEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitRange.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44
# IDO emits the three independent loop initializers in the opposite legal
# order. Assert that exact output before restoring the shipped schedule.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CloneRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x5c 62dfc925593f88cbe83369482560a0e0e9323be9eb08d77a1c771ff1f2429d28 \
		fields:0x20:op=9@35,rs=zero@sp,rt=v1@a0,imm=50@24 \
		fields:0x24:op=35@0,rs=sp@v0,rt=a0@zero,rd=zero@a1,fn=24@37 \
		fields:0x28:op=0@9,rs=v0@zero,rt=zero@v1,rd=a1@zero,fn=37@50 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SelectMaskedMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
# The source is exact except for one complete stack-home lifetime. Preserve the
# independently decoded three-call runtime identities while using the common
# pre-loader relocation carrier required by the configured overlay link.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SearchNearby.c.o: \
	config/normalizations/overlay1SearchNearby.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SearchNearby.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x17C 6db7aa6f757439c4795cf0cde43fb1f9943895cca1c9e1260cdbeea810ca2a1b \
		@config/normalizations/overlay1SearchNearby.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x110:func_80005820:func_8000572C \
		0x148:overlay4RemoveObject:func_8000572C && \
	$(OBJCOPY) --redefine-sym func_8000572C=overlay1SearchRangeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x17C
# Natural source supplies the exact pool traversal and relocation-bearing
# local addends. Select retail's equivalent private suffix register web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AllocateRecord.c.o: \
	config/normalizations/overlay1AllocateRecord.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AllocateRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xA0 e63a50979d5235a5f3f06bd35c6c33ab542831b5be11856201839f3194bb2507 \
		@config/normalizations/overlay1AllocateRecord.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
# Preserve the complete initializer instruction/relocation permutation and
# bounded temporary web, then fold its three runtime calls to the pre-loader
# carrier while retaining their shipped table identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateTransient.c.o: \
	config/normalizations/overlay1UpdateTransient.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateTransient.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x174 8f50cf62ad7b5e78f39c699cf227b7a687f6e6a8083c7a1b73bb039f8e395573 \
		@config/normalizations/overlay1UpdateTransient.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x88:overlay1ReadSelection:func_overlay_036_F0000694_1883B4C \
		0xD4:overlay1InitTimedState:func_overlay_036_F0000694_1883B4C && \
	$(OBJCOPY) --redefine-sym \
		func_overlay_036_F0000694_1883B4C=overlay1TransientCallReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x174
# The natural source supplies the exact cache traversal, integer/FP conversion
# paths, and local relocation pair. Select retail's equivalent two-register web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateValueCache.c.o: \
	config/normalizations/overlay1UpdateValueCache.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateValueCache.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1E0 e5c3cff0bba5bb13d831662306d47866c86702f433636f39b47894546c10e18e \
		@config/normalizations/overlay1UpdateValueCache.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E0
# The natural body owns the exact boundary, frame, CFG, FP schedule, calls,
# delay slots, memory effects, and all eight runtime relocations. Select only
# the two interchangeable private GPR-color webs and fold the resident sqrtf
# call onto its shipped pre-loader relocation carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AppendPathPoint.c.o: \
	config/normalizations/overlay1AppendPathPoint.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AppendPathPoint.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1B0 e65036706b42d770615a5eb643985626525febcebe9119385e6633b68b380c83 \
		@config/normalizations/overlay1AppendPathPoint.ops && \
	$(OBJCOPY) --redefine-sym sqrtf=overlay1SqrtReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1B0
# R4300 multiply hazards are target-proven for this exact TU. Natural source
# supplies the complete instruction stream and all ten runtime relocations;
# fold only the independently decoded resident sqrtf call to its shipped
# pre-loader carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CreateRecord.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CreateRecord.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym sqrtf=overlay1SqrtReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x190
# Natural source owns the exact 162-op schedule, frame, CFG, memory effects,
# stack layout, and all 22 relocation sites. Select only the complete private
# temporary-color web, then fold the two independently decoded external routes
# onto their shipped pre-loader carriers.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvancePath.c.o: \
	config/normalizations/overlay1AdvancePath.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvancePath.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x288 fd2024afe2973ee233170c5b498bf755e0ee7424c7dc11fb5068eb1bfc65dd06 \
		@config/normalizations/overlay1AdvancePath.ops && \
	$(OBJCOPY) --redefine-sym overlay2TracePath=overlay1TracePathReloc \
		--redefine-sym overlay1GetEntry=overlay1GetEntryReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x288
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateCountdown.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
# Natural source owns the exact boundary, frame, CFG, memory effects, and call
# ABI. Restore retail's equivalent count/pointer schedule and complete private
# coloring, remove the two target-proven literal local-pointer relocations, and
# bind the independently decoded active-count and resident-loader identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34RemoveRecord.c.o: \
	config/normalizations/overlay34RemoveRecord.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34RemoveRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xB0 cc5ac0352574865711b8c7e33d86442fec4f461933063962e9ab9bc545ed9bfc \
		@config/normalizations/overlay34RemoveRecord.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x10:5:gOverlay34Pointers 0x20:6:gOverlay34Pointers && \
	$(OBJCOPY) --redefine-sym gOverlay34ActiveCount=D_C \
		--redefine-sym func_80034448=overlay34LoadTextureReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB0
# R4300 multiply hazards are target-proven. Natural source is otherwise exact;
# select the complete private FP web, retain the runtime local-data pair, and
# fold the two independently decoded resident calls to the pre-loader carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87InitializeObject.c.o: \
	config/normalizations/overlay87InitializeObject.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87InitializeObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x128 94a0f9e03cfdbc4b59cdc47c58e10e5ffafa2ff27a77903d361825635a79149b \
		@config/normalizations/overlay87InitializeObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x10C:func_8005AD64:mathRnd && \
	$(OBJCOPY) --redefine-sym mathRnd=overlay87RuntimeCallReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45SetMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ResetState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ReadPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
# All four random-range calls are instruction-natural and share retail's
# offset-zero stored overlay carrier; retain distinct runtime identities in
# the authoritative relocation ledger and trim only section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45RandomizeOffsets.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		overlay45RandomRangeReloc=func_overlay_045_F0000000_188C458 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45SetField22.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45SetField20.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ReleaseDescriptor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45CreateDescriptor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x264
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ConfigureLayout.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x32C
# Materialize the proved local D_510 addend, discard its runtime-only HILO
# records, and fold three distinct resident calls through O47's stored
# offset-zero carrier. Keep +2DE8..+2DF0 as separately owned padding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47SpawnObject.c.o: \
	config/normalizations/overlay47SpawnObject.ops \
	config/normalizations/overlay47SpawnObject.filter.spec \
	config/normalizations/overlay47SpawnObject.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47SpawnObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD8 d2d8771ce53b5eaf618c1cf3293372aca36c8628e162a9efb3b52257b73cfd58 \
		@config/normalizations/overlay47SpawnObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay47SpawnObject.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		func_8000590C=func_overlay_047_F0000000_1890E18 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay47SpawnObject.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
# Natural source owns the exact calls, control flow, loops and instruction
# multiset. Two local scheduler swaps plus the complete reviewed status/data
# allocation web select retail's equivalent private register schedule.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47ReleaseResources.c.o: \
	config/normalizations/overlay47ReleaseResources.schedule.ops \
	config/normalizations/overlay47ReleaseResources.ops \
	config/normalizations/overlay47ReleaseResources.filter.spec \
	config/normalizations/overlay47ReleaseResources.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x160 0a4de66ec65334aa684083f8dea6451b2ae51a9e10f04f01cdba902e8dde8bf9 \
		@config/normalizations/overlay47ReleaseResources.schedule.ops \
		@config/normalizations/overlay47ReleaseResources.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay47ReleaseResources.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym func_overlay_045_F0000270_188C6C8=func_overlay_047_F0000000_1890E18 \
		--redefine-sym D_D0_entries=D_D0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay47ReleaseResources.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x160
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PayloadLimit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61InitResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x21C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61UpdateInput.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ResetCounters.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61AddEntry.c.o: \
	config/normalizations/overlay61AddEntry.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61AddEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1E4 2fba44cde11a74e2cbfb671dc3baaf3a4565e018b70906f5dd8b300dd1b95217 \
		@config/normalizations/overlay61AddEntry.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x404
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawList.c.o: \
	config/normalizations/overlay61DrawList.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawList.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A4 1b62f1584887e240cee20b739399859ab3c1fe8f90baddc10eae002f578c8de5 \
		@config/normalizations/overlay61DrawList.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61WriteCharacter.c.o: \
	config/normalizations/overlay61WriteCharacter.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61WriteCharacter.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xE8 66c95d34d485b9e5c8701f1fdc90791cd66813107868850ad25e464cc26f35d7 \
		@config/normalizations/overlay61WriteCharacter.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReadCharacter.c.o: \
	config/normalizations/overlay61ReadCharacter.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReadCharacter.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x110 e71a892273ffaaa4207316ccb9949613aae490b18c3771657d6bda735044d514 \
		@config/normalizations/overlay61ReadCharacter.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0001648_18C0A10.c.o: \
	config/normalizations/func_overlay_061_F0001648_18C0A10.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0001648_18C0A10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x170 a1e7329d5771c4fe22fc8bd73c95f651d0a790c9ab7a34d027e8ac4dc2b4998b \
		@config/normalizations/func_overlay_061_F0001648_18C0A10.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay85Configure.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay85Configure.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay85Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1DC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitTimedState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1PointerWrap.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
# NON_MATCHING/GLOBAL_ASM per docs/acceleration-survey.md sec.13.2: this
# object's instructions used to be reached by rewriting three fields after
# compilation (normalize_elf_instructions.py), which no gold-standard N64
# decomp does. The .c now GLOBAL_ASMs the extracted retail bytes instead;
# only the symbol rename below (metadata, not instructions) survives.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetEntry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_001_F0000050_184C430=overlay1GetEntry $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetEntryIndex.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindType5ByKey.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x9c daeb9395211c01871e6c40bafdf49a8187ac111a96855d1ed62d05ca5e80271d \
		fields:0x1c:rt=a0@v1 \
		fields:0x20:rt=t7@t6 \
		fields:0x28:rt=a0@v1,rd=t6@t7 \
		fields:0x2c:rs=a0@v1,rt=t7@t6 \
		fields:0x34:rt=t6@t7,rd=v1@a0 \
		fields:0x40:rs=v1@a0,rt=a0@v1 \
		fields:0x44:rs=v1@a0,rt=v1@a0 \
		fields:0x50:rs=a0@v1 \
		fields:0x54:rt=t2@t3 \
		fields:0x5c:rt=a0@v1 \
		fields:0x60:rt=v0@t1 \
		fields:0x64:rs=a0@v1,rt=t1@t2 \
		fields:0x68:rs=v0@t1,rt=t1@t2 \
		fields:0x6c:rt=a0@v1 \
		fields:0x74:rs=a0@v1 \
		fields:0x78:rt=a0@v1 \
		fields:0x7c:rs=a0@v1,rt=t2@t3 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindPreviousUsable.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xa0 de878d3087f5c8fe913df99c0eae33ddf08bd76bcd89a5cabfca7916c65d5c22 \
		fields:0x14:rt=t4@t2 \
		fields:0x20:op=20@4,imm=26@25 \
		fields:0x24:op=35@0,rs=sp@v1,rt=t4@zero,rd=zero@a1,fn=4@37 \
		fields:0x28:op=4@0,rd=zero@a2,fn=23@37 \
		fields:0x2c:op=9@4,rs=v1@a1,rt=t1@zero,imm=65535@22 \
		fields:0x30:op=0@9,rs=t1@a1,rt=zero@a1,rd=a1@ra,sa=0@31,fn=37@63 \
		fields:0x34:rt=t2@t0 \
		fields:0x44:op=0@9,rs=t1@v1,rt=zero@a0,rd=a0@ra,sa=0@31,fn=37@63 \
		fields:0x48:rt=t2@t0 \
		fields:0x88:rt=t4@t2 \
		fields:0x8c:rt=t3@t1 \
		fields:0x94:rs=t4@t2,rt=t3@t1 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1PreviousIndex.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1NextIndex.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1Noop.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CallReset.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ReturnZero.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SubmitGlobals.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SubmitAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1RelativeAngles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AngleBetweenSamples.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ScaledDistance.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1DistanceFromCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ScaledDistance.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1DistanceFromSelected.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetLinkedActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CopyBytes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ModeChecks.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateModeSound.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ReadSelection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagCC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15ReleaseResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseHandle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseTree.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureResource.c.o: \
	config/normalizations/overlay20ConfigureResource.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x15C a1f6c87daeef1c5ae09c92c11aa98cde8b5be8e597a5ce6f7e674f26fc0f0725 \
		@config/normalizations/overlay20ConfigureResource.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateObjectResource.c.o: \
	config/normalizations/overlay20UpdateObjectResource.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateObjectResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x188 a627ff31fd721ca0b53ad4f40af40eb92c7a1f3b57a73ed41658736b3deb840b \
		@config/normalizations/overlay20UpdateObjectResource.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x30:overlay20LookupReloc:overlay20GetContextReloc \
		0x134:overlay20ConfigureResourceReloc:overlay20GetContextReloc \
		0x168:overlay20SqrtReloc:overlay20GetContextReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay20GetContextReloc=func_overlay_020_F0000000_18765D8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x188
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20BuildTileCommands.c.o: \
	config/normalizations/overlay20BuildTileCommands.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20BuildTileCommands.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x218 9262eca64c4c5593bf9cf7ce808c01ef25e8f85c2f4fa528ea5ab5f5ab223865 \
		@config/normalizations/overlay20BuildTileCommands.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x218
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20RemoveEntry.c.o: \
	config/normalizations/overlay20RemoveEntry.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20RemoveEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD4 8710b61d5300efc59a2b9587c24fc6a9678dca7facfd3879dbf1f7ea478d1d4c \
		@config/normalizations/overlay20RemoveEntry.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureEntry.c.o: \
	config/normalizations/overlay20ConfigureEntry.ops \
	$(TOOLS_DIR)/resize_elf_function.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x150 52e6985c989687badb9e133d31d7d396a78cbb1f08f61019b502d907b28cef34 \
		@config/normalizations/overlay20ConfigureEntry.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/resize_elf_function.py $@ .text \
		overlay20ConfigureEntry 0x14C 0x150 \
		52e6985c989687badb9e133d31d7d396a78cbb1f08f61019b502d907b28cef34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20MarkNested.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20AdvanceEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20CreateEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20DrawResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
# The typed source owns the exact boundary, CFG, FP behavior, calls, and
# overlap-array semantics. The guarded ledger selects retail's equivalent
# frame/allocation/schedule web; the relocation fold and local-data filter
# preserve the runtime relocation surface proven by Overlay 20's retail table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o: \
	config/normalizations/overlay20UpdateGrid.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o: CFLAGS += \
	-Wab,-r4300_mul -DEXPLICIT_BOUNDS -DSCAN_TOP_LOAD
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x35C 6f03934f0a2bdd75e0fb6dd0817476de322ac6d1b4b056d851bbbf68e692ef0f \
		@config/normalizations/overlay20UpdateGrid.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x248:func_8002A8C0:func_overlay_020_F0000000_18765D8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x3c:5:gOverlay20EntryCount 0x4c:6:gOverlay20EntryCount \
		0x70:5:gOverlay20Entries 0x74:6:gOverlay20Entries && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x35C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
# The typed initializer naturally owns the exact frame, semantic CFG, calls,
# FP behavior, and all table writes. A whole-owner guard expands two complete
# equivalent induction/sign-extension webs; after trimming the asserted growth
# pad, a bijective schedule/register ledger selects retail's private codegen.
# The remaining steps preserve the exact split-object relocation surface while
# the original loader table remains authoritative for its ten runtime roles.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildLookupTables.c.o: \
	config/normalizations/overlay31BuildLookupTables.prepare.py \
	config/normalizations/overlay31BuildLookupTables.ops \
	config/normalizations/overlay31BuildLookupTables.filter.spec \
	config/normalizations/overlay31BuildLookupTables.calls.spec \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildLookupTables.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay31BuildLookupTables.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2E8 \
		0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2E8 d88c1a31ad4fba9e9467ccb8a25ca3e0c5d81cdf8d218afabf3899bccee70b36 \
		@config/normalizations/overlay31BuildLookupTables.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay31BuildLookupTables.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay31BuildLookupTables.calls.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay31AllocateReloc=func_overlay_031_F0000000_187F520 \
		--redefine-sym gOverlay31IndexRows=D_10 \
		--redefine-sym gOverlay31FloatRows=D_8 $@
# The clean-room analogue naturally owns all 245 semantic words, branches,
# calls, delay slots, FP behavior, and runtime state effects. A guarded
# frame/home-offset ledger selects retail's equivalent stack representation;
# loader-local HILO carriers are then removed and the 16 runtime call roles
# rebound to their exact raw overlay identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeBuffers.c.o: \
	config/normalizations/overlay31InitializeBuffers.ops \
	config/normalizations/overlay31InitializeBuffers.calls.spec \
	config/normalizations/overlay31InitializeBuffers.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeBuffers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3E0 cb378b217ed41fd48c13e895e82c7403a5f1e0902eac0ea32ea942e8525b60eb \
		@config/normalizations/overlay31InitializeBuffers.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3D4 \
		000000000000000000000000 && \
	$(OBJCOPY) \
		--redefine-sym overlay31ResetReloc=func_overlay_031_F0000000_187F520 \
		--redefine-sym overlay31CreateConfig=func_overlay_031_F0000A84_187FFA4 \
		--redefine-sym overlay31CreateRecords=func_overlay_031_F0000DC4_18802E4 \
		--redefine-sym overlay31CreatePool=func_overlay_031_F0000E7C_188039C $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay31InitializeBuffers.calls.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay31InitializeBuffers.filter.spec
# Natural output is opcode/CFG/frame/relocation exact. The guarded ledger
# selects four complete private compiler-representation webs only.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateConfig.c.o: \
	config/normalizations/overlay31CreateConfig.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateConfig.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x340 43fe93dec2619d929e2a047471d108014dc9916045bcbbcfab2ea9a323779782 \
		@config/normalizations/overlay31CreateConfig.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x340
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreatePool.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xc8 65bb467311dab6250086aced8802fe747e840ccd33b24cbb9cbaed2947e4f79d \
		fields:0x0:imm=65488@65480 \
		fields:0x28:rd=a0@a1 \
		fields:0x30:rd=a1@a2 \
		fields:0x38:rs=a0@a1 \
		fields:0x3c:rs=a0@a1 \
		fields:0x40:rs=a0@a1 \
		fields:0x44:rs=a0@a1 \
		fields:0x48:rs=a0@a1 \
		fields:0x4c:rs=a0@zero,rt=v0@v1,imm=12@3 \
		fields:0x50:rs=zero@a1,rt=v1@v0,imm=3@12 \
		fields:0x70:rs=a1@a2,rt=a1@a2 \
		fields:0x74:rs=a1@a2 \
		fields:0x78:rs=a0@a1,rt=a0@a1 \
		fields:0xbc:imm=48@56 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildPalettes.c.o: \
	config/normalizations/overlay31BuildPalettes.filter.spec \
	config/normalizations/overlay31BuildPalettes.calls.spec \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildPalettes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1B8 \
		0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay31BuildPalettes.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay31BuildPalettes.calls.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay31AllocateReloc=func_overlay_031_F0000000_187F520 $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
# The SDK-shaped source naturally owns the exact opcode/relocation inventory.
# A reviewed one-to-one instruction permutation plus decoded-field web selects
# the shipped private schedule; loader-owned HILO rows are then removed and
# all runtime call roles are folded through the overlay's stored-zero proxy.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33BuildDisplayList.c.o: \
	config/normalizations/overlay33BuildDisplayList.ops \
	config/normalizations/overlay33BuildDisplayList.filter \
	config/normalizations/overlay33BuildDisplayList.rebind \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33BuildDisplayList.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4D0 4403550f5ebac1faac2a1b2edaacba47fe06feffce7cdcfa1ae561697166aba0 \
		@config/normalizations/overlay33BuildDisplayList.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay33BuildDisplayList.filter && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay33BuildDisplayList.rebind && \
	$(OBJCOPY) --redefine-sym \
		overlay33GetDimensionsReloc=func_overlay_033_F0000000_18807E8 $@ && \
	$(OBJCOPY) --redefine-sym \
		overlay33BuildDisplayList=func_overlay_033_F000019C_1880984 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4D0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33InitializeBuffers.c.o: \
	config/normalizations/overlay33InitializeBuffers.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33InitializeBuffers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x144 322f51bac3cc17a6e96e3789a7fb157bd99fbad27d10f4e9564cc70276a732de \
		@config/normalizations/overlay33InitializeBuffers.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x144
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33ReleaseGlobal.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33PresentAndSwap.c.o: \
	config/normalizations/overlay33PresentAndSwap.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33PresentAndSwap.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x9C 814466e0a7afaddec241e49f39018b65fd53db67cbe679f7a37dcde1d14f9f91 \
		@config/normalizations/overlay33PresentAndSwap.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CallModes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46Submit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x120
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46ReleaseState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x88 5ca308ce799aec42b766643a87b2cc75d1018262d3472ed74f3129a890e2343a \
		fields:0x20:rt=a0@v0 \
		fields:0x24:rs=a0@v0,rt=a0@v0 \
		fields:0x28:rs=a0@v0 \
		fields:0x34:rs=zero@v0,rd=zero@a0,fn=0@37 \
		fields:0x38:rt=a0@v0 \
		fields:0x3c:rs=a0@v0,rt=a0@v0 \
		fields:0x40:rs=a0@v0 \
		fields:0x4c:rs=zero@v0,rd=zero@a0,fn=0@37 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
# The complete loop and all effects are natural. IDO schedules the independent
# loop-counter constant before the resource-table low half; the shipped object
# places those adjacent, unconditional initializations in the opposite order.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeParticles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1d8 4146a371cd66f6cdb6dcfa32ce1d72d7d2766683a0b3ac3fe202104e58580881 \
		fields:0x60:rs=zero@s4,rt=s3@s4,imm=18@0 \
		fields:0x64:rs=s4@zero,rt=s4@s3,imm=0@18 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1D8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeBuffers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeBuffers.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0xC:5:gOverlay46DisplayState \
		0x14:6:gOverlay46DisplayState \
		0x18:4:func_800221E8 \
		0x20:5:gOverlay46DisplayState \
		0x24:5:gOverlay46DisplayOutput \
		0x28:6:gOverlay46DisplayOutput \
		0x2C:4:func_80022B94 \
		0x30:6:gOverlay46DisplayState \
		0x84:4:overlay41IsUnitScale \
		0xD0:4:func_80028D30 \
		0x12C:5:gOverlay46FadeOutput \
		0x130:4:func_80039E34 \
		0x134:6:gOverlay46FadeOutput \
		0x13C:5:gOverlay46FadeOutput \
		0x140:6:gOverlay46FadeOutput && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0

# This typed spawn-record initializer naturally reproduces the shipped CFG,
# calls, memory effects, and FP associations. The target-local normalization
# selects one independent initializer ordering and two complete private
# register-carrier webs; its digest rejects any source or compiler drift.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65SpawnRecord.c.o: \
	config/normalizations/overlay65SpawnRecord.ops \
	config/normalizations/overlay65SpawnRecord.filter.spec \
	config/normalizations/overlay65SpawnRecord.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65SpawnRecord.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65SpawnRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A0 9b0c911accf406d31d99ff7c931a2562ec89312201d6244367c741d655420b00 \
		@config/normalizations/overlay65SpawnRecord.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay65SpawnRecord.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		o65GetCamera=func_overlay_065_F0000000_18C4268 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay65SpawnRecord.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A0

# The typed source owns O64's complete procedural texture generator. IDO's
# natural stream contains four redundant representations; the target-local
# digest-guarded preparation removes exactly those words before a complete
# decoded schedule/register selection. Restore all 20 shipped runtime carrier
# records, then expose only the two configured R26 call records; the retained
# relocation tail owns the 18 loader-local HILO records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay64GenerateTexture.c.o: \
	config/normalizations/overlay64GenerateTexture.prepare.py \
	config/normalizations/overlay64GenerateTexture.ops \
	config/normalizations/overlay64GenerateTexture.filter.spec \
	$(TOOLS_DIR)/set_elf_symbol_size.py \
	$(TOOLS_DIR)/add_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay64GenerateTexture.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay64GenerateTexture.c.o: POSTPROCESS = \
	$(OBJCOPY) -O binary --only-section=.text $@ $@.natural.bin && \
	$(HOST_PYTHON) config/normalizations/overlay64GenerateTexture.prepare.py \
		$@.natural.bin $@.intermediate.bin && \
	$(OBJCOPY) --update-section .text=$@.intermediate.bin $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/set_elf_symbol_size.py $@ \
		func_overlay_064_F0000000_18C3B28 0x6A0 0x690 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x690 c938acb87cc923647cf57bf5b5d0d7bcb02b23dc68d95db529a73716a59f744f \
		@config/normalizations/overlay64GenerateTexture.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x690 c938acb87cc923647cf57bf5b5d0d7bcb02b23dc68d95db529a73716a59f744f \
		0x004:HI16:gO64BufferSelect 0x008:LO16:gO64BufferSelect \
		0x02c:HI16:gO64BuffersA 0x034:LO16:gO64BuffersA \
		0x048:HI16:gO64BuffersB 0x050:LO16:gO64BuffersB \
		0x070:HI16:gO64Initialized 0x074:LO16:gO64Initialized:4 \
		0x0d0:R26:o64RandomRange \
		0x100:HI16:gO64Initialized 0x108:LO16:gO64Initialized:4 \
		0x150:R26:o64RandomRange \
		0x1a0:HI16:gO64BufferSelect 0x1a4:LO16:gO64BufferSelect \
		0x1a8:HI16:gO64BufferSelect 0x1bc:LO16:gO64BufferSelect \
		0x1c4:HI16:gO64BuffersA 0x1d0:LO16:gO64BuffersA \
		0x1d8:HI16:gO64BuffersB 0x1e0:LO16:gO64BuffersB && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay64GenerateTexture.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xd0:o64RandomRange:func_overlay_064_F0000000_18C3B28 \
		0x150:o64RandomRange:func_overlay_064_F0000000_18C3B28 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x690

$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F0000000_1885D10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x154

$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay38UpdateParticles.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay38UpdateParticles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x328

$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F000047C_188618C.c.o: CFLAGS += \
	-Wab,-r4300_mul -DO38_TRANSFORM_TAIL -DO38_TAIL_SIZE=8 \
	-DO38_VOLATILE_TEST -DO38_POOL_CURSOR -DO38_FINISH_ONE \
	-DO38_VOLATILE_FINAL
$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F000047C_188618C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x36C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000000_18C91C8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8

$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2AC

$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3A4
# The typed particle/update body preserves every loop bound, call, relocation,
# memory effect, and FP association. IDO emits the same 720 semantic carriers
# in a compact private schedule; the fail-loud preparation and complete
# relocation-anchored permutation select the shipped schedule without dropping
# or synthesizing any semantic instruction.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65UpdateParticles.c.o: \
	config/normalizations/overlay65UpdateParticles.prepare.py \
	config/normalizations/overlay65UpdateParticles.ops \
	config/normalizations/overlay65UpdateParticles.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65UpdateParticles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay65UpdateParticles.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xB40 54a8f06a6028cc9f93e4c6ffdcf0001451d64409c20a95acdcd78be28a3538ed \
		@config/normalizations/overlay65UpdateParticles.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay65UpdateParticles.filter.spec && \
	$(OBJCOPY) --redefine-sym o65BeginDraw=func_overlay_065_F0000000_18C4268 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x0b4:o65GetCamera:func_overlay_065_F0000000_18C4268 \
		0x0c0:o65PrepareCamera:func_overlay_065_F0000000_18C4268 \
		0x0cc:o65LoadCursor:func_overlay_065_F0000000_18C4268 \
		0x270:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x298:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x2ac:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x2bc:o65Sin:func_overlay_065_F0000000_18C4268 \
		0x2c8:o65Cos:func_overlay_065_F0000000_18C4268 \
		0x318:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x328:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x338:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x348:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x358:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x368:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x378:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x388:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x3b0:o65FindGround:func_overlay_065_F0000000_18C4268 \
		0x5f4:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x648:o65Cos:func_overlay_065_F0000000_18C4268 \
		0x680:o65Transform:func_overlay_065_F0000000_18C4268
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65ResetSlots.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65ResetSlots.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ByteLength.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101PromoteSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair2.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedFloat.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedByte.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair3.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedScaled.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x100
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x120
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateFrames.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleFrames.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x140
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleGlobalPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
# The natural event dispatcher emits retail's complete CFG, switch topology,
# frame, register allocation, FP conversion, call order, and instruction
# schedule. Four guarded immediates preserve already-linked local addends; the
# narrow relocation filters and rebinding retain the authoritative overlay
# runtime-relocation carriers instead of IDO's private jump table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchEvents.c.o: \
	config/normalizations/overlay101DispatchEvents.ops \
	config/normalizations/overlay101DispatchEvents.filter.spec \
	config/normalizations/overlay101DispatchEvents.calls.spec \
	tools/filter_elf_relocations.py tools/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchEvents.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchEvents.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3B0 ff35c5287a31c725af0a3d102589584438017af1fd8521f71740edd01ba42e14 \
		@config/normalizations/overlay101DispatchEvents.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay101DispatchEvents.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay101SchedulePair=func_overlay_101_F0000000_18DB820 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay101DispatchEvents.calls.spec && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B0
# The shipped dispatcher uses an existing runtime-relocated jump table at
# overlay data +0xE0C. IDO emits an identical private .rodata table; patch the
# three proved raw addends, then discard compiler relocations/table bytes so
# the original initialized-data and relocation assets remain authoritative.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x17c 8eed8377c1aabac2fbdfff0a4a1c5398898fb595c0cc70b9d8e55c4efb9d6752 \
		fields:0x4:imm=0@540 \
		fields:0x28:imm=0@2120 \
		fields:0x4c:imm=0@3596 && \
	$(OBJCOPY) --remove-relocations=.text --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x17C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawElement.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101GetBounds.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
# Natural codegen has the exact boundary, frame, ABI, calls, CFG, FP topology,
# and stack layout. This complete ledger selects two command schedules and
# equivalent private temporary-register webs.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawTransformed.c.o: \
	config/normalizations/overlay101DrawTransformed.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawTransformed.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x298 e6114aff1a4f4a44186b450d9e131e32c22a44f327c329692e644bb998576515 \
		@config/normalizations/overlay101DrawTransformed.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x298
# The natural builder owns the complete four-call CFG and all presentation
# effects, but redundantly rematerializes one live color constant. Delete that
# asserted addiu, then select retail's equivalent private schedule/register web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationA.c.o: \
	config/normalizations/overlay101BuildPresentationA.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x340 9e322bc8ea678df948b5a43818782e38bcf5e467b7b447009a73da514d0c5b75 \
		@config/normalizations/overlay101BuildPresentationA.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x340
# This sibling uses the same semantic pool graph with different asset, input,
# root-field, and final-text addends. Rebind only those distinct source roles;
# the raw ROM keeps all pool addends and the local byte-length call explicit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationB.c.o: \
	config/normalizations/overlay101BuildPresentationB.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x340 e3e1d372193b8528644c24bdf898c82dd4de190e3af6545d18b0aca93d07c821 \
		@config/normalizations/overlay101BuildPresentationB.ops && \
	$(OBJCOPY) \
		--redefine-sym gOverlay101BuilderAssetA=gOverlay101BuilderAssetBReloc \
		--redefine-sym gOverlay101BuilderText=gOverlay101BuilderTextBReloc \
		--redefine-sym gOverlay101BuilderInput12C=gOverlay101BuilderInput12CBReloc \
		--redefine-sym gOverlay101BuilderInput130=gOverlay101BuilderInput130BReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x340
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationC.c.o: \
	config/normalizations/overlay101BuildPresentationC.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x340 971f9a1e8fb7b51d04cab8c4b6d0a142e40dc48c7b1002ec267240f34765316d \
		@config/normalizations/overlay101BuildPresentationC.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x340
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationD.c.o: \
	config/normalizations/overlay101BuildPresentationD.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationD.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x338 f124fe4d95cd03a7159cb63bfe7f79b4f6d74a69aa18aa167bb713897c00c277 \
		@config/normalizations/overlay101BuildPresentationD.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x338
# IDO naturally owns the complete three-word ABI, six-call semantic graph,
# and all relocation identities. The guarded preparation recovers three
# retained address rematerializations from one unused argument home and the
# object's two alignment words, then the bijective ledger selects the shipped
# private frame/register/schedule web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailA6BC.c.o: \
	config/normalizations/overlay101TailA6BC.prepare.py \
	config/normalizations/overlay101TailA6BC.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailA6BC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay101TailA6BC.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x490 b2fabc29043d5683740994ee79109613a336941b8cd3e08b8a15ac559e778f53 \
		@config/normalizations/overlay101TailA6BC.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x490
$(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16InitializeBuffer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16BuildGradient.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16ReleaseBuffer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
# The typed natural source is exact except for two CFG-proven unreachable
# duplicate ternary stores and one closed whole-function allocation/schedule
# web. Each deletion and the final equivalent representation are fail-loud and
# digest-guarded; the following three-nop island remains separate padding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16ApplyGradient.c.o: \
	config/normalizations/overlay16ApplyGradientDrop1.ops \
	config/normalizations/overlay16ApplyGradientDrop2.ops \
	config/normalizations/overlay16ApplyGradient.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16ApplyGradient.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x248 cac4e5e080672e3f26e69ff7516c5d76a332c3168cbc8912126c5930cca3b045 \
		@config/normalizations/overlay16ApplyGradientDrop1.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x244 6d34c26ac7aa59633b65cb90eec39abc025afbd9f827b4cbbe0dc5ec696b6fb4 \
		@config/normalizations/overlay16ApplyGradientDrop2.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x244 e51a18791518c07f21b505a4105a51c8d4ef354bba38f2444af0ec2562aa78e6 \
		@config/normalizations/overlay16ApplyGradient.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x244
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
# The natural owner recovers the exact loop, initialization, switch CFG, five
# calls, and FP behavior. Select the complete private frame/allocation web and
# a closed five-instruction schedule, then bind the five calls to their shipped
# resident or overlay-local identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeAndUpdate.c.o: \
	config/normalizations/overlay84InitializeAndUpdate.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeAndUpdate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2CC c72042be3d18284708ca2b10269ea50b71eb9bcfa6e4414f48bb8943bb6ad1a2 \
		@config/normalizations/overlay84InitializeAndUpdate.ops && \
	$(OBJCOPY) \
		--redefine-sym overlay84GetNodes=func_overlay_084_F0000000_18D04E0 \
		--redefine-sym overlay84RefreshCurrent=func_overlay_084_F0000B7C_18D105C \
		--redefine-sym overlay84UpdateCurrent=func_overlay_084_F0000314_18D07F4 \
		--redefine-sym overlay84UpdateResource=func_overlay_084_F0000A54_18D0F34 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x1D0:overlay84Atan2:func_overlay_084_F0000000_18D04E0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84IsUnitScale.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetEnabledCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ResetCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84UpdateResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84RefreshCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15ReleaseResource10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
# Restore the compiler-folded chain-address materialization and its complete
# private register web under immutable natural text/relocation guards.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Initialize.c.o: \
	$(TOOLS_DIR)/normalize_o63_initialize.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_o63_initialize.py $@ $@
# The natural owner has the exact size, frame, CFG, call topology, and external
# identities. Normalize the complete bounded private allocation/schedule web,
# materialize the proved local particle base, then discard only compiler
# section-alignment bytes beyond the executable boundary.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateEffects.c.o: \
	$(TOOLS_DIR)/normalize_o63_update_effects.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateEffects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_o63_update_effects.py $@ $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x578
# IDO folds one token-identity copy in this function. Restore that complete
# private representation web under immutable text/relocation guards, then keep
# the overlay's two target-owned padding words as a separate asm segment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateSequence.c.o: \
	$(TOOLS_DIR)/normalize_o63_update_sequence.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateSequence.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_o63_update_sequence.py $@ $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1AC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreateEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleasePrimary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseSecondary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreatePayload.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68AttachObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateTrail.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ClearNestedFlag.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68FinishEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68StartTimer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
# Natural codegen has exact size, opcodes, calls, copy loop, and data roles.
# This complete ledger selects retail's private frame, owner precolor, spill
# web, and equivalent branch-likely null exit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PromoteSecondary.c.o: \
	config/normalizations/overlay68PromoteSecondary.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PromoteSecondary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x134 fafb9f7f98cc195d6c8f8cb3c7a41c25d21e71052fdf6bcf0af9ba0efb2a4a66 \
		@config/normalizations/overlay68PromoteSecondary.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
# IDO emits the complete interpolation body followed by three alignment NOPs.
# Prove and adopt that full owner, then apply its complete schedule/FP web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68Interpolate.c.o: \
	config/normalizations/overlay68Interpolate.ops \
	$(TOOLS_DIR)/extend_elf_function_to_text.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68Interpolate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/extend_elf_function_to_text.py $@ \
		overlay68Interpolate 0x284 0x290 \
		b47465737613f5ed72e307b277b8fa93f9537c5c53b16b80e50c0ae30a8d5671 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x290 1f78119c6e5c9e039c4b62a5b9ce725dca2c76de4d836a3dbfd71d8843bfe7fa \
		@config/normalizations/overlay68Interpolate.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
# Natural source owns the exact frame, boundary, CFG, calls, and runtime
# relocation sites. The complete decoded-field ledger selects retail's
# equivalent private allocation; the call-symbol fold and four-record filter
# are asserted metadata transforms for relocation ownership already preserved
# by the shipped overlay tables.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateAnimation.c.o: \
	config/normalizations/overlay68UpdateAnimation.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateAnimation.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x590 47bef232fa52b1c315950abfbce6cbb2d4bffa828e393fec9409965c59d015e4 \
		@config/normalizations/overlay68UpdateAnimation.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x480:overlay68Angle2Reloc:overlay68Angle3Reloc \
		0x494:overlay68AngleDifferenceReloc:overlay68Angle3Reloc \
		0x550:overlay68SetDirectionReloc:overlay68Angle3Reloc \
		0x578:overlay68AdvanceObjectReloc:overlay68Angle3Reloc && \
	$(OBJCOPY) --redefine-sym \
		overlay68Angle3Reloc=func_overlay_068_F0000000_18C7160 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x4:5:gOverlay68GlobalFlagReloc \
		0x8:6:gOverlay68GlobalFlagReloc \
		0x4b0:5:gOverlay68GlobalFlagReloc \
		0x4b4:6:gOverlay68GlobalFlagReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x590
# Natural source supplies the exact boundary, CFG, memory effects, call count,
# runtime relocation sites, and stack-array layout. The reviewed schedule
# permutations preserve the peeled adjacent swap and move the prepare call with
# its relocation; the decoded-field ledger selects retail's equivalent private
# register and stack homes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68DrawSortedEntries.c.o: \
	config/normalizations/overlay68DrawSortedEntries.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68DrawSortedEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x354 c18d9d92f517f3e0773420470978c3b973bb4ace3c52da97470e0394dd24a717 \
		@config/normalizations/overlay68DrawSortedEntries.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x248:overlay68PrepareDrawReloc:overlay68MeasureVectorReloc \
		0x310:overlay68SubmitEntryReloc:overlay68MeasureVectorReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay68MeasureVectorReloc=func_overlay_068_F0000000_18C7160 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x354
# Natural codegen is exact in boundary, frame, opcode schedule, calls, and
# relocation positions. This complete ledger selects the retail private stack
# and register homes, then the trimmer removes compiler section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68RebuildSecondaryEntry.c.o: \
	config/normalizations/overlay68RebuildSecondaryEntry.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68RebuildSecondaryEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1E8 b5559458459d8bb9b7d681a504b4a08f376f284327ba73e0e5e2b2c40f6c9cdd \
		@config/normalizations/overlay68RebuildSecondaryEntry.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseTertiary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
# The natural object is exact in size, frame, CFG, calls, and relocation
# positions. These asserted words select the shipped private stack homes, one
# equal table-loop comparison order, one complete cursor/fallback schedule,
# and the complete dependent temp/result webs; observable values and effects
# are unchanged.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CheckKind.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x140 8440b1fb0117a7ff680a82b6844372516cefa33e00b32555666201bdde86324b \
		fields:0x14:imm=32@52 \
		fields:0x50:rs=v1@a1,rt=a1@v1 \
		fields:0x68:imm=48@68 \
		fields:0x6c:imm=48@68 \
		fields:0x90:imm=60@44 \
		fields:0x9c:imm=56@36 \
		fields:0xa8:imm=56@36 \
		fields:0xbc:imm=60@44 \
		fields:0xc0:op=9@0,rt=t7@zero,rd=zero@t4,sa=0@1,fn=1@0 \
		fields:0xcc:op=0@9,rs=a1@zero,rt=zero@t8,rd=v1@zero,fn=37@1 \
		fields:0xd0:op=21@5,imm=3@2 \
		fields:0xd4:op=35@0,rt=t3@t4,rd=zero@v1,fn=16@33 \
		fields:0x100:rd=t4@t5 \
		fields:0x104:rt=t4@t5,rd=t5@t6 \
		fields:0x108:rs=t5@t6,rt=t6@t7 \
		fields:0x10c:rs=t6@t7 \
		fields:0x120:rt=t7@t8,imm=32@52 \
		fields:0x130:imm=32@52 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x140
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2Enable.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ValidateRegion.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8002A910=overlay2AngleReloc \
		--redefine-sym func_8002AA0C=overlay2AngleDifferenceReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1BC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ContainsPoint.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8002A4C0=overlay2PointAngleReloc \
		--redefine-sym func_8002A5BC=overlay2PointAngleDifferenceReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2CopyColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AppendLine.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x108
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClassifyBoundary.c.o: \
	config/normalizations/overlay2ClassifyBoundary.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClassifyBoundary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x13c 2df25f34d8fb3c8f9c9120c0cc14fa170d63d896721be9184b3cc3986ffb8fb5 \
		@config/normalizations/overlay2ClassifyBoundary.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2IntersectBoundary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClipLines.c.o: \
	config/normalizations/overlay2ClipLines.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClipLines.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x244 05945c5c8e0d7cf4e5162c0cc09a29a384971983ea728dd8dff640324ffd3721 \
		@config/normalizations/overlay2ClipLines.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x244
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ChooseBoundary.c.o: \
	config/normalizations/overlay2ChooseBoundary.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/add_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ChooseBoundary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x490 a45b77d27880ee2229a1ee2cd5a9bf96b30c5044c74ab609f5b8571472fb6fdd \
		@config/normalizations/overlay2ChooseBoundary.ops && \
	$(OBJCOPY) --remove-section=.mdebug $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x490 a45b77d27880ee2229a1ee2cd5a9bf96b30c5044c74ab609f5b8571472fb6fdd \
		0xf8:HI16:gOverlay2BoundaryValue \
		0x110:LO16:gOverlay2BoundaryValue
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2SplitRegion.c.o: \
	config/normalizations/overlay2SplitRegion.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2SplitRegion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x120 61e1236ca6089c167c1e1135ad43343e56086e8b5ea88b1384fc45d41a086fe8 \
		@config/normalizations/overlay2SplitRegion.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AdjacentIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
# The typed recursive query owns the exact 253-word CFG, call order, FP work,
# and all 51 runtime roles. Fail-loud complete representation/schedule webs
# select retail's equivalent boolean and GPR forms; the filter preserves the
# assembler-authored 25-record static surface and its exact REL order.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2QueryNode.c.o: \
	config/normalizations/overlay2QueryNode.ops \
	config/normalizations/overlay2QueryNode.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/order_o2_query_node_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2QueryNode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3F4 ef3ea4b8784e8d7945bd323a04b1bd2f50e9672716dc4506a58a60813582cc58 \
		@config/normalizations/overlay2QueryNode.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay2QueryNode.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay2IntersectSegments=func_overlay_002_F0000000_1856DF8 \
		--redefine-sym overlay2IntersectBoundary=func_overlay_002_F0000400_18571F8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/order_o2_query_node_relocations.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3F4
# Natural IDO output owns the exact 217-word CFG, all seven calls, branch and
# FPU schedules, and the complete 59-record runtime relocation surface. This
# guarded ledger selects three complete private representation webs while the
# object-local aliases preserve retail's zero SYMBOL carriers and LOCAL data
# addends without changing global compiler policy.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o: \
	config/normalizations/func_overlay_002_F0001A94_185888C.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o: \
	CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o: \
	POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x364 c67d3450c49c538373561a073b7bf37777e00e1fae06537c74cb3c35bde99d5e \
		@config/normalizations/func_overlay_002_F0001A94_185888C.ops && \
	$(OBJCOPY) \
		--redefine-sym overlay1GetEntry=overlay1GetEntryReloc \
		--redefine-sym overlay2ContainsPoint=overlay2ContainsPointReloc \
		--redefine-sym overlay2QueryNode=func_overlay_002_F00016A0_1858498 \
		--redefine-sym overlay2AdjacentIndices=func_overlay_002_F0001658_1858450 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x364 \
		000000000000000000000000
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawBorder.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawLine.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB0
# The natural 222-word gradient writer owns the exact CFG, FP lanes, stack
# object home, calls, and twelve runtime relocation roles. Six complete
# schedule permutations plus the complete asserted GPR web select retail's
# equivalent allocation; call carriers retain loader-owned zero payloads.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c.o: \
	config/normalizations/func_overlay_060_F0002F54_18BCD2C.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c.o: \
	CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x378 a18144a89d174564a22e637800757affc2aa407ca83dbfa2e98ce9d2cac3dca6 \
		@config/normalizations/func_overlay_060_F0002F54_18BCD2C.ops && \
	$(OBJCOPY) \
		--redefine-sym func_overlay_082_F00004A4_18CF624=func_overlay_060_F0000000_18B9DD8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xE0:func_80034554:func_overlay_060_F0000000_18B9DD8 \
		0x1A4:func_80036600:func_overlay_060_F0000000_18B9DD8 \
		0x1BC:func_80036660:func_overlay_060_F0000000_18B9DD8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x378
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReassignChoiceSlots.c.o: \
	config/normalizations/overlay60ReassignChoiceSlots.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReassignChoiceSlots.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD4 2a3fabec79e38bf8428c6143b62568b39f6b18fab2dede2315ddef45df7ebf64 \
		@config/normalizations/overlay60ReassignChoiceSlots.ops && \
	$(OBJCOPY) \
		--redefine-sym gOverlay60ChoicesPass1=overlay60ChoiceTableRuntimeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x24:gOverlay60ChoicesPass1End:overlay60ChoiceTableRuntimeReloc \
		0x28:gOverlay60ChoicesPass1End:overlay60ChoiceTableRuntimeReloc \
		0x60:gOverlay60ChoicesPass2:overlay60ChoiceTableRuntimeReloc \
		0x64:gOverlay60ChoicesPass2End:overlay60ChoiceTableRuntimeReloc \
		0x68:gOverlay60ChoicesPass2End:overlay60ChoiceTableRuntimeReloc \
		0x6C:gOverlay60ChoicesPass2:overlay60ChoiceTableRuntimeReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Call.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13CreateRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xFC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
# Complete guarded loop/allocator representation for the O13 per-record
# updater. The source retains all five runtime roles; the split object embeds
# the gravity-local addend, so filter only that asserted pair and retain the
# exact call plus active-count static relocation surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessRecord.c.o: \
	config/normalizations/overlay13ProcessRecord.ops \
	config/normalizations/overlay13ProcessRecord.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x284 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x284 a6e97f7b24f2e06e498b3141515c6a332922d16d4649e579f1796608fa1ce285 \
		@config/normalizations/overlay13ProcessRecord.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay13ProcessRecord.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay13Prepare=func_overlay_013_F0000000_186EB18 \
		--redefine-sym gOverlay13ActiveCount=D_2C $@
# Exact-size semantic renderer with an opcode-identical complete carrier web.
# Keep the six call and two local HILO pairs exposed by the split object;
# remove only the eight asserted runtime-local records whose addends are
# already embedded in the shipped instruction words.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawRecord.c.o: \
	config/normalizations/overlay13DrawRecord.ops \
	config/normalizations/overlay13DrawRecord.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F4 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2F4 76239653392277e946f5184a8ae604fdfd86ddf8d1afdbccef40cb9a4c228f5a \
		@config/normalizations/overlay13DrawRecord.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay13DrawRecord.filter.spec && \
	$(OBJCOPY) --redefine-sym o13GetRenderState=func_overlay_013_F0000000_186EB18 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x08c:o13SetColor:func_overlay_013_F0000000_186EB18 \
		0x0c4:o13DrawRecord:func_overlay_013_F0000000_186EB18 \
		0x0cc:o13FinishDraw:func_overlay_013_F0000000_186EB18 \
		0x0e4:o13SetupRecord:func_overlay_013_F0000000_186EB18 \
		0x284:o13DrawRecord:func_overlay_013_F0000000_186EB18 && \
	$(OBJCOPY) --strip-symbol D_20 --strip-symbol D_28 --strip-symbol D_4 \
		--strip-symbol o13SetColor --strip-symbol o13DrawRecord \
		--strip-symbol o13FinishDraw --strip-symbol o13SetupRecord $@
# Exact semantic active-record collector/sorter with a complete guarded
# carrier web. Extend the natural function over its existing alignment nop,
# restore the second pool-base HILO pair, and filter only the eight asserted
# embedded local-addend records. The two call relocations remain exposed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawActive.c.o: \
	config/normalizations/overlay13DrawActive.ops \
	config/normalizations/overlay13DrawActive.filter.spec \
	config/normalizations/overlay13DrawActive.extend.py \
	$(TOOLS_DIR)/add_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x298 && \
	$(HOST_PYTHON) config/normalizations/overlay13DrawActive.extend.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x298 ebf3ae79472203e7ad40559db961e8792b31bce86b13e5f5e54a48c46b997f90 \
		@config/normalizations/overlay13DrawActive.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x298 ebf3ae79472203e7ad40559db961e8792b31bce86b13e5f5e54a48c46b997f90 \
		0x244:5:D_0 0x248:6:D_0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay13DrawActive.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym o13GetView=func_overlay_013_F0000000_186EB18 $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11EnableHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11DisableHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C8
# Natural source recovers all 301 opcodes, the complete CFG, and all 102
# relocation sites. Select only the asserted private frame/spill schedule,
# retain distinct loader identities through semantic zero carriers, and trim
# the compiler's independent section-alignment tail.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateMenu.c.o: \
	config/normalizations/overlay11UpdateMenu.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateMenu.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4B4 18c39fe96915d7681d054af2c46d57ee22e57a52ca27f482bea3f9cf218c6204 \
		@config/normalizations/overlay11UpdateMenu.ops && \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_80005820=overlay11GetObjectReloc \
		--redefine-sym func_80028374=overlay11StartTransitionReloc \
		--redefine-sym func_80028528=overlay11CommitTransitionReloc \
		--redefine-sym func_8003A754=overlay11ModeActionReloc \
		--redefine-sym func_800290AC=overlay11SetModeReloc \
		--redefine-sym func_800291D8=overlay11SetTimerReloc \
		--redefine-sym func_800006BC=overlay11StartEffectReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue \
		--redefine-sym func_overlay_066_F0000000=overlay11SelectOverlay66Reloc \
		--redefine-sym func_overlay_011_F0002BF4_186B43C=overlay11ReleaseCurrentGroupRuntimeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4B4
# Natural source recovers all 140 instructions and 46 relocation sites. The
# only private residual is the complete two-use loop-index spill home.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o: \
	config/normalizations/overlay11UpdateTwoOptionMenu.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x230 46895030579b29364c8b4dba93d432f619bb787dcc04f9f126d7e2a8cf4b361a \
		@config/normalizations/overlay11UpdateTwoOptionMenu.ops && \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_80028374=overlay11StartTransitionReloc \
		--redefine-sym func_80028528=overlay11CommitTransitionReloc \
		--redefine-sym func_800290AC=overlay11SetModeReloc \
		--redefine-sym func_800291D8=overlay11SetTimerReloc \
		--redefine-sym func_800006BC=overlay11StartEffectReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue \
		--redefine-sym func_overlay_066_F0000000=overlay11SelectOverlay66Reloc \
		--redefine-sym func_overlay_011_F0002BF4_186B43C=overlay11ReleaseCurrentGroupRuntimeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x230
# The natural 244-instruction dispatcher has the exact CFG, registers, calls,
# and 78 text relocation sites. Select its complete two-use private spill home,
# point the switch dispatch at the retained runtime-local table, and discard
# only the compiler's duplicate five-entry table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o: \
	config/normalizations/overlay11UpdateFiveOptionMenu.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3D0 a828dc0ff707d26c81caf34c2732c7eab8cd0c26ba6b2cf40529419b90739189 \
		@config/normalizations/overlay11UpdateFiveOptionMenu.ops && \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_80005820=overlay11GetObjectReloc \
		--redefine-sym func_8002675C=overlay11GetTransitionReloc \
		--redefine-sym func_80028374=overlay11StartTransitionReloc \
		--redefine-sym func_800290AC=overlay11SetModeReloc \
		--redefine-sym func_800291D8=overlay11SetTimerReloc \
		--redefine-sym func_800006BC=overlay11StartEffectReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue \
		--redefine-sym func_overlay_066_F0000000=overlay11SelectOverlay66Reloc \
		--redefine-sym func_overlay_011_F0002BF4_186B43C=overlay11ReleaseCurrentGroupRuntimeReloc $@ && \
	$(OBJCOPY) --add-symbol gOverlay11FiveOptionSwitchTableReloc=0x40,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x164:.rodata:gOverlay11FiveOptionSwitchTableReloc \
		0x16c:.rodata:gOverlay11FiveOptionSwitchTableReloc && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3D0
# Exact typed five-way dispatcher. Select the complete 296-carrier schedule,
# extend the owner over IDO's existing alignment carrier, retain the 41-record
# split relocation surface, and discard only the duplicate compiler table
# already present byte-for-byte in the overlay's extracted data/rodata asset.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0001E4C_186A694.c.o: \
	config/normalizations/func_overlay_011_F0001E4C_186A694.ops \
	config/normalizations/func_overlay_011_F0001E4C_186A694.filter.spec \
	config/normalizations/func_overlay_011_F0001E4C_186A694.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/resize_elf_function.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0001E4C_186A694.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4A0 e09d692e5da08232200d18b426bc670abea418a7111040d0b835bbf2fde523a7 \
		@config/normalizations/func_overlay_011_F0001E4C_186A694.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/resize_elf_function.py $@ .text \
		func_overlay_011_F0001E4C_186A694 0x498 0x49C \
		b960ae1b20c9d886667aebccbce4261fd57dd2679856a9ddf8894d2935ff0bfa && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x49C && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_011_F0001E4C_186A694.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_011_F0001E4C_186A694.rebind.spec && \
	$(OBJCOPY) --remove-section=.rodata $@
# Adjacent exact typed dispatcher. Select its complete 268-carrier schedule,
# retain the exact 39-record split text surface, and discard only the duplicate
# compiler table already preserved in the original data/rodata asset.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F00022E8_186AB30.c.o: \
	config/normalizations/func_overlay_011_F00022E8_186AB30.ops \
	config/normalizations/func_overlay_011_F00022E8_186AB30.filter.spec \
	config/normalizations/func_overlay_011_F00022E8_186AB30.rebind.spec \
	config/normalizations/func_overlay_011_F00022E8_186AB30.link.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F00022E8_186AB30.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x430 67d9fc03f2b950dc1064833de6e25ac479b471a0cfb2163286575249728c7be4 \
		@config/normalizations/func_overlay_011_F00022E8_186AB30.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x42C && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_011_F00022E8_186AB30.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_011_F00022E8_186AB30.rebind.spec && \
	$(OBJCOPY) --add-symbol overlay11EmbeddedLocalReloc=0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_011_F00022E8_186AB30.link.rebind.spec && \
	$(OBJCOPY) --remove-section=.rodata $@
# Natural source is exact in length, frame, CFG, delay slots, calls, and all
# 47 relocation sites. Select only the complete two-use private loop-index
# spill home, then map the established runtime proxy identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o: \
	config/normalizations/overlay11UpdateModeSix.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x234 74bf71b2575100930e5c76d3271264058e0d58d22a3471e158c106e6e14d71b5 \
		@config/normalizations/overlay11UpdateModeSix.ops && \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_80028374=overlay11StartTransitionReloc \
		--redefine-sym func_800290AC=overlay11SetModeReloc \
		--redefine-sym func_800291D8=overlay11SetTimerReloc \
		--redefine-sym func_800006BC=overlay11StartEffectReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue \
		--redefine-sym func_overlay_066_F0000000=overlay11SelectOverlay66Reloc \
		--redefine-sym func_overlay_011_F0002BF4_186B43C=overlay11ReleaseCurrentGroupRuntimeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x234
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11CreateHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
# The compiler emits the exact six-entry switch table already present in the
# overlay's extracted data/rodata asset. Rebind the text pair to its proved
# runtime-local `+8` addend, then discard only the duplicate private table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_800290AC=overlay11ResidentModeReloc \
		--redefine-sym func_800005CC=overlay11ResidentFloatReloc \
		--redefine-sym overlay66Select=overlay11Overlay66SelectReloc \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym func_8004B0A4=overlay11DrawModeReloc \
		--redefine-sym func_8004B0B8=overlay11DrawColorReloc $@ && \
	$(OBJCOPY) --add-symbol gOverlay11SwitchTableReloc=0x8,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xA8:.rodata:gOverlay11SwitchTableReloc \
		0xB0:.rodata:gOverlay11SwitchTableReloc && \
	$(OBJCOPY) --remove-section=.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym sprintf=overlay11FormatReloc \
		--redefine-sym func_overlay_045_F000000C_188B438=overlay11CreateReloc \
		--redefine-sym D_800D31BC=gOverlay11ResidentFlagsReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
# IDO emits a private switch table, while Mickey uses the runtime-relocated
# table at overlay +0x7C. Resolve the proved raw addends, then discard the
# duplicate table and compiler relocations; the overlay relocation assets
# remain authoritative at runtime.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseCurrentGroup.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xc0 bf28e7e108072d4b6f9c332eb7b3242d207d5a28206ff315b015fa98877ee16a \
		fields:0x4:imm=0@436 \
		fields:0x38:imm=0@124 \
		fields:0x44:target=0@2642 \
		fields:0x54:target=0@2692 \
		fields:0x64:target=0@2667 \
		fields:0x88:target=0@2742 \
		fields:0x98:target=0@2717 \
		fields:0xa8:target=0@2767 && \
	$(OBJCOPY) --remove-relocations=.text --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Submit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildLine.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1AC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawLines.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
# The natural source recovers the exact boundary and effect topology. Apply
# the complete relocation-aware schedule/allocation web, then trim only IDO's
# standalone section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Update.c.o: \
	config/normalizations/overlay83Update.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x274 a49c5debb35204e89f8dad2a03cd83cbc136e118a4a348451b9fe7da63bc1e13 \
		@config/normalizations/overlay83Update.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x274
# The typed batch builder naturally recovers the exact boundary, calls, CFG,
# FP work, and memory effects. Select the complete reviewed schedule/frame/
# allocation web, then remove only the two local-data records already owned
# by the shipped runtime relocation table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildBatch.c.o: \
	config/normalizations/overlay83BuildBatch.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildBatch.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2A0 e7fd906c95b9f31cb6634b5461cb2d766f366c8e9d580644c50d15a122ae2ad4 \
		@config/normalizations/overlay83BuildBatch.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x7c:5:gOverlay83ScaleReloc 0xa0:6:gOverlay83ScaleReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2A0
# IDO assigns the shared all-white word to v1; the shipped object assigns the
# same constant to a2. Assert and restore the three register-only words.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83SubmitAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x148 3537e34bc0e0604031acb3f85b46d54b15f8899f79f4ae4bd1d6076ed419c808 \
		fields:0xdc:rt=v1@a2 \
		fields:0x104:rt=v1@a2 \
		fields:0x118:rt=v1@a2 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x148
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
# The inline command builder has the exact four-command CFG and relocation
# topology. Select its complete relocation-aware private schedule/register web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawStrip.c.o: \
	config/normalizations/overlay83DrawStrip.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawStrip.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x134 e12017ffc3810cd8977f1393ff9efe328401b0e1f2906d08235b9594b6febc7d \
		@config/normalizations/overlay83DrawStrip.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Dispatch.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x94
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99GetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ReleaseEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99InitializeEntries.c.o: \
	config/normalizations/overlay99InitializeEntries.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99InitializeEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1B8 e38eb2e72a078f6c86903ca83131436351fbcaef931e520568bf4e37006df684 \
		@config/normalizations/overlay99InitializeEntries.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1B8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ProjectVector.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
# The typed source naturally recovers the complete equation, grid traversal,
# strict FP predicates, and runtime identities. Select the asserted bijective
# private compiler representation, then preserve the runtime table's ownership
# of local data relocations and the assembly owner's exact static REL order.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ApplySegment.c.o: \
	config/normalizations/overlay99ApplySegment.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/order_o99_apply_segment_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ApplySegment.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3A0 6a4c04cb13e226cbf4b5e263a6f623961971d7c95f8cb55be7603aebed7aad57 \
		@config/normalizations/overlay99ApplySegment.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x398 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x68:5:gOverlay99CurrentGrid 0x6c:6:gOverlay99CurrentGrid \
		0x74:5:gOverlay99Grids 0x80:6:gOverlay99Grids \
		0xcc:5:.rodata 0xd8:6:.rodata \
		0x8c:5:gOverlay99GridHeight 0x10c:6:gOverlay99GridHeight \
		0x114:5:gOverlay99GridWidth 0x138:6:gOverlay99GridWidth \
		0x310:5:gOverlay99GridWidth 0x314:6:gOverlay99GridWidth \
		0x32c:5:gOverlay99GridHeight 0x330:6:gOverlay99GridHeight && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x2d8:overlay99AngleWavePhaseReloc:overlay99AngleWave && \
	$(OBJCOPY) \
		--redefine-sym gOverlay99Arg5=D_A8 \
		--redefine-sym gOverlay99HeightMinusOne=D_A0 \
		--redefine-sym gOverlay99Arg4=D_A4 \
		--redefine-sym gOverlay99WidthMinusOne=D_9C \
		--redefine-sym overlay99ProjectVector=func_overlay_099_F000021C_18D97CC \
		--redefine-sym overlay99AngleWave=func_overlay_099_F0000000_18D95B0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/order_o99_apply_segment_relocations.py $@
# The source owns the exact CFG and every data effect. Normalize two complete
# allocation webs, then remove only private-data relocation records already
# owned by the overlay runtime table and bind the retained local identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99BuildHeightGrid.c.o: \
	config/normalizations/overlay99BuildHeightGrid.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99BuildHeightGrid.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99BuildHeightGrid.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1C8 671c4fc3c3753b31adeecf41847cfe122d083dd4fa4cb45b4a2b55b706460729 \
		@config/normalizations/overlay99BuildHeightGrid.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:gOverlay99CurrentGrid 0x4:6:gOverlay99CurrentGrid \
		0x110:5:gOverlay99CurrentGrid 0x114:6:gOverlay99CurrentGrid \
		0x8:5:gOverlay99Grids 0x18:6:gOverlay99Grids \
		0x11c:5:gOverlay99Grids 0x128:6:gOverlay99Grids \
		0x54:5:gOverlay99WidthMinusOne 0x58:6:gOverlay99WidthMinusOne \
		0x60:5:gOverlay99HeightMinusOne 0x64:6:gOverlay99HeightMinusOne \
		0x6c:5:gOverlay99Arg4 0x70:6:gOverlay99Arg4 \
		0x74:5:gOverlay99Arg5 0x78:6:gOverlay99Arg5 \
		0x84:5:gOverlay99Segments 0x90:6:gOverlay99Segments \
		0xb8:5:gOverlay99SegmentCount 0xbc:6:gOverlay99SegmentCount && \
	$(OBJCOPY) \
		--redefine-sym gOverlay99GridWidth=D_94 \
		--redefine-sym gOverlay99GridHeight=D_98 \
		--redefine-sym overlay99ApplySegment=func_overlay_099_F00002A0_18D9850 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C8
# The typed renderer naturally recovers the exact boundary, calls, CFG, FP
# topology, and memory effects. Select two complete local ownership webs plus
# one command-definition schedule, collapse the six runtime-relocated call
# identities to their shipped carrier, and remove only the two private data
# pairs already represented by the overlay relocation assets.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSortedEntries.c.o: \
	config/normalizations/overlay99RenderSortedEntries.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSortedEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3A4 2ce9c275ee2ae1f50473ef3b34a4b74c8490407446b66920e17ec0fc961392ec \
		@config/normalizations/overlay99RenderSortedEntries.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x1e4:overlay99GetTransform:overlay99Measure \
		0x220:overlay99UpdateState:overlay99Measure \
		0x258:overlay99BuildMatrix:overlay99Measure \
		0x32c:overlay99BuildRecord:overlay99Measure \
		0x360:overlay99DrawEntry:overlay99Measure && \
	$(OBJCOPY) --redefine-sym \
		overlay99Measure=func_overlay_099_F0000000_18D95B0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x1f4:5:D_4 0x21c:6:D_4 \
		0x260:5:D_8 0x264:6:D_8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3A4
# This renderer naturally recovers the complete CFG, FP schedule, calls, and
# memory effects. Select its two complete ownership webs, collapse the six
# runtime helper roles to their shipped carrier, and remove only private-data
# relocations already encoded by the overlay's runtime relocation assets.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSegments.c.o: \
	config/normalizations/overlay99RenderSegments.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSegments.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSegments.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x238 e5f3c1534bd285bbf9985e72be5b28a7aac2b939d285cd6b66dad6e951327a38 \
		@config/normalizations/overlay99RenderSegments.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xe8:overlay99Setup:overlay99Begin \
		0xf0:overlay99End:overlay99Begin \
		0x11c:overlay99Angle:overlay99Begin \
		0x158:overlay99Sqrt:overlay99Begin \
		0x1b0:overlay99DrawObject:overlay99Begin && \
	$(OBJCOPY) --redefine-sym \
		overlay99Begin=func_overlay_099_F0000000_18D95B0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x04:5:gOverlay99SegmentCount 0x08:6:gOverlay99SegmentCount \
		0x2c:5:gOverlay99Segments 0x60:6:gOverlay99Segments \
		0xb8:5:gOverlay99Texture 0xc0:6:gOverlay99Texture \
		0x1dc:5:gOverlay99SegmentCount 0x1e0:6:gOverlay99SegmentCount && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x238
# Natural IDO owns the exact 205-word frame/CFG/FP/runtime-relocation surface.
# Two guarded schedule bijections and the complete private/runtime-local field
# web select retail's representation. Loader-owned proxy relocations are
# removed only after their exact local addends have been embedded.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Initialize.c.o: \
	config/normalizations/overlay60Initialize.ops \
	config/normalizations/overlay60Initialize.filter.spec \
	config/normalizations/overlay60Initialize.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x334 55b6e4a4d9711c24ab2356241e27050da2843b434a0593c921ce24b384917d16 \
		@config/normalizations/overlay60Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay60Initialize.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay60Initialize.rebind.spec && \
	$(OBJCOPY) \
		--redefine-sym gOverlay60Data28=D_28 \
		--redefine-sym gOverlay60Data38=D_38 \
		--redefine-sym gOverlay60ObjectC8=D_C8 \
		--redefine-sym gOverlay60CoordsD8=D_D8 \
		--redefine-sym gOverlay60Data58=D_58 \
		--redefine-sym overlay60SpawnReloc=func_overlay_060_F0000000_18B9DD8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x334 \
		000000000000000000000000
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateInterface.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57InitializeMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57BeginMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x90
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57StartMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x98
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x170 84ad281985fb26137e1aa0764014bf410461a0eb5b6379e39d4764259a93fd0e \
		reorder:0x18=0x1c,0x1c=0x18 \
		reorder:0x94=0x98,0x98=0x94 \
		reorder:0xbc=0xc0,0xc0=0xbc \
		reorder:0xe4=0xe8,0xe8=0xe4 \
		reorder:0x10c=0x110,0x110=0x10c \
		reorder:0x134=0x138,0x138=0x134 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x170
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58EnsureResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
# IDO emits the exact 201-instruction CFG, calls, memory and FP effects.  The
# guarded transform selects retail's equivalent private register webs and
# instruction schedule; its body digest rejects compiler or source drift.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawSegmentStrip.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x324 6f7bdd1370a5fa6dfe783fdbb72b92e24de4bbbd01937eed56aa024056336639 \
		@config/normalizations/overlay58DrawSegmentStrip.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x324
# These sibling point-quad renderers have the same straight-line schedule and
# allocator shape. The guarded normalization records only five permutations
# and the complete private GPR web; the distinct body digests own constants.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawPointQuad.c.o: \
	config/normalizations/overlay58DrawPointQuad.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawPointQuad.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A0 c7d842be0b07c50f045741be61abf1fef99b863e863b24862d7b525456db963e \
		@config/normalizations/overlay58DrawPointQuad.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawLargePointQuad.c.o: \
	config/normalizations/overlay58DrawPointQuad.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawLargePointQuad.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A0 3fde2a7981b466b62a0325f93dbb381681074764acf34ade45ce1078446c0d79 \
		@config/normalizations/overlay58DrawPointQuad.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A0
# IDO emits the exact CFG, frame, calls, memory effects, and delay slots.  The
# guarded transform selects two complete interchangeable register webs; its
# final body digest rejects compiler or source drift.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58RefreshRankSet.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x260 f29edacad6763566df09648e4416a720ed1f09b34a14bbb45b047856c873aa48 \
		@config/normalizations/overlay58RefreshRankSet.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x260
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58FinalizePackedStatus.c.o: \
	config/normalizations/overlay58FinalizePackedStatus.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58FinalizePackedStatus.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4C0 1c9aefdde67debfa8947030463e84c10625082bd5188137618e74cb8de39b27d \
		@config/normalizations/overlay58FinalizePackedStatus.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17CalculateEndpoints.c.o: \
	config/normalizations/overlay17CalculateEndpoints.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17CalculateEndpoints.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x318 dd9d1eee3f8e2d2cef8ad3543c188f0ca2e6115dce5a01f6f023b2cf107509aa \
		@config/normalizations/overlay17CalculateEndpoints.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x108:overlay17SqrtReloc:overlay17TransformReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay17TransformReloc=func_overlay_017_F0000000_18739B8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x318
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17CreateChain.c.o: \
	config/normalizations/overlay17CreateChain.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17CreateChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x310 46372736806a2433adde61c1a5ba86787af17f3014c58b1df88aab7c6838e118 \
		@config/normalizations/overlay17CreateChain.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0xbc:5:gOverlay17TemplateReloc \
		0xc8:6:gOverlay17TemplateReloc
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17ReleaseChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
# The typed chain update is naturally exact from +0x104 onward.  This guarded
# ledger selects the complete private pre-call copy/schedule/register web while
# preserving the exact frame, call, relocation, CFG, and memory effects.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17AdvanceChain.c.o: \
	config/normalizations/overlay17AdvanceChain.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17AdvanceChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x24C 9fd1ba4bc365994bede02dbd38aafbca1d3af31bf6fbb73efec1c76ee92e5bf5 \
		@config/normalizations/overlay17AdvanceChain.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24C
# Natural IDO output has the exact 119-instruction CFG, calls, memory effects,
# and delay slots.  The guarded transform selects retail's equivalent register
# web and redundant-move schedule; the digest rejects any compiler drift.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17DrawStrip.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1DC 281eb4cb4086e40a6b5e868f8e17a7ec6cabe46c7449ccff83e3cc5b2389f69a \
		reorder:0x48=0x4c,0x4c=0x48,0x50=0x64,0x54=0x58,0x58=0x60,0x5c=0x50,0x60=0x68,0x64=0x6c,0x68=0x74,0x6c=0x70,0x70=0x78,0x74=0x54,0x78=0x7c,0x7c=0x80,0x80=0x84,0x84=0x88,0x88=0x8c,0x8c=0x94,0x90=0x5c,0x94=0x90 \
		fields:0x0:imm=65472@65480 \
		fields:0x48:rt=a0@v0 \
		fields:0x50:rt=t6@t7 \
		fields:0x54:rs=a0@v0,rt=t8@t6 \
		fields:0x58:rt=t8@t6 \
		fields:0x5c:rt=t9@t8 \
		fields:0x60:rs=a0@v0,rt=t9@t8 \
		fields:0x64:rs=a0@v0,rt=t6@t7 \
		fields:0x6c:rt=t7@t9 \
		fields:0x70:rd=v1@a0 \
		fields:0x74:rs=zero@t3,rt=t5@zero,rd=t6@v1,sa=24@0,fn=0@37 \
		fields:0x78:rt=t7@t9,rd=t8@t6 \
		fields:0x7c:rt=t8@t6,rd=t9@t7 \
		fields:0x80:rs=t9@t7 \
		fields:0x90:rs=zero@v0,rt=t6@zero,rd=t5@a1,sa=24@0,fn=3@37 \
		fields:0x94:imm=70@69 \
		fields:0xb8:rt=t6@t8 \
		fields:0xbc:rs=t6@t8 \
		fields:0xd4:rs=v1@a0 \
		fields:0xdc:rs=v1@a0 \
		fields:0x100:rs=t1@t3 \
		fields:0x110:rd=a2@a1 \
		fields:0x114:rs=a2@a1,rt=t8@t6 \
		fields:0x11c:rt=t8@t6,rd=t9@t7 \
		fields:0x120:rt=t7@t9 \
		fields:0x124:rt=t7@t9 \
		fields:0x128:rs=t9@t7,rt=t6@t8 \
		fields:0x12c:rt=t6@t8,rd=t7@t9 \
		fields:0x130:rd=t9@t7 \
		fields:0x134:rt=t9@t7,rd=t6@t8 \
		fields:0x138:rs=t7@t9,rd=t8@t6 \
		fields:0x13c:rs=t6@t8,rt=t7@t9 \
		fields:0x140:rs=t7@t9,rt=t9@t7 \
		fields:0x144:rs=t8@t6,rt=t9@t7,rd=t6@t8 \
		fields:0x148:rt=t6@t8 \
		fields:0x14c:rt=a2@a1 \
		fields:0x154:rt=t8@t6 \
		fields:0x158:rt=t8@t6,rd=t9@t7 \
		fields:0x15c:rt=t7@t9 \
		fields:0x160:rt=t7@t9 \
		fields:0x164:rs=t9@t7,rd=t6@t8 \
		fields:0x168:rs=t6@t8,rt=t7@t9 \
		fields:0x16c:rt=t7@t9,rd=t8@t6 \
		fields:0x170:rt=t6@t8 \
		fields:0x174:rt=t6@t8,rd=t7@t9 \
		fields:0x178:rs=t8@t6,rd=t9@t7 \
		fields:0x17c:rs=t7@t9,rt=t8@t6 \
		fields:0x180:rs=t9@t7,rt=t8@t6,rd=t6@t8 \
		fields:0x184:rt=t6@t8 \
		fields:0x188:rt=t7@t9 \
		fields:0x190:rs=t7@t9,rd=t9@t7 \
		fields:0x194:rt=t9@t7 \
		fields:0x19c:rs=t3@t1,rd=v0@a0 \
		fields:0x1ac:rt=v1@v0 \
		fields:0x1b0:rt=t6@t8 \
		fields:0x1b4:rs=v1@v0,rt=t8@t6 \
		fields:0x1b8:rt=t8@t6 \
		fields:0x1bc:rs=v1@v0 \
		fields:0x1c0:rs=v1@v0,rt=t6@t8 \
		fields:0x1d8:imm=64@56 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1DC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
# IDO emits the exact straight-line startup topology, 60-record relocation
# contract, and display-list protocol. Select the shipped result-publication
# schedule, private display web, and two equal zero materializations.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Load.c.o: \
	config/normalizations/overlay18Load.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Load.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1F4 1a01a1043cb89075aee41c1996e67877a16534decf934cc1110c54963247cd88 \
		@config/normalizations/overlay18Load.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F4
# IDO emits the exact control-flow, call, memory, and integer-opcode topology,
# plus one provably redundant copy. Restore the shipped schedule, private stack
# homes, and complete interchangeable GPR webs with fail-loud semantic ops.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Reconfigure.c.o: \
	config/normalizations/overlay18Reconfigure.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Reconfigure.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2A8 ca8ba5ac3f8cf00edf9f3e7813f455a8e050bca61ecfd33b37f312e42b9ec325 \
		@config/normalizations/overlay18Reconfigure.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2A8
# IDO emits the exact frame, straight-line CFG, opcodes, calls, and relocation
# contract. Select the shipped global-load schedules, local addends, private
# spill, and complete interchangeable temporary webs with fail-loud ops.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18InitializeBuffers.c.o: \
	config/normalizations/overlay18InitializeBuffers.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18InitializeBuffers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x15C 39053edd81f942317d4d015740accb86008ac870c7a55962b9f8577182014fea \
		@config/normalizations/overlay18InitializeBuffers.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
# Natural codegen has the exact startup CFG, frame, opcode census, and all 33
# loader records. A two-word guarded schedule plus two proved local addends
# selects retail; runtime-only pairs remain owned by the loader tables.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55Initialize.c.o: \
	config/normalizations/overlay55Initialize.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x13C 4cb07c78079f15449f1b9f01a76c3544174f4cf8478b052353e65698c0ebbba3 \
		@config/normalizations/overlay55Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x8:5:gOverlay55SetupBase \
		0x30:6:gOverlay55SetupBase \
		0x34:5:gOverlay55ExternalSetupTarget \
		0x3C:6:gOverlay55ExternalSetupTarget \
		0x58:5:gOverlay55StateWord \
		0x5C:6:gOverlay55StateWord \
		0x88:5:gOverlay55SourceBase \
		0x9C:6:gOverlay55SourceBase \
		0xF4:5:gOverlay55StateValue \
		0xFC:6:gOverlay55StateValue \
		0x10C:5:gOverlay55Result \
		0x130:6:gOverlay55Result && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
# IDO selects two different overlapping entry-color register webs while every
# opcode, frame/stack slot, FP lane, call, branch, and delay slot is exact.
# Assert every definition/use in both complete webs before selecting retail.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x498 30c832783e8a26045cfc34de24edb5128c87de4e615fbedcb655f6a21400e4e9 \
		fields:0x44:rd=v1@at \
		fields:0x50:rt=v1@at,rd=a3@v1 \
		fields:0x54:rt=v1@at \
		fields:0x64:rt=a3@v1 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x498
$(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87ReleaseCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87HasNearby.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87HasNearby.c.o: CFLAGS += -Wab,-r4300_mul
# Natural IDO owns the exact 185-word boundary and complete initialization
# semantics. The guarded preparation restores the retail branch-likely web;
# relocation-aware ledgers select the shipped private schedule and addends.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Initialize.c.o: \
	config/normalizations/overlay50Initialize.prepare.py \
	config/normalizations/overlay50Initialize.shared.filter.spec \
	config/normalizations/overlay50Initialize.ops \
	config/normalizations/overlay50Initialize.addends.ops \
	config/normalizations/overlay50Initialize.filter.spec \
	config/normalizations/overlay50Initialize.calls.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay50Initialize.shared.filter.spec && \
	$(HOST_PYTHON) config/normalizations/overlay50Initialize.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2E4 2650cc1bb861ab7b536ed954284b44c77b1f92a8d869f80c17811c818f376a0c \
		@config/normalizations/overlay50Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2E4 255dfa61cac617f8f98578be8e3a63a9824a388f91521b394f18564219f61bb4 \
		@config/normalizations/overlay50Initialize.addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay50Initialize.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay50PatchIndices=func_overlay_050_F00002E4_1896C54 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay50Initialize.calls.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2E4 \
		000000000000000000000000
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Cleanup.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50SubmitTimeGlyphs.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x214
$(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay51PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
# Mickey-local typed source owns O52's exact initializer frame, calls, loops,
# memory effects, and all runtime roles. Four complete schedule bijections plus
# the asserted private allocation/address representations select retail's
# equivalent compiler web; the relocation contract retains the exact 113 rows.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o: \
	config/normalizations/overlay52Initialize.ops \
	config/normalizations/overlay52Initialize.filter.spec \
	config/normalizations/overlay52Initialize.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4F0 3e08c19f673e58baa9cbf0033efb03e32678b287e7dd04318ca285b549a0b4ba \
		@config/normalizations/overlay52Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay52Initialize.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay52Initialize.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52CopyOffsetEntries.c.o: \
	config/normalizations/overlay52CopyOffsetEntries.sort.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52CopyOffsetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xFC \
		00000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x40:overlay52QuerySecondaryModeReloc:overlay52QueryPrimaryModeReloc && \
	$(OBJCOPY) \
		--redefine-sym overlay52QueryPrimaryModeReloc=func_overlay_052_F0000000_189A670 \
		--redefine-sym gOverlay52Offsets=D_27C $@ && \
	$(HOST_PYTHON) config/normalizations/overlay52CopyOffsetEntries.sort.py $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Cleanup.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x90
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
# Typed source supplies the exact frame, ten calls, fixed two-entry loop, and
# complete runtime relocation surface. The guarded ledger selects one ordering
# of independent address finalizers and restores two complete local addends.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53Initialize.c.o: \
	config/normalizations/overlay53Initialize.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x11C cd11ededd4cde879a67cd92b8e409fa7e49eb78bb6df7a149d7c8797ef60feec \
		@config/normalizations/overlay53Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53CopyOffsetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
# Typed source recovers O54's exact initializer ABI, two loops, 26-call order,
# memory effects, frame, and complete opcode inventory plus one scheduler NOP.
# This fail-loud target-local contract selects retail's equivalent schedule and
# allocation web, deletes only that asserted NOP, and restores all 86 static
# relocation identities and their original REL order.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54Initialize.c.o: \
	config/normalizations/overlay54Initialize.ops \
	config/normalizations/overlay54Initialize.filter.spec \
	config/normalizations/overlay54Initialize.rebind.spec \
	config/normalizations/overlay54Initialize.symbols \
	config/normalizations/overlay54Initialize.sort.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3CC 016c1293209a5133b692731b77bda48be2214a1172f20e2003ce6f62be2ba017 \
		@config/normalizations/overlay54Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3CC \
		00000000 && \
	while IFS= read -r symbol; do \
		test -z "$$symbol" || $(OBJCOPY) --add-symbol "$$symbol=0,global" $@; \
	done < config/normalizations/overlay54Initialize.symbols && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay54Initialize.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay54Initialize.rebind.spec && \
	$(HOST_PYTHON) config/normalizations/overlay54Initialize.sort.py $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay91Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay91UpdateTimeline.c.o: POSTPROCESS = \
	$(OBJCOPY) --remove-relocations=.text --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x470
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay91UpdateTimeline.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay91Render.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SetNodeValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58SetNodeValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay51ReleaseState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54CopyOffsetRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54GetOffsets.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55CopyOffsetRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55GetOffsets.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3A.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6A.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateForward.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateBackward.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29Select.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36TickState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36PrepareAndTick.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SelectState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
# The typed weighted-state selector reaches the exact frame, CFG, constants,
# calls, and runtime relocation topology under the measured R4300 multiply
# scheduler. A guarded redundant-rematerialization deletion and complete
# private scheduling/register web select the shipped representation; 28
# asserted loader-local HILO records are removed from the five-call split.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36ChooseWeightedState.c.o: \
	config/normalizations/overlay36ChooseWeightedState.ops \
	config/normalizations/overlay36ChooseWeightedState.filter.spec \
	config/normalizations/overlay36ChooseWeightedState.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36ChooseWeightedState.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36ChooseWeightedState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2A8 26ec72f1b7c01edb717392a5a01004f4a135b4284c5ff556490c45096548ce69 \
		@config/normalizations/overlay36ChooseWeightedState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay36ChooseWeightedState.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2A8 && \
	$(OBJCOPY) --add-symbol \
		func_overlay_036_F0000000_18834B8=0xF0000000,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay36ChooseWeightedState.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdateInteractiveEntity.c.o: \
	config/normalizations/overlay36UpdateInteractiveEntity.source_shape.ops \
	config/normalizations/overlay36UpdateInteractiveEntity.reorder.ops \
	config/normalizations/overlay36UpdateInteractiveEntity.fields.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdateInteractiveEntity.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4C4 b83f6f433c27002452e1a5c6669cce5534f72c02018252d638adf990a73d3758 \
		@config/normalizations/overlay36UpdateInteractiveEntity.source_shape.ops \
		@config/normalizations/overlay36UpdateInteractiveEntity.reorder.ops \
		@config/normalizations/overlay36UpdateInteractiveEntity.fields.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnTransient.c.o: \
	config/normalizations/overlay36SpawnTransient.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnTransient.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x11C d6cf5d7eb1322b0c5d0c1c94d6f09df1fbff38575184e5f0112fbdf3502a2b43 \
		@config/normalizations/overlay36SpawnTransient.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36InitVectorState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CheckNearbyHeight.c.o: \
	config/normalizations/overlay36CheckNearbyHeight.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CheckNearbyHeight.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x100 0802a132f7e5a32217137fd215b78bc1e390e44c6b27b419e01d014203bec3b9 \
		'fields:0x6c:op=0x11@0x9,rs=0x4@0x0,rt=0x1@0x1,imm=0x8000@0x0' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xFC 4dfaeeec4835e04d4018bafaea9a7e81d2c89601c3311bee1d6ea36705348ca8 \
		'drop-li:0x6c:at:0:func_overlay_036_F0000818_1883CD0' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xFC && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xFC c457a77ecb3120432d679b38d5b6716a01d75ffa0074997e2c643dcba54c418f \
		@config/normalizations/overlay36CheckNearbyHeight.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36FlushQueue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x90
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36QueueAction.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnFinalEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xc0 d49198645981e674857758aad1bb210e0ffe090aa3dd12dcedae740a3053ea5a \
		fields:0x8:imm=65480@65488 \
		fields:0xb4:imm=56@48 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAtPosition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAndUpdate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnLinked7F.c.o: \
	config/normalizations/overlay36SpawnLinked7F.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnLinked7F.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x194 dc2c921f8d88f8b348b4664e00655f4c83e474a311fc4b352dc518fc5f1a980a \
		@config/normalizations/overlay36SpawnLinked7F.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnDirectional.c.o: \
	config/normalizations/overlay36SpawnDirectional.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnDirectional.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x164 2fa63c297a6014cba90a5066d0c36ebb0a5878226354f84edc5938b90c502f7c \
		@config/normalizations/overlay36SpawnDirectional.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x164
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnOffsetA9.c.o: \
	config/normalizations/overlay36SpawnOffsetA9.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnOffsetA9.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x164 09ca017f94f75e81dc4c72dac149249d429790053d9ae965f04ec033ad0be7d2 \
		@config/normalizations/overlay36SpawnOffsetA9.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x164
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdatePeers.c.o: \
	config/normalizations/overlay36UpdatePeers.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdatePeers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x17C cecde58454b830f4623d7726a9208f5cd2f8ae3d44ab2067933372c62570620e \
		@config/normalizations/overlay36UpdatePeers.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x17C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100InitializeMotion.c.o: \
	config/normalizations/overlay100InitializeMotion.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100InitializeMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100InitializeMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x214 611b65a2b376059ba7f41c4df6c8af825f4b62f1c6ea2021eea956289a85078f \
		@config/normalizations/overlay100InitializeMotion.ops && \
	$(OBJCOPY) --redefine-sym \
		overlay100AllocReloc=func_overlay_100_F0000000_18DAD28 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x110:overlay100RandomReloc:func_overlay_100_F0000000_18DAD28 \
		0x120:overlay100RandomReloc:func_overlay_100_F0000000_18DAD28 \
		0x130:overlay100RandomReloc:func_overlay_100_F0000000_18DAD28 \
		0x150:overlay100InitVelocityReloc:func_overlay_100_F0000000_18DAD28 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x004:5:gOverlay100CountReloc 0x008:6:gOverlay100CountReloc \
		0x0ac:5:gOverlay100VelocityScaleReloc 0x108:6:gOverlay100VelocityScaleReloc \
		0x1c0:5:gOverlay100CountReloc 0x1c4:6:gOverlay100CountReloc \
		0x1cc:5:gOverlay100EntriesReloc 0x1d8:6:gOverlay100EntriesReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x214
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100UpdateMotion.c.o: \
	config/normalizations/overlay100UpdateMotion.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100UpdateMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100UpdateMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x180 f6aea2d767bea5e52a6c8c364127427f8262561ccd590077822c353c885b9b1d \
		@config/normalizations/overlay100UpdateMotion.ops && \
	$(OBJCOPY) --redefine-sym \
		overlay100ReleaseMotionReloc=func_overlay_100_F0000000_18DAD28 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0d4:5:gOverlay100GravityReloc 0x0d8:6:gOverlay100GravityReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x180
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyToValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100DrawMotion.c.o: \
	config/normalizations/overlay100DrawMotion.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100DrawMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100DrawMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3D0 75edd7555913e4cc9340f7469f14bf4f66a9be745398a272cc84308f30edda5d \
		@config/normalizations/overlay100DrawMotion.ops && \
	$(OBJCOPY) --redefine-sym \
		overlay100GetViewReloc=func_overlay_100_F0000000_18DAD28 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x14c:overlay100PrepareAnglesReloc:func_overlay_100_F0000000_18DAD28 \
		0x160:overlay100SinReloc:func_overlay_100_F0000000_18DAD28 \
		0x174:overlay100CosReloc:func_overlay_100_F0000000_18DAD28 \
		0x374:overlay100FinishCommandsReloc:func_overlay_100_F0000000_18DAD28 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x058:5:gOverlay100SegmentReloc 0x060:6:gOverlay100SegmentReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o090/overlay90Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xFC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ResetObjects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ContainsValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x60
# Natural source owns the exact ABI, CFG, memory effects, calls, and relocation
# topology. Remove one proved nop, then select the complete equivalent schedule,
# private allocation web, and local addends through a fail-loud decoded ledger.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay49Initialize.c.o: \
	config/normalizations/overlay49Initialize.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay49Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1f4 75dbf550918557a9ade8802c46458d379d324e23d1eca9b477ed9c902e564a96 \
		@config/normalizations/overlay49Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x008:func_80028F54:overlay49Initialize \
		0x018:D_8007BF08:gOverlay49Modes \
		0x01c:D_8007BF08:gOverlay49Modes \
		0x024:D_8007BF04:gOverlay49Modes \
		0x03c:D_8007BF04:gOverlay49Modes \
		0x02c:D_800D3128:gOverlay49Modes \
		0x060:D_800D3128:gOverlay49Modes \
		0x040:gOverlay49Masks:gOverlay49Modes \
		0x054:gOverlay49Masks:gOverlay49Modes \
		0x044:gOverlay49Shifts:gOverlay49Modes \
		0x058:gOverlay49Shifts:gOverlay49Modes \
		0x064:gOverlay49Result:gOverlay49Modes \
		0x074:gOverlay49Result:gOverlay49Modes \
		0x09c:func_800508B4:overlay49Initialize \
		0x0a4:D_800D3128:gOverlay49Modes \
		0x0a8:D_800D3128:gOverlay49Modes \
		0x0c4:gOverlay49Result:gOverlay49Modes \
		0x0c8:gOverlay49Result:gOverlay49Modes \
		0x19c:func_8002917C:overlay49Initialize \
		0x1c4:gOverlay49FastFinishEnabled:gOverlay49Modes \
		0x1cc:gOverlay49FastFinishEnabled:gOverlay49Modes \
		0x1d0:gOverlay49FastFinishEnabled:gOverlay49Modes \
		0x1d4:gOverlay49FastFinishEnabled:gOverlay49Modes \
		0x1dc:gOverlay49Timer:gOverlay49Modes \
		0x1e0:gOverlay49Timer:gOverlay49Modes \
		0x1e4:gOverlay49Finished:gOverlay49Modes \
		0x1e8:gOverlay49Finished:gOverlay49Modes && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F4
# Natural source owns the exact boundary, ABI, CFG, calls, memory effects, and
# relocation topology. Select the complete equivalent post-decrement loop web,
# then bind runtime-relocated resident sites to stored-zero overlay proxies.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay49Update.c.o: \
	config/normalizations/overlay49Update.ops \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay49Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x160 f799a104ca8595ae0afea6ecc8b0c0e20fdd9c22296f3092df022bdbb70610bf \
		@config/normalizations/overlay49Update.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x024:func_800254FC:overlay65UpdateReloc \
		0x030:func_8002554C:overlay65UpdateReloc \
		0x0cc:func_800016EC:overlay65UpdateReloc \
		0x0d4:D_8007BF08:gOverlay49Timer \
		0x0d8:D_8007BF08:gOverlay49Timer \
		0x0e4:func_8003A754:overlay65UpdateReloc \
		0x0ec:D_8007BF04:gOverlay49Timer \
		0x0f0:D_8007BF04:gOverlay49Timer \
		0x104:overlay48InitializeReloc:overlay65UpdateReloc \
		0x120:func_80028374:overlay65UpdateReloc \
		0x130:D_800D0000:gOverlay49Timer \
		0x134:D_800D0004:gOverlay49Timer \
		0x138:D_800D0004:gOverlay49Timer \
		0x13c:D_800D0000:gOverlay49Timer && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x160
# Natural source supplies the exact boundary, frame, stores, and calls. The
# complete guarded ledger selects retail's equivalent initialization schedule,
# private register web, and proved constant boolean; local fixed-address ELF
# records absent from the shipped runtime table are then removed explicitly.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48InitializeState.c.o: \
	config/normalizations/overlay48InitializeState.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48InitializeState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xe4 312385b63b2beaee48fb7cb069e6737ad4f7a622031d1e50220c157016092ce0 \
		@config/normalizations/overlay48InitializeState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x00:5:gOverlay48HeaderLifetime \
		0x0c:6:gOverlay48HeaderLifetime \
		0x10:5:gOverlay48HeaderActive \
		0x14:6:gOverlay48HeaderActive \
		0x20:5:gOverlay48HeaderSeed \
		0x18:6:gOverlay48HeaderSeed \
		0x04:5:gOverlay48HeaderHandle \
		0x30:6:gOverlay48HeaderHandle \
		0x94:5:gOverlay48Timer \
		0x9c:6:gOverlay48Timer \
		0xa0:5:gOverlay48ScriptIndex \
		0xa4:6:gOverlay48ScriptIndex \
		0xa8:5:gOverlay48Finished \
		0xac:6:gOverlay48Finished \
		0xb8:5:gOverlay48Script \
		0xc0:6:gOverlay48Script && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE4
# Natural codegen is exact except for one complete instruction schedule
# permutation and its five mechanically induced branch displacements.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48UpdateState.c.o: \
	config/normalizations/overlay48UpdateState.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48UpdateState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2C8 b40572043e4e1dec19f3511702b715adc2dfa9a6fd24ef7e7e76663cdbd11051 \
		@config/normalizations/overlay48UpdateState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x60
$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay28ResetBuffer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35SelectHeight.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
# The natural 244-word grid-mask builder has the complete instruction multiset,
# CFG, and memory effects. Seven complete schedule bijections plus the asserted
# private-stack/register web select retail's equivalent frame representation;
# the function has no static or runtime relocations.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35BuildGridMasks.c.o: \
	config/normalizations/overlay35BuildGridMasks.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35BuildGridMasks.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3D0 e085a79a131703629efa70bf02e51c022ad1aaf2aaa3442cda4aec5e5c31c028 \
		@config/normalizations/overlay35BuildGridMasks.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3D0
# The typed source has the exact 120-word topology. Two bijective scheduling
# swaps and the complete private register web select retail's allocation;
# loader-owned global HILO records remain represented in the runtime atlas.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35Initialize.c.o: \
	config/normalizations/overlay35Initialize.ops \
	config/normalizations/overlay35Initialize.filter.spec \
	config/normalizations/overlay35Initialize.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1E0 9736fdb41db206553f861dfa22eb6fba1745488075704ab8d102d6b99b98f91e \
		@config/normalizations/overlay35Initialize.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay35Initialize.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay35Initialize.rebind.spec
# The natural 104-word body owns the exact CFG, calls, memory and FP effects.
# A complete object-identity/constant-one lifetime web selects the equivalent
# retail compiler representation; the local D_B18 pair remains intact.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26InitializeObject.c.o: \
	config/normalizations/overlay26InitializeObject.ops \
	config/normalizations/overlay26InitializeObject.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1A0 8384955420456c11e8d747df14f1e57dae3f85716162cfed69ad3828d6b817ca \
		@config/normalizations/overlay26InitializeObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay26InitializeObject.rebind.spec

# The natural 269-word body owns the exact frame, CFG, FP and memory effects,
# and all 23 calls. This complete private stack/GPR web selects retail's
# equivalent allocation; all calls remain explicit loader-owned symbol roles.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26HandleEffects.c.o: \
	config/normalizations/overlay26HandleEffects.ops \
	config/normalizations/overlay26HandleEffects.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26HandleEffects.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26HandleEffects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x434 5f717915b96969273aa05ff29748f02310f5fe098da30b87311ba6349a2b40fa \
		@config/normalizations/overlay26HandleEffects.ops && \
	$(OBJCOPY) --redefine-sym \
		func_80006EA0=func_overlay_026_F0000000_187A3F8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay26HandleEffects.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x434

# The typed 134-word draw loop owns the exact frame, CFG, calls, FP and memory
# effects. A hash-guarded dead pointer increment is removed before this
# complete effective-address/schedule/allocation web selects retail's form.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26DrawGroups.c.o: \
	config/normalizations/overlay26DrawGroups.prepare.py \
	config/normalizations/overlay26DrawGroups.ops \
	config/normalizations/overlay26DrawGroups.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26DrawGroups.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay26DrawGroups.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x218 e73c06efd288dd232e3122a588a7be1b0f16871c71bdc2709873550748d2756a \
		@config/normalizations/overlay26DrawGroups.ops && \
	$(OBJCOPY) --redefine-sym \
		o26PrepareNode=func_overlay_026_F0000000_187A3F8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay26DrawGroups.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x218

$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29ProjectPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29ProjectPoint.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59PrepareEntry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_059_F0000070_18B87C0=overlay59PrepareEntry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ResetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Advance.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_059_F000036C_18B8ABC=overlay59Advance $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x418
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59BuildList.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59AppendValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Interpolate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59DrawFrame.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
# The natural object has the exact 221-word frame, CFG, opcode schedule, calls,
# and FP payload copy. The guarded file selects retail's equivalent private
# integer allocation for the two easing loops; it contains decoded fields only.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57EaseAndLatch.c.o: \
	config/normalizations/overlay57EaseAndLatch.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57EaseAndLatch.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x374 aa3c82d78e296bccaebbf7d56add3b8c0e213b422d2b0c66bffaea37100ae86d \
		@config/normalizations/overlay57EaseAndLatch.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x374
# The natural object has the exact 200-word frame, CFG, opcode/control-flow
# schedule, all calls and all FP lanes.  This guarded complete-field selection
# chooses retail's equivalent integer allocation and paired x/y halfword fields
# for the two private smoothing loops; the operation file contains decoded
# fields only, never raw words.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SmoothAndCheckDistance.c.o: \
	config/normalizations/overlay57SmoothAndCheckDistance.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SmoothAndCheckDistance.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x320 5c55f3c99fe92cb4944d95ef07ca93e8e726339cdb8aa321eac8e5e6427d03d2 \
		@config/normalizations/overlay57SmoothAndCheckDistance.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x320
# The natural object has all 208 words, calls, branches, FP lanes, stack homes,
# and memory effects. Two complete independent schedules and guarded private
# register/frame webs select retail's equivalent representation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57Draw32A0.c.o: \
	config/normalizations/overlay57Draw32A0.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57Draw32A0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x340 8696256f3733ef4a7822b3df6e3f5a744c27c7f8467ced6286dbd4915cb8c3e0 \
		@config/normalizations/overlay57Draw32A0.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x340
# IDO emits the complete exact-size selection routine. The guarded decoded
# transform owns its relocation-carrying schedules and complete private webs.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateSelection.c.o: \
	config/normalizations/overlay57UpdateSelection.ops \
	tools/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateSelection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x46C 5300e008a4c83f974324e25d64970434ff30bfed3142148dc9148ee8348e41be \
		@config/normalizations/overlay57UpdateSelection.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x148:gO57SelectionPrimary4E8Reloc:gOverlay57Flags37ECReloc \
		0x14C:gO57SelectionPrimary4E8Reloc:gOverlay57Flags37ECReloc \
		0x430:gO57SelectionCurrent100Reloc:gOverlay57Flags37ECReloc \
		0x440:gO57SelectionCurrent100Reloc:gOverlay57Flags37ECReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x46C
# Natural codegen owns the exact 354-word CFG, all 32 calls, all 45 address
# pairs, and every immediate/schedule choice. This fail-loud ledger selects
# only the shipped complete private GPR-allocation web.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeState.c.o: \
	config/normalizations/overlay57UpdateModeState.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x588 8d9c7845b2927118ffc8bb78fbe1e8d15fe298a892abd275391625be346876d9 \
		@config/normalizations/overlay57UpdateModeState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x588
# The natural object has the exact 217-word CFG, calls, delay slots, FP lanes,
# and frame. A guarded three-word schedule and private caller-register web
# select retail's equivalent representation.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57HandleModeInput.c.o: \
	config/normalizations/overlay57HandleModeInput.ops
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57HandleModeInput.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x364 230c1b927f0fe5b124bcfc07d2fc3764d86f0ff6fcd039c44afade0bb55f70c0 \
		@config/normalizations/overlay57HandleModeInput.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x364
# Natural source owns the exact-size CFG, all seven calls, delay slots, and
# effects. The guarded complete setup schedule and private index/register web
# select retail's equivalent representation. Three independently proved LOCAL
# records restore the compiler-elided base carriers, including two explicit SW
# addends; the relocation helper checks their opcodes, addends, and final hash.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeTrigger.c.o: \
	config/normalizations/overlay57UpdateModeTrigger.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/add_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeTrigger.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x178 5a0c128145749be3068ebebd0810e767b1a4d03a0636fb8b658279c63db2ff66 \
		@config/normalizations/overlay57UpdateModeTrigger.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		0x178 5a0c128145749be3068ebebd0810e767b1a4d03a0636fb8b658279c63db2ff66 \
		0x5C:HI16:gOverlay57SetupValues \
		0x74:LO16:gOverlay57SetupValues:0x16C \
		0x7C:LO16:gOverlay57SetupValues:0x168 && \
	$(OBJCOPY) \
		--redefine-sym gOverlay57Countdown=gOverlay57TailCountdownReloc \
		--redefine-sym gOverlay57ModeFlag=gOverlay57TailModeFlagReloc \
		--redefine-sym gOverlay57SetupStatus=gOverlay57TailSetupStatusReloc \
		--redefine-sym gOverlay57SetupValues=gOverlay57TailSetupValuesReloc \
		--redefine-sym gOverlay57Timer=gOverlay57TailTimerReloc \
		--redefine-sym gOverlay57Object=gOverlay57TailObjectReloc \
		--redefine-sym gOverlay57ObjectStatus=gOverlay57TailObjectStatusReloc \
		--redefine-sym gOverlay57ObjectId=gOverlay57TailObjectIdReloc \
		--redefine-sym gOverlay57TriggerLatched=gOverlay57TailTriggerLatchedReloc \
		--redefine-sym gOverlay57SetupDelay=gOverlay57TailSetupDelayReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x178
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57CheckDistance.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x100
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateNode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x100
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyTable.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138

$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Interpolate.c.o: CFLAGS += -Wab,-r4300_mul

OVERLAY_TRIMMED_OBJECTS := \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o006/overlay_006.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o076/overlay_076.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o078/overlay_078.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o102/overlay_102.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o103/overlay_103.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o106/overlay_106.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o107/osRamTest4_6105.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1WrapOffset.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SignedOffset.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindNextAngle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindPreviousAngle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1RefreshMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindBestRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ReleaseRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7EntryPool.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7FillValues.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7CreateEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7AppendEntry.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7DispatchModes.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7UpdateOwnerMode.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7DispatchSelection.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7CommitSelection.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay7InitPool.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectTarget.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3RunCachedModeAction.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3FindClosestObject.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectScoredObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3TouchObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3UpdateTimedEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19Dispatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildOutput.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildPlanes.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildAdjacency.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19FindAdjacent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19ClassifyEdge.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildSpatialMasks.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4GroupCount.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4InitializeObjectMotion.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4FindCategory2Object.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4FindSearchPosition.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateObjectMotion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4UpdateGroupSpacing.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4AttachObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay4RemoveObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitMotion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8Ignore.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8GetIndexed.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8StartMotion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8Activate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8UpdateChild.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8UpdateChannels.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8ApplyColors.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8ScaleOutputs.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8SetBuffer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8WriteCommand.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8SetValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay8UpdateMotionOutput.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateObjectState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateAngle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateOutput.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateInputState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9IntegrateVelocity.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9ResolveHeight.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9Ignore.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay9UpdateMotion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchBank.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchInst.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchSound.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchWaveTable.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay5InitializeAudio.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay5CreatePlayer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0000000_186F8D8.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F000013C_186FA14.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14Reset.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReturnOne.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReturnOneCallbacks.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReleaseOwner.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14FinalizeActiveHandle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14DispatchCommand.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14CallUpdate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14PrepareInputState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14AdvanceCommand.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14StepCommand.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ApplyValues.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14MoveCommandCursor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14CreateValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14LoadRelocatedValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ResetMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00009F4_18702CC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14BuildRects.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00013F4_1870CCC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001540_1870E18.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001830_1871108.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CallGlobal.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CallGlobal.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36InitObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27CanUse.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27Activate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AdvanceStepRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateColorRecords.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SampleCurve.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41InterpolateAngle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateCurveObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41IsUnitScale.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateProgress.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41ProcessEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AddSlot.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SpawnItems.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41EnqueueTransition.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41TickTransitions.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41DrawItem.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53ReleaseResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54ReleaseResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29BuildChain.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29UpdateRatio.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29Sample.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29InitializeObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29DrawGroups.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o030/overlay30Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o030/overlay30TransposePixels.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay51Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay5InitSequence.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ConsumeTimer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1TestDirection.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BuildPointRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ResetFlags.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagC4.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagC8.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReleaseCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15GetResource4.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15InitStarsAndPalette.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15MoveStars.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawScreenStars.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15InitStars.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15UpdateMovingStars.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15GetResource10.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15SetValueC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15ClearValue7C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15DrawRain.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SetValue10.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34InitStorage.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34InterpolateColor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34CreateRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34ResetStorage.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34UpdateRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41Ignore.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66GetCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66Select.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66SmoothAndDraw.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61RecordSize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79SetLink.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79InitState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79UpdateTimers.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79FindNearby.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000000_18CCFA0.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0001290_18CE230.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84CopyPair.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84AdvanceCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84LoadCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SetBit.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetValues.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ActivateCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ClearActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ClearMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SetAngle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84Mark.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SelectCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ProcessCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ScaledVectorPosition.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86BuildTransform.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86SelectPosition.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94UpdateController.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94SetValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101AllocateEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Reset.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101FindEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry12.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ActivateSlot.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101AdvanceSlot.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte17.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte16.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8B.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateFloat12.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateDelta16.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte18.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateGlobalPair.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateColor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildIntensityColors.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildBorder.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawClock.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildFrame.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SetScissor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Cleanup.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdatePresentation.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawChain.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateChains.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawSlots.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101PromoteSlot.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair2.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedFloat.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedByte.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair3.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedScaled.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedColor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateFrames.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleFrames.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleGlobalPair.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchEvents.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawElement.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawTransformed.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationA.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationB.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationD.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailA6BC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailAB4C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailB544.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailBA34.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC144.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC6E8.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101GetBounds.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16BuildGradient.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16InitializeBuffer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16ReleaseBuffer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay16ApplyGradient.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SchedulePair.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SchedulePair12.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleByte17.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleByte16.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21RegisterPlane.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21ApplyPriorities.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23SpawnAttachments.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23RenderEffect.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay24Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay24Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay24RenderState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25InitializeEffect.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25UpdateEffect.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay25SetVectorFlags.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay27UpdateEffectState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56SplitTime.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56UnpackColor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56AdjustCoordinates.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56SetMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56LoadResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay56ReleaseResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Update.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Render.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37RecordMinimum.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37RecordActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay39Write.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay_039_tail.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40AddEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40RemoveEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40UpdateEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40BuildFrame.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40DrawEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40SetValues.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40Interpolate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40DrawTintRectangle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40FadeRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Resume.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42DrawCapturedBuffer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay42Present.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43InitializeState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FlushPending.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ReleaseResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ComputeMotion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43AllocateResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43SubmitChildren.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FilterImage.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44CreateAnimationState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44ReleaseHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44UpdateFrameCache.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69UpdateAnchor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69DrawSortedGeometry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o067/overlay67BuildVertices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000000_18C9B20.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000278_18C9D98.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o071/overlay71UpdateCoordinates.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000870_18CA390.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o072/overlay72Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o072/overlay72Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay73Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay73Draw.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay74Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay74Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75UpdateMovingObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75MarkSlot.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay77Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay77Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay_077_tail.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay81CheckNearby.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay_081_leafs.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay82Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay82Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay82Accessors.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88UpdateAnchor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88DrawSortedGeometry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateEffect.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89Evaluate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89InitializeEffect.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateStateAndParticles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay92Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay92FindNearestCourse.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o093/overlay_093.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o095/overlay95NoOp.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o095/overlay95Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitRadius.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitBounds.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96Register.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96Unregister.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96BuildVolume.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96FindVolume.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96TestBit.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96DrawObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CollectUniqueY.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CollectAccepted.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98RenderReflections.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CheckObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97CopyAngles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitTransform.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitSelection.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitPlane.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97CreateDescriptor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97AssignState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitDirection.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitScale.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100RemoveEntry.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100InitializeMotion.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitRange.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CloneRecord.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateValueCache.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AppendPathPoint.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CreateRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvancePath.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34RemoveRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34CreateRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateTransient.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87InitializeObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AllocateRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SearchNearby.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SelectMaskedMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateCountdown.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ResetState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ReadPair.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45CreateDescriptor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ReleaseDescriptor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45ConfigureLayout.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45RandomizeOffsets.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45SetMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45SetField22.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay45SetField20.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47ReleaseResources.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47SpawnObject.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61UpdateInput.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61InitResources.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61AddEntry.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawEntry.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawList.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReleaseResources.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0001648_18C0A10.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61WriteCharacter.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReadCharacter.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ResetCounters.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PayloadLimit.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay85Configure.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay85Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1PointerWrap.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetEntryIndex.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindType5ByKey.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindPreviousUsable.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1PreviousIndex.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1NextIndex.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1Noop.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CallReset.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ReturnZero.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChoosePath.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1LoadBuildRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SubmitGlobals.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SubmitAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1RelativeAngles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1TransitionState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateObjectPhysics.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AngleBetweenSamples.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1DistanceFromCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ScaledDistance.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1DistanceFromSelected.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetLinkedActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1GetRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitTimedState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1CopyBytes.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ModeChecks.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateModeSound.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ReadSelection.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagCC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15ReleaseResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseTree.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseHandle.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureResource.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateObjectResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20BuildTileCommands.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20RemoveEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20MarkNested.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20AdvanceEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20CreateEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20DrawResource.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildLookupTables.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeParticleAssets.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildPalettes.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeBuffers.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateConfig.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreatePool.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33InitializeBuffers.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33ReleaseGlobal.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallA.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33BuildDisplayList.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33PresentAndSwap.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallB.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CallModes.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46ReleaseState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46Submit.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeBuffers.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65UpdateParticles.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65ResetSlots.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay64GenerateTexture.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F0000000_1885D10.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay38UpdateParticles.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F000047C_188618C.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000000_18C91C8.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ByteLength.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeAndUpdate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84IsUnitScale.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetEnabledCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ResetCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84UpdateResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84RefreshCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SelectCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay15ReleaseResource10.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateEffects.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateSequence.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreateEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleasePrimary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseSecondary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreatePayload.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68AttachObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateTrail.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ClearNestedFlag.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68FinishEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68StartTimer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PromoteSecondary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68InitializeObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateAnimation.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68DrawSortedEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseTertiary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CheckKind.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ValidateRegion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ContainsPoint.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2Enable.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2CopyColor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AppendLine.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClassifyBoundary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2IntersectBoundary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClipLines.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AdjacentIndices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2QueryNode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawBorder.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawLine.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReassignChoiceSlots.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14DispatchCommand.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13CreateRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11EnableHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11DisableHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateMenu.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0001E4C_186A694.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F00022E8_186AB30.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11CreateHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseCurrentGroup.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Call.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Submit.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildLine.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawLines.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildBatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83SubmitAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawEntries.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawStrip.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Dispatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99GetEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ReleaseEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99InitializeEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ProjectVector.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ApplySegment.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99BuildHeightGrid.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSortedEntries.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSegments.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReleaseResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateInterface.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57BeginMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57StartMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57InitializeMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57EaseAndLatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SmoothAndCheckDistance.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57Draw32A0.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58EnsureResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58RefreshRankSet.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawSegmentStrip.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawPointQuad.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawLargePointQuad.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58FinalizePackedStatus.c.o
OVERLAY_TRIMMED_OBJECTS += \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17CalculateEndpoints.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17ReleaseChain.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17AdvanceChain.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17DrawStrip.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Load.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Reconfigure.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18InitializeBuffers.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58ReleaseResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87ReleaseCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87HasNearby.c.o
OVERLAY_TRIMMED_OBJECTS += \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Initialize.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50PatchIndices.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Cleanup.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50SubmitTimeGlyphs.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay51PatchIndices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52PatchIndices.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52CopyOffsetEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Cleanup.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53PatchIndices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53CopyOffsetEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54PatchIndices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55PatchIndices.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay91Init.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay91UpdateTimeline.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay91Render.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixA.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixB.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeA.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeB.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SetNodeValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58SetNodeValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay51ReleaseState.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54CopyOffsetRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54GetOffsets.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55CopyOffsetRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55GetOffsets.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup4.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3A.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6A.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6B.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3B.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateForward.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateBackward.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29Select.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36TickState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36PrepareAndTick.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SelectState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36ChooseWeightedState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdateInteractiveEntity.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnTransient.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36InitVectorState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CheckNearbyHeight.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36FlushQueue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36QueueAction.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnFinalEffect.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAtPosition.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAndUpdate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnLinked7F.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnDirectional.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnOffsetA9.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdatePeers.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100UpdateMotion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyToValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100DrawMotion.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o090/overlay90Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ResetObjects.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ContainsValue.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48InitializeState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48UpdateState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48ReleaseAll.c.o
OVERLAY_TRIMMED_OBJECTS += \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay49Initialize.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay49Update.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay28ResetBuffer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay28UpdateVertices.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35SelectHeight.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35BuildGridMasks.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59PrepareEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ResetEntries.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Advance.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59AppendValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Interpolate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59BuildList.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59DrawFrame.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57CheckDistance.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateTransition.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateNode.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyTable.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14UpdateTransition.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeTrigger.c.o

$(OVERLAY_TRIMMED_OBJECTS): $(TOOLS_DIR)/trim_elf_section.py

$(TARGET).elf: $(O_FILES) $(LD_SCRIPT) overlay_undefined_syms.$(VERSION).txt | $(ALL_DIRS) $(SPLAT_STAMP)
	$(LD) $(LDFLAGS) -o $@

# symbol_addrs is a real input to the split, not just documentation: naming a
# function there changes the labels in the generated .s and, for `c`
# subsegments, the *filenames* under asm/nonmatchings/ that the source's
# GLOBAL_ASM pragmas point at. Leaving it off this list meant adding a symbol
# name did not re-split, and the build failed with a missing nonmatchings file.
#
# Order-only prereq on $(PYTHON) so the split never runs against a nonexistent
# venv; `setup` is what actually installs splat into it.
$(SPLAT_STAMP): $(BASENAME).$(VERSION).yaml symbol_addrs.$(VERSION).txt \
                config/overlays.$(VERSION).json $(TOOLS_DIR)/overlay_atlas.py \
                $(TOOLS_DIR)/overlay_tables.py requirements.txt | $(ALL_DIRS) $(PYTHON)
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@$(MAKE) --no-print-directory prune-asm
	@touch $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

# n64crc is expected to be a no-op: every byte, CRC words included, came out of
# the baserom via splat. If it rewrites the header the build is wrong upstream,
# so say so loudly instead of silently "fixing" it.
$(TARGET).z64: $(TARGET).bin $(CRC)
	@cp $(TARGET).bin $@
	@before=$$($(SHA1) $@ | cut -d' ' -f1); \
	$(CRC) $@ >/dev/null; \
	after=$$($(SHA1) $@ | cut -d' ' -f1); \
	if [ "$$before" != "$$after" ]; then \
		echo "ERROR: n64crc rewrote the CRC words ($$before -> $$after);"; \
		echo "       the linked image did not reproduce the ROM's own checksums."; \
		echo "       This means the build is wrong upstream -- failing loudly"; \
		echo "       instead of silently shipping a 'fixed' ROM."; \
		exit 1; \
	else \
		echo "n64crc: checksums already correct (no-op)"; \
	fi
	@ls -l $@

.PHONY: default all setup hooks extract prune-asm verify cleanroom audit-decoders overlay-tables overlay-atlas overlay-atlas-write overlay-donors overlay-donors-write overlay-donors-scan-check check-fixtures check-docs reference-builds check-reference-builds progress scoreboard check-scoreboard clean distclean
.SECONDARY:
SHELL = /bin/bash -e -o pipefail
