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

BUILD_DIR := build
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
BIN_DIRS  := assets
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
INCLUDE_CFLAGS := -I . -I include -I include/libc -I include/PR -I include/sys -I assets
CFLAGS  := -non_shared -G 0 -Xcpluscomm -fullwarn -woff 649,838 -nostdinc \
           $(DEFINES) $(INCLUDE_CFLAGS)

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
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@mkdir -p $(BUILD_DIR)
	@touch $(SPLAT_STAMP)

# Unconditional re-split, e.g. after hand-editing yaml and wanting the result
# immediately without going through the dependency graph. Keeps the stamp in
# sync so a following `gmake` doesn't redundantly split again.
extract:
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@mkdir -p $(BUILD_DIR)
	@touch $(SPLAT_STAMP)

verify:
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
#   gmake reference-builds REFS_ARGS="--root DIR --jobs 8"
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
# reason to maintain two recipes. The .s files the pragmas name are regenerated
# by splat into asm/nonmatchings/, hence the $(SPLAT_STAMP) order-only prereq
# (same caveat as the .s rule above: `all`/`verify`'s two-phase make is what
# actually guarantees freshness).
$(BUILD_DIR)/%.c.o: %.c $(H_FILES) | $(ALL_DIRS) $(SPLAT_STAMP)
	$(ASM_PROCESSOR) $(CC) -- $(AS) $(ASM_PROC_ASFLAGS) -- \
		-c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<

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
LIBULTRA_O1_TUS := ai aigetlen dp dpsetstat getactivequeue si sp \
                   spgetstat spsetstat sptaskyield
$(foreach f,$(LIBULTRA_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O1))
$(foreach f,$(LIBULTRA_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))

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
#   NOT MEASURED: `-O2` rather than `-O1`. Those two are byte-identical on both
#   of these translation units, so these bytes do not discriminate the
#   optimisation level at all. -O2 is used because Jet Force Gemini's published
#   Makefile builds its libultra io/ TUs that way -- it is BORROWED, not
#   established, and a future TU in this group may well settle it the other way.
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
# Scoped to the two TUs actually measured. The other fourteen libultra TUs
# matched in Task 3 are still `asm`; each is a candidate for the same treatment
# and none should be moved into this list without its own measurement.
LIBULTRA_O2_G3_TUS := contpfs epidma epilinkhandle epirawdma epirawread \
                      epirawwrite epiread epiwrite pfsallocatefile pfschecker \
                      pfsfilestate pfsfreeblocks pfsgetstatus pfsinit \
                      pfsisplug pfsnumfiles pfsreadwritefile pfssearchfile \
                      pfsselectbank piacs pidma pigetcmdq pirawdma
$(foreach f,$(LIBULTRA_O2_G3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O2 -g3))
$(foreach f,$(LIBULTRA_O2_G3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))

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

$(TARGET).elf: $(O_FILES) $(LD_SCRIPT) | $(ALL_DIRS) $(SPLAT_STAMP)
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
                requirements.txt | $(ALL_DIRS) $(PYTHON)
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
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

.PHONY: default all setup hooks extract verify cleanroom audit-decoders overlay-tables check-fixtures check-docs reference-builds check-reference-builds progress scoreboard check-scoreboard clean distclean
.SECONDARY:
SHELL = /bin/bash -e -o pipefail
