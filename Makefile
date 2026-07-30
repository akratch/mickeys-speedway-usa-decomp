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
ASM_DIRS  := asm asm/data
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

# IDO 5.3, unused until the first C file lands (Phase 1) but kept configured.
CC      := $(TOOLS_DIR)/ido/cc
OPT_FLAGS := -O2
MIPSISET  := -mips1 -32
DEFINES   := -D_LANGUAGE_C -D_FINALROM -DTARGET_N64 -DVERSION_$(VERSION)
INCLUDE_CFLAGS := -I . -I include -I include/libc -I include/PR -I include/sys -I assets
CFLAGS  := -non_shared -G 0 -Xcpluscomm -fullwarn -woff 649,838 -nostdinc \
           $(DEFINES) $(INCLUDE_CFLAGS)

CRC := $(TOOLS_DIR)/n64crc

# ---------------------------------------------------------------------------
# Flags
# ---------------------------------------------------------------------------

# Verified working assembler invocation (Task 4).
ASFLAGS := -march=vr4300 -32 -mabi=32 -G0 -I include

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

O_FILES := $(foreach f,$(S_FILES),$(BUILD_DIR)/$(f).o) \
           $(foreach f,$(BIN_FILES),$(BUILD_DIR)/$(f).o)

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(ASM_DIRS) $(BIN_DIRS))

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

$(TARGET).elf: $(O_FILES) $(LD_SCRIPT) | $(ALL_DIRS) $(SPLAT_STAMP)
	$(LD) $(LDFLAGS) -o $@

# Order-only prereq on $(PYTHON) so the split never runs against a nonexistent
# venv; `setup` is what actually installs splat into it.
$(SPLAT_STAMP): $(BASENAME).$(VERSION).yaml requirements.txt | $(ALL_DIRS) $(PYTHON)
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

.PHONY: default all setup extract verify cleanroom clean distclean
.SECONDARY:
SHELL = /bin/bash -e -o pipefail
