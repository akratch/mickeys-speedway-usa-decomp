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

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

default: all

all: $(TARGET).z64

setup: $(PYTHON)
	$(PYTHON) -m pip install -q -r requirements.txt
	$(TOOLS_DIR)/setup_toolchain.sh
	$(TOOLS_DIR)/verify_baseroms.sh
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml

extract:
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml

verify: $(TARGET).z64
	@got=$$($(SHA1) $(TARGET).z64 | cut -d' ' -f1); \
	echo "expected $(EXPECTED_SHA1)"; \
	echo "built    $$got"; \
	if [ "$$got" = "$(EXPECTED_SHA1)" ]; then \
		echo "OK  $(TARGET).z64 matches the baserom"; \
	else \
		echo "FAIL $(TARGET).z64 does not match the baserom"; \
		exit 1; \
	fi

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

$(BUILD_DIR)/%.s.o: %.s | $(ALL_DIRS)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.bin.o: %.bin | $(ALL_DIRS)
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips $< $@

$(TARGET).elf: $(O_FILES) $(LD_SCRIPT) | $(ALL_DIRS)
	$(LD) $(LDFLAGS) -o $@

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
		echo "WARNING: n64crc rewrote the CRC words ($$before -> $$after);"; \
		echo "         the linked image did not reproduce the ROM's own checksums."; \
	else \
		echo "n64crc: checksums already correct (no-op)"; \
	fi
	@ls -l $@

.PHONY: default all setup extract verify clean distclean
.SECONDARY:
SHELL = /bin/bash -e -o pipefail
