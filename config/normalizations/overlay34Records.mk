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

$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SortAndDraw.c.o: \
	config/normalizations/overlay34SortAndDraw.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SortAndDraw.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2F8 d477cc8d3a6fb0208dea06a124ec8041fba9e6864ad4b04129cf57127ab5f652 \
		@config/normalizations/overlay34SortAndDraw.ops && \
	$(OBJCOPY) --remove-section=.mdebug \
		--redefine-sym func_80024938=overlay34DepthReloc \
		--redefine-sym overlay34InterpolateColor=overlay34InterpolateReloc \
		--redefine-sym func_800084C4=overlay34RenderReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F8
