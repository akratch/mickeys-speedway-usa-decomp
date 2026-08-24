O28_UPDATE_VERTICES_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay28UpdateVertices.c.o

$(O28_UPDATE_VERTICES_OBJ): $(TOOLS_DIR)/trim_elf_section.py

# The natural function owns every executable word, frame/register choice,
# branch/delay slot, FP association, call identity, and runtime relocation.
# Remove only IDO's unowned one-function section-alignment padding.
$(O28_UPDATE_VERTICES_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x148

O28_INITIALIZE_WORK_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay28InitializeWork.c.o

$(O28_INITIALIZE_WORK_OBJ): \
	config/normalizations/overlay28InitializeWork.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py

# The callback HILO is owned by O28's shipped runtime relocation table and is
# stored as zero before loading. Preserve the three natural call relocations
# and remove only that proved loader-owned carrier pair from the static link.
$(O28_INITIALIZE_WORK_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay28InitializeWork.filter.spec

O28_UPDATE_WORK_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay28UpdateWork.c.o

$(O28_UPDATE_WORK_OBJ): \
	config/normalizations/overlay28Epoch12.mk \
	$(TOOLS_DIR)/rebind_elf_relocations.py

# O28's runtime table owns the local call target at +0x70 while the stored ROM
# carries the overlay-root zero proxy. Retain all six call sites and bind only
# that one static carrier to the root-valued symbol used before loading.
$(O28_UPDATE_WORK_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x104:overlay28UpdateVertices:ext_o0_29e00

O28_RENDER_WORK_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/func_overlay_028_F00004D8_187CDA8.c.o
$(O28_RENDER_WORK_OBJ): $(TOOLS_DIR)/trim_elf_section.py
$(O28_RENDER_WORK_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x314
