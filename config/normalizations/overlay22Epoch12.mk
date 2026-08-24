O22_REMOVE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22RemoveObject.c.o
O22_RESOLVE_PLANE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22ResolvePlane.c.o
O22_INITIALIZE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22InitializeObject.c.o

# Natural IDO owns the exact instruction count/opcode population, FP lanes,
# semantic relocation surface, CFG and effects. Select the complete equivalent
# private frame/home/GPR carrier and bind only the nine stored-zero call symbols.
$(O22_INITIALIZE_OBJECT_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_INITIALIZE_OBJECT_OBJ): \
	config/normalizations/overlay22Epoch12.mk \
	config/normalizations/overlay22InitializeObject.ops \
	config/normalizations/overlay22InitializeObject.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(O22_INITIALIZE_OBJECT_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2B0 6d3acc702fbde78123cee98513d881636a133d0c8ac4c845d70e0611827309b5 \
		@config/normalizations/overlay22InitializeObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay22InitializeObject.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B0

$(O22_REMOVE_OBJECT_OBJ): \
	config/normalizations/overlay22Epoch12.mk \
	config/normalizations/overlay22RemoveObject.ops \
	config/normalizations/overlay22RemoveObject.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py

# Natural IDO owns the exact frame, CFG, opcode/immediate/memory/call schedule,
# and boundary. Select the complete equivalent private GPR allocation web,
# remove only loader-owned local address pairs, and leave boundary padding in
# its separately accounted atlas owner.
$(O22_REMOVE_OBJECT_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x16C 2c9bed1cab4a3c91d48c7ab85f3caa5cdf342e62e74bf6fe77c99233ee17a2d6 \
		@config/normalizations/overlay22RemoveObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay22RemoveObject.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x16C

# Natural IDO owns the exact boundary, call/branch topology, opcode stream and
# memory effects. Select the complete equivalent private frame/FPR web and one
# proved three-instruction schedule, then remove only the loader-owned local
# HILO carrier before trimming compiler section alignment.
$(O22_RESOLVE_PLANE_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_RESOLVE_PLANE_OBJ): \
	config/normalizations/overlay22Epoch12.mk \
	config/normalizations/overlay22ResolvePlane.ops \
	config/normalizations/overlay22ResolvePlane.filter.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(O22_RESOLVE_PLANE_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2B4 1b340c61ca782a33805be1af28587c871bfbee81f2dda00eff73ed21348a1890 \
		@config/normalizations/overlay22ResolvePlane.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay22ResolvePlane.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B4
