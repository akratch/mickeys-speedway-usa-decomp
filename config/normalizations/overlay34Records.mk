$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34ResetStorage.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym overlay34RemoveRecord=overlay34RemoveRecordReloc \
		--redefine-sym func_8002B768=overlay34FreeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x94

# NON_MATCHING fallback assembly exports the generated symbol; restore the
# source-facing alias expected by the generated overlay linker assignments,
# then discard only IDO's trailing section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34UpdateRecords.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34UpdateRecords.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_034_F000040C_18815B4=overlay34UpdateRecords $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134

$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SortAndDraw.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_034_F0000608_18817B0=overlay34SortAndDraw $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F8
