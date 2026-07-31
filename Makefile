# Mickey's Speedway USA (US) — clean-room decompilation build
#
# Phase 0: the ROM is rebuilt entirely from splat's disassembly + extracted
# binaries. No C is compiled yet; the IDO variables below are kept wired up so
# that later phases only have to add source files, not re-derive the toolchain.
#
#   gmake            build/mickey.us.z64
#   gmake verify     build + SHA1 compare against the baserom hash
#   gmake setup      venv + toolchain + baserom check + splat extraction
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

setup: $(PYTHON)
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

cleanroom:
	bash $(TOOLS_DIR)/cleanroom_check.sh

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

.PHONY: default all setup extract verify cleanroom check-docs progress clean distclean
.SECONDARY:
SHELL = /bin/bash -e -o pipefail
