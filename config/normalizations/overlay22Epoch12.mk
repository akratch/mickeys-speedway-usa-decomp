O22_REMOVE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22RemoveObject.c.o
O22_RESOLVE_PLANE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22ResolvePlane.c.o
O22_INITIALIZE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22InitializeObject.c.o

# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(O22_INITIALIZE_OBJECT_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_INITIALIZE_OBJECT_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000000_1878108=func_overlay_022_F0000000_1878108 $@


# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(O22_REMOVE_OBJECT_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000D30_1878E38=func_overlay_022_F0000D30_1878E38 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x16C

# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(O22_RESOLVE_PLANE_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_RESOLVE_PLANE_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000A7C_1878B84=func_overlay_022_F0000A7C_1878B84 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B4
