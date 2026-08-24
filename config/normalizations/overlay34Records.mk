$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34ResetStorage.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym overlay34RemoveRecord=overlay34RemoveRecordReloc \
		--redefine-sym func_8002B768=overlay34FreeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x94

$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34UpdateRecords.c.o: \
	config/normalizations/overlay34UpdateRecords.prepare.py \
	config/normalizations/overlay34UpdateRecords.ops \
	config/normalizations/overlay34UpdateRecords.postprocess.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34UpdateRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay34UpdateRecords.postprocess.py $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SortAndDraw.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_034_F0000608_18817B0=overlay34SortAndDraw $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F8
