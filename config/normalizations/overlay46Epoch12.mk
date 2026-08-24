O46_UPDATE_SEQUENCE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateSequence.c.o

$(O46_UPDATE_SEQUENCE_OBJ): \
	config/normalizations/overlay46UpdateSequence.ops \
	config/normalizations/overlay46UpdateSequence.local_addends.ops \
	config/normalizations/overlay46UpdateSequence.filter.spec \
	config/normalizations/overlay46UpdateSequence.rebind.spec \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py

# The natural source owns the exact 0x4F4 boundary, frame, CFG, opcode,
# immediate, memory, call, and delay-slot schedules. Select retail's complete
# private register web, materialize the runtime-owned local addends, retain the
# 56 exact static carriers, and discard only compiler section artifacts.
$(O46_UPDATE_SEQUENCE_OBJ): POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym gOverlay46Timer5C=D_5C \
		--redefine-sym gOverlay46Group54Render=D_54 \
		--redefine-sym \
			func_80037664=func_overlay_046_F0000000_188E3F8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4F4 876c2302b9141f3e36d9942fce2df63a90cb01457f465f6294e02c75d063076c \
		@config/normalizations/overlay46UpdateSequence.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x4F4 dfdada3fec6b8c11c7c4ee2d53b4c2b314cbb35ef03dc9d9dde077bbd6ba9948 \
		@config/normalizations/overlay46UpdateSequence.local_addends.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay46UpdateSequence.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay46UpdateSequence.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4F4 \
		000000000000000000000000 && \
	$(OBJCOPY) --remove-section=.rodata --remove-section=.rel.rodata $@
