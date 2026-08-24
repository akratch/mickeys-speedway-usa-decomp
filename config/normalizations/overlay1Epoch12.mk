$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChoosePath.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6F8

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1TransitionState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B4

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateObjectPhysics.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1818

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindDirectionalObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindDirectionalObject.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0005CD4_18520B4=overlay1FindDirectionalObject $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F8

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolvePathPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolvePathPoint.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0007D6C_185414C=overlay1ResolvePathPoint $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1StartTimerCallbacks.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0005BF4_1851FD4=overlay1StartTimerCallbacks $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BendPathPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BendPathPoint.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0007730_1853B10=overlay1BendPathPoint $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1AC

# Typed source naturally owns the full selection/interpolation state machine,
# exact frame, calls, loops, FP behavior, and all 63 loader runtime roles. Drop
# one asserted redundant zero rematerialization, then select the complete
# relocation-aware private schedule/register representation. The retained
# overlay assets own the 32 explicitly filtered duplicate LOCAL records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChoosePath.c.o: CFLAGS += \
	-Wab,-r4300_mul

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ConsumeNearbyPending.c.o: CFLAGS += -g3
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ConsumeNearbyPending.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0006A14_1852DF4=overlay1ConsumeNearbyPending $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114

# IDO naturally emits the exact dispatcher and a private switch table. Mickey
# already ships that table and its runtime relocation records in the retained
# overlay assets. Materialize the proved pre-loader text addends, then discard
# only the duplicate compiler table/relocations before trimming alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1DispatchMode.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0005ED4_18522B4=overlay1DispatchMode $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x31C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChooseModeObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChooseModeObject.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0006270_1852650=overlay1ChooseModeObject $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SolveAngleCandidates.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SolveAngleCandidates.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F00064F8_18528D8=overlay1SolveAngleCandidates $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x22C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateAimedTransient.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateAimedTransient.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0006D4C_185312C=overlay1UpdateAimedTransient $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3E4

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateRangeFlags.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateRangeFlags.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F00067C0_1852BA0=overlay1UpdateRangeFlags $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ActivateObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ActivateObject.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F00004B4_184C894=overlay1ActivateObject \
		--redefine-sym func_overlay_001_F0000614_184C9F4=overlay1FindClosestSample $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2A4

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindType47ByAngle.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F00001AC_184C58C=overlay1FindType47ByAngle $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitMotionScale.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitMotionScale.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0000BD4_184CFB4=overlay1InitMotionScale $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InterpolatePath.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0000CA8_184D088=overlay1InterpolatePath $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolveMotionPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolveMotionPoint.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0000DF4_184D1D4=overlay1ResolveMotionPoint $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1MeasureCurves.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1MeasureCurves.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0000F84_184D364=overlay1MeasureCurves $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeModeState.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeModeState.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_001_F0000614_184C9F4=overlay1ModeResolverReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BuildObjectMappings.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0001A54_184DE34=overlay1BuildObjectMappings $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceObjectGauges.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceObjectGauges.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F000296C_184ED4C=overlay1AdvanceObjectGauges $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceGauge.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceGauge.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0002AA4_184EE84=overlay1AdvanceGauge $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeGaugeObjects.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeGaugeObjects.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0003578_184F958=overlay1InitializeGaugeObjects $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AssignRecordIndex.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F00036A0_184FA80=overlay1AssignRecordIndex $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1HandleCachedMode.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F00061F0_18525D0=overlay1HandleCachedMode $@

# The typed source preserves the complete 48-call physics/update semantics and
# all 184 runtime relocation roles. IDO's target-proved non-unrolled loop is
# expanded by a fail-loud representation pool, then a complete guarded,
# relocation-aware schedule/register/frame web selects the shipped form.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateObjectPhysics.c.o: CFLAGS += \
	-Wo,-loopunroll,0

# Typed source preserves the complete two-path, five-phase transition state
# machine and all 13 runtime roles. A fail-loud three-word representation pool
# and complete guarded bijective schedule/register web select the shipped form.

# Typed source preserves the complete group/link and large-record build paths,
# all sixteen calls, and all 114 loader relocation identities. Expand the
# natural 0x8A0 text by its proved representation pool, select the complete
# guarded relocation-aware schedule/register/frame web, then retain only the
# 32 records owned by the raw static link. Descriptive loader-call identities
# collapse to retail's shared zero-addend carrier only after the runtime ledger
# has been preserved by the configured object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1LoadBuildRecords.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F00010C8_184D4A8=overlay1LoadBuildRecords $@
