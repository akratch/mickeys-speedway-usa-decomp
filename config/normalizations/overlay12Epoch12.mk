O12_SPAWN_PARTICLE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12SpawnParticle.c.o

O12_SHUTDOWN_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12Shutdown.c.o

$(O12_SHUTDOWN_OBJ): CFLAGS += -Wo,-loopunroll,0
$(O12_SPAWN_PARTICLE_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4

O12_SPAWN_EFFECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay12SpawnEffect.c.o
