O12_SPAWN_PARTICLE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12SpawnParticle.c.o

O12_SHUTDOWN_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12Shutdown.c.o

$(O12_SHUTDOWN_OBJ): \
	config/normalizations/overlay12Epoch12.mk \
	config/normalizations/overlay12Shutdown.filter.spec \
	config/normalizations/overlay12Shutdown.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py

# Natural source owns the exact boundary, frame, CFG, calls, delay slots, and
# effects. The loader owns the local BSS addends; after removing those guarded
# semantic relocations, select retail's equivalent complete private loop test.
$(O12_SHUTDOWN_OBJ): CFLAGS += -Wo,-loopunroll,0
$(O12_SHUTDOWN_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay12Shutdown.filter.spec && \
	$(OBJCOPY) --redefine-sym gOverlay12Entries=D_20 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xF0 07791769f8f86987a6ee23437f65a85547c38f56b2928634b1f935e34a5bc4d8 \
		@config/normalizations/overlay12Shutdown.ops

$(O12_SPAWN_PARTICLE_OBJ): \
	config/normalizations/overlay12SpawnParticle.ops \
	config/normalizations/overlay12SpawnParticle.filter.spec \
	config/normalizations/overlay12SpawnParticle.prepare.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py

# Natural source owns the shipped O32 ABI, frame, CFG, loop, stores, and call.
# Remove one proved redundant final count-address materialization, translate
# only its guarded branch/relocation topology, then select the equivalent
# surviving base field and runtime-owned ready-state addend.
$(O12_SPAWN_PARTICLE_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay12SpawnParticle.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym \
			overlay12Initialize=func_overlay_012_F0000000_186D280 \
		--redefine-sym gOverlay12ParticleCount=D_4 \
		--redefine-sym gOverlay12Particles=D_1520 $@ && \
	$(HOST_PYTHON) \
		config/normalizations/overlay12SpawnParticle.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4 \
		000000000000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xC4 2e244a8b0d2a63bb65dfd98f5eb184e2026a62a19dd0ad592b643c1d737a35dc \
		@config/normalizations/overlay12SpawnParticle.ops

O12_SPAWN_EFFECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12SpawnEffect.c.o

$(O12_SPAWN_EFFECT_OBJ): \
	config/normalizations/overlay12SpawnEffect.filter.spec \
	config/normalizations/overlay12SpawnEffect.ops \
	config/normalizations/overlay12SpawnEffect.prepare.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py

# Natural source owns the exact boundary, frame, CFG, call and memory-effect
# inventory. Select the shipped equivalent order for the independent record
# initializer, the four private FPR fields, and the complete three-site spill
# home, then bind only identities already represented by the runtime loader.
$(O12_SPAWN_EFFECT_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay12SpawnEffect.filter.spec && \
	$(OBJCOPY) --redefine-sym \
		overlay12Initialize=func_overlay_012_F0000000_186D280 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xd0:overlay12Lookup:func_overlay_012_F0000000_186D280 \
		0xfc:overlay12Lookup:func_overlay_012_F0000000_186D280 && \
	$(OBJCOPY) --redefine-sym gOverlay12Effects=D_20 $@ && \
	$(HOST_PYTHON) config/normalizations/overlay12SpawnEffect.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x130 a5533ed396d9767d5ae8515a5239cf2c4e06d5e70755f350018a8625800d4a27 \
		@config/normalizations/overlay12SpawnEffect.ops
