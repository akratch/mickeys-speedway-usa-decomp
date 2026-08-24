O46_UPDATE_SEQUENCE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateSequence.c.o

$(O46_UPDATE_SEQUENCE_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4F4
