$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindDirectionalObject.c.o: \
	config/normalizations/overlay1FindDirectionalObject.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindDirectionalObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindDirectionalObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1F8 d9cb02c73e426f6b2a4c36a71a7ab3c4d38b5d74812dbe63e39b77e187489e66 \
		@config/normalizations/overlay1FindDirectionalObject.ops && \
	$(OBJCOPY) --redefine-sym overlay1GetObjectList=overlay1ObjectListReloc \
		--redefine-sym sqrtf=overlay1SqrtReloc \
		--redefine-sym overlay1TrigX=overlay1TrigXReloc \
		--redefine-sym overlay1TrigY=overlay1TrigYReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F8

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolvePathPoint.c.o: \
	config/normalizations/overlay1ResolvePathPoint.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolvePathPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolvePathPoint.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x260 33306da7b622053e30f3405bab944f30333b1efd4dd763f2fb118a75127ad3bb \
		@config/normalizations/overlay1ResolvePathPoint.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x260

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1StartTimerCallbacks.c.o: \
	config/normalizations/overlay1StartTimerCallbacks.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1StartTimerCallbacks.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xE0 f3c91e2fc503b18a3c613732fe20e10e9ef6fa33ebedfe31c2851df98c651c3a \
		@config/normalizations/overlay1StartTimerCallbacks.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE0

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BendPathPoint.c.o: \
	config/normalizations/overlay1BendPathPoint.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BendPathPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BendPathPoint.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1AC e0ccf37ee706bd9abf7931693b2821ddaac0fc83a3410000e8b9dbd4072d1704 \
		@config/normalizations/overlay1BendPathPoint.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1AC

# Typed source naturally owns the full selection/interpolation state machine,
# exact frame, calls, loops, FP behavior, and all 63 loader runtime roles. Drop
# one asserted redundant zero rematerialization, then select the complete
# relocation-aware private schedule/register representation. The retained
# overlay assets own the 32 explicitly filtered duplicate LOCAL records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChoosePath.c.o: \
	config/normalizations/overlay1ChoosePath.ops \
	config/normalizations/overlay1ChoosePath.filter \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChoosePath.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChoosePath.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x6FC 75023ca8f7afc6554e89b955dd03709b6b1d9980e8e1e59ab02d67313cb7d3ee \
		'fields:0x26c:op=0x11@0x9,rs=0x4@0x0,rt=0x0@0x1,imm=0x1000@0x0' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x6F8 13256665fcf0acb281df6228da8da4c0cf89680020de534ee05ee7a4bd9072fd \
		'drop-li:0x26c:at:0:func_overlay_001_F0003750_184FB30' && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6F8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x6F8 c3819bf6ea02931bcf6fc633142cb75c860f8ec89d5a8381151869acbc8de9f8 \
		@config/normalizations/overlay1ChoosePath.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x5d0:D_1DA0:D_1D68 0x610:D_1DA0:D_1D68 \
		0x5d4:D_1D6C:D_1DA0 0x60c:D_1D6C:D_1DA0 \
		0x5d8:D_214:D_1D6C 0x608:D_214:D_1D6C \
		0x5dc:D_210:D_214 0x604:D_210:D_214 \
		0x5e0:D_20C:D_210 0x600:D_20C:D_210 \
		0x5e4:D_1D60:D_20C 0x5fc:D_1D60:D_20C \
		0x5e8:D_208:D_1D60 0x5f8:D_208:D_1D60 \
		0x5ec:D_1D68:D_208 0x5f4:D_1D68:D_208 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay1ChoosePath.filter && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x260:overlay1GetChoiceObjects:overlay1RandomWave \
		0x588:overlay1SubmitChoice:overlay1RandomWave && \
	$(OBJCOPY) \
		--redefine-sym overlay1RandomWave=func_overlay_001_F0000000_184C3E0 \
		--redefine-sym overlay1InterpolatePath=func_overlay_001_F0000CA8_184D088 \
		--redefine-sym overlay1FindChoice=func_overlay_001_F00001AC_184C58C \
		--redefine-sym overlay1MeasureChoice=func_overlay_001_F00000E4_184C4C4 $@

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ConsumeNearbyPending.c.o: \
	config/normalizations/overlay1ConsumeNearbyPending.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ConsumeNearbyPending.c.o: CFLAGS += -g3
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ConsumeNearbyPending.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x114 dce8a10514f20586e8504e0a2c8a29e8ec29d4699b10eb9f86e4f6022d2d0ac0 \
		@config/normalizations/overlay1ConsumeNearbyPending.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114

# IDO naturally emits the exact dispatcher and a private switch table. Mickey
# already ships that table and its runtime relocation records in the retained
# overlay assets. Materialize the proved pre-loader text addends, then discard
# only the duplicate compiler table/relocations before trimming alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1DispatchMode.c.o: \
	config/normalizations/overlay1DispatchMode.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1DispatchMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x31C e8faa26be0625819767e67a7c916ee69be0d3e1b637a0f67da46c2da973afb8d \
		@config/normalizations/overlay1DispatchMode.ops && \
	$(OBJCOPY) --remove-relocations=.text --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x31C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChooseModeObject.c.o: \
	config/normalizations/overlay1ChooseModeObject.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChooseModeObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ChooseModeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x15C a850d7a4391045f9fbf46c38ea7d4d9fcf89543ba703f95e568c337b1019cb1f \
		@config/normalizations/overlay1ChooseModeObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x18:5:D_1D9C 0x4C:6:D_1D9C && \
	$(OBJCOPY) --redefine-sym overlay1GetSelectObjects=overlay1GetSelectObjectsReloc \
		--redefine-sym overlay1SelectRandom=overlay1SelectRandomReloc \
		--redefine-sym overlay1SelectValue=overlay1SelectValueReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SolveAngleCandidates.c.o: \
	config/normalizations/overlay1SolveAngleCandidates.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/resize_elf_function.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SolveAngleCandidates.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1SolveAngleCandidates.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x240 964b543eb0a00cd76dff7995dfd7cfb3ea5e4f15f964f9fcc7d673a109966d00 \
		@config/normalizations/overlay1SolveAngleCandidates.ops && \
	$(OBJCOPY) --strip-symbol overlay1LoopControlCarrier $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/resize_elf_function.py $@ .text \
		overlay1SolveAngleCandidates 0x224 0x22C \
		34cb7aea1efe5006a94ce32ba12d1d1e7aeefeb15c4e7cedcfade43a083f9a5e && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x22C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateAimedTransient.c.o: \
	config/normalizations/overlay1UpdateAimedTransient.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateAimedTransient.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateAimedTransient.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3F0 9fe083a33e4d47f48545f3b7cff1f70231edcef84c1676e7680efaeeffe5aa74 \
		@config/normalizations/overlay1UpdateAimedTransient.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x10:5:D_1DA0 0xB8:6:D_1DA0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xB0:overlay1ReadSelection:func_overlay_036_F0000694_1883B4C \
		0x254:func_overlay_001_F00064F8_18528D8:func_overlay_036_F0000694_1883B4C \
		0x360:overlay1InitTimedState:func_overlay_036_F0000694_1883B4C && \
	$(OBJCOPY) \
		--redefine-sym func_overlay_036_F0000694_1883B4C=overlay1TransientCallReloc \
		--redefine-sym sqrtf=overlay1SqrtReloc \
		--redefine-sym func_8002A910=overlay1AngleReloc \
		--redefine-sym func_8002A8BC=overlay1TrigXReloc \
		--redefine-sym func_8002A8C0=overlay1TrigYReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3E4

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateRangeFlags.c.o: \
	config/normalizations/overlay1UpdateRangeFlags.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateRangeFlags.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateRangeFlags.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1E0 20f4c618c398f99e547483c22187cf1589128af4e447dde6f152f4248ea99302 \
		@config/normalizations/overlay1UpdateRangeFlags.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xCC:overlay1GetAngleValueReloc:overlay1GetObjectListReloc && \
	$(OBJCOPY) \
		--redefine-sym overlay1GetObjectListReloc=func_overlay_001_F0000000_184C3E0 \
		--redefine-sym overlay1ActivateObjectReloc=func_overlay_001_F00004B4_184C894 \
		--redefine-sym overlay1PlaySoundReloc=func_overlay_001_F00019B8_184DD98 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E0

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ActivateObject.c.o: \
	config/normalizations/overlay1ActivateObject.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ActivateObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ActivateObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x2A4 dc87390d75490d0fae0b8a3237ac4abf767154cd218060a2e16fddc2bed6906e \
		@config/normalizations/overlay1ActivateObject.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x34:5:D_0 0x4C:6:D_0 \
		0x60:5:D_1D58 0x64:6:D_1D58 \
		0xA8:5:D_1D60 0xB0:6:D_1D60 \
		0xAC:5:D_1D68Read 0xB8:6:D_1D68Read \
		0xC8:5:D_1DA0Read 0xCC:6:D_1DA0Read \
		0xD0:5:D_1D60 0xD4:6:D_1D60 \
		0xDC:5:D_0208 0xF0:6:D_0208 \
		0xE0:5:D_1D64 0xF8:6:D_1D64 \
		0xFC:5:D_020C 0x10C:6:D_020C \
		0x114:5:D_1D68Read 0x118:6:D_1D68Read \
		0x120:5:D_0210 0x12C:6:D_0210 \
		0x138:5:D_0214 0x148:6:D_0214 \
		0x184:5:D_B0 0x1A8:6:D_B0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x0:D_1D9C:D_1DA0 \
		0x4:D_1DA0:D_1D9C \
		0x8:D_1DA0:D_1D9C \
		0xC:D_1D9C:D_1DA0 \
		0xA0:overlay1Chain0Reloc:overlay1Chain0ContextReloc \
		0xB4:overlay1Chain40Reloc:overlay1Chain0ContextReloc \
		0x1B0:overlay1Chain0Reloc:overlay1Chain0ContextReloc \
		0x1C0:overlay1Chain40Reloc:overlay1Chain0ContextReloc \
		0x1CC:overlay1Chain40Reloc:overlay1Chain0ContextReloc \
		0x1F8:overlay1InterpolateReloc:overlay1Chain0ContextReloc \
		0x214:overlay1InterpolateReloc:overlay1Chain0ContextReloc && \
	$(OBJCOPY) \
		--redefine-sym overlay1Chain0ContextReloc=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2A4

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindType47ByAngle.c.o: \
	config/normalizations/overlay1FindType47ByAngle.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1FindType47ByAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x130 8589731532dcd7ee50f9f97d5347bb494717ae7754cef1ac4db9bc85f7b09738 \
		@config/normalizations/overlay1FindType47ByAngle.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitMotionScale.c.o: \
	config/normalizations/overlay1InitMotionScale.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitMotionScale.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitMotionScale.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xD4 1c8e29b249480bd39c293ea63777d293c2df7052afb5e14089e1b4acbe9b5a4a \
		@config/normalizations/overlay1InitMotionScale.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:D_20C 0x4:5:D_210 0x8:6:D_210 0xC:6:D_20C \
		0x44:5:D_20C 0x48:5:D_1D9C 0x4C:6:D_1D9C 0x50:6:D_20C \
		0xAC:5:D_1DA0 0xB0:6:D_1DA0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xA0:overlay1AngleFromIndex:overlay1SquareRoot && \
	$(OBJCOPY) --redefine-sym \
		overlay1SquareRoot=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InterpolatePath.c.o: \
	config/normalizations/overlay1InterpolatePath.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InterpolatePath.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x14C 007e72800ac8e4f84fecae7609f26af761f437375666589b7c575ab0ad92b001 \
		@config/normalizations/overlay1InterpolatePath.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x04:5:D_1DA0 0x08:6:D_1DA0 \
		0x3C:5:D_1D60 0x44:5:D_1D64 0x48:5:D_1D68 0x4C:5:D_1D6C \
		0x54:6:D_1D60 0x58:6:D_1D64 0x5C:6:D_1D68 0x60:6:D_1D6C && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xF0:overlay1CubicInterpolate:overlay1NextControlTable \
		0x118:overlay1CubicInterpolate:overlay1NextControlTable && \
	$(OBJCOPY) --redefine-sym \
		overlay1NextControlTable=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolveMotionPoint.c.o: \
	config/normalizations/overlay1ResolveMotionPoint.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolveMotionPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1ResolveMotionPoint.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x190 97fafcaba543da3f34b5efd4cdcc72e4ab5441ff9a06f8b4a5056c59ed1a3066 \
		@config/normalizations/overlay1ResolveMotionPoint.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x28:5:D_0 0x4C:6:D_0 \
		0xE4:5:D_B4 0xF4:6:D_B4 \
		0x134:5:D_B8 0x144:6:D_B8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x120:overlay1SinAngle:overlay1SquareRoot \
		0x158:overlay1CosAngle:overlay1SquareRoot && \
	$(OBJCOPY) \
		--redefine-sym overlay1HasPathData=func_overlay_001_F00004B4_184C894 \
		--redefine-sym overlay1InterpolatePath=func_overlay_001_F0000CA8_184D088 \
		--redefine-sym overlay1SquareRoot=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x190

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1MeasureCurves.c.o: \
	config/normalizations/overlay1MeasureCurves.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1MeasureCurves.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1MeasureCurves.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x13C 305159f6f32f9be876c680bc8a451a9b7ce3b03aaa17ca64e86df047e8a6a146 \
		@config/normalizations/overlay1MeasureCurves.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xE8:overlay1SquareRoot:overlay1EvaluateCurve && \
	$(OBJCOPY) --redefine-sym \
		overlay1EvaluateCurve=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeModeState.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeModeState.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_001_F0000614_184C9F4=overlay1ModeResolverReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BuildObjectMappings.c.o: \
	config/normalizations/overlay1BuildObjectMappings.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/resize_elf_function.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1BuildObjectMappings.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x250 3abde2cc090eab8a26b74ce59075a119eb3a0c6ac3eac67f877297504cecec34 \
		@config/normalizations/overlay1BuildObjectMappings.ops && \
	$(OBJCOPY) --strip-symbol overlay1BuildScheduleCarrier $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/resize_elf_function.py $@ .text \
		overlay1BuildObjectMappings 0x240 0x250 \
		3abde2cc090eab8a26b74ce59075a119eb3a0c6ac3eac67f877297504cecec34

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceObjectGauges.c.o: \
	config/normalizations/overlay1AdvanceObjectGaugesDrop1.ops \
	config/normalizations/overlay1AdvanceObjectGaugesDrop2.ops \
	config/normalizations/overlay1AdvanceObjectGauges.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceObjectGauges.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceObjectGauges.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x13C 9f80a277c091e5091605f50e52011813464193f37e76dad34708667926ef3fdf \
		@config/normalizations/overlay1AdvanceObjectGaugesDrop1.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x138 3bf723c01e1083be212d95b705a4eced7c03020642cd7e3089ae3306b2c2bfe9 \
		@config/normalizations/overlay1AdvanceObjectGaugesDrop2.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x138 2ee064583e186df6302a93d4bc0cae1df66096fa52e0e65355dd6b31c84b6d40 \
		@config/normalizations/overlay1AdvanceObjectGauges.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x58:5:D_0 0x5C:6:D_0 && \
	$(OBJCOPY) --redefine-sym \
		overlay1GetGaugeObjectsRaw=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xA4:overlay1GetGaugeLimit:func_overlay_001_F0000000_184C3E0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceGauge.c.o: \
	config/normalizations/overlay1AdvanceGauge.ops \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceGauge.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AdvanceGauge.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xA8 63acb6fa56a9907f65b9d09b35dc1416b8c2b0cb4a38c8958f42880768ae0c72 \
		@config/normalizations/overlay1AdvanceGauge.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x34:5:D_0 0x38:6:D_0 && \
	$(OBJCOPY) --redefine-sym \
		overlay1GetGaugeObjects=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeGaugeObjects.c.o: \
	config/normalizations/overlay1InitializeGaugeObjects.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeGaugeObjects.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1InitializeGaugeObjects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x128 be06be1346c8dca8d15a9666d2c63484f6a070e60d3f628020326c70a7a2c284 \
		@config/normalizations/overlay1InitializeGaugeObjects.ops && \
	$(OBJCOPY) --redefine-sym \
		overlay1GetGaugeTable=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x28:overlay1GetGaugeObjects:func_overlay_001_F0000000_184C3E0 \
		0xD4:overlay1RandomRange:func_overlay_001_F0000000_184C3E0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AssignRecordIndex.c.o: \
	config/normalizations/overlay1AssignRecordIndex.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1AssignRecordIndex.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0xB0 9f87498d659a217b4bb311b7325437409abe66332ab3cc152bf76cf527f7d9fd \
		@config/normalizations/overlay1AssignRecordIndex.ops && \
	$(OBJCOPY) \
		--redefine-sym overlay1GetVariableRecords=func_overlay_001_F0000000_184C3E0 \
		--redefine-sym D_1D8CRead=D_1D8C $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB0

$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1HandleCachedMode.c.o: \
	config/normalizations/overlay1HandleCachedModeDrop1.ops \
	config/normalizations/overlay1HandleCachedModeDrop2.ops \
	config/normalizations/overlay1HandleCachedModeDrop3.ops \
	config/normalizations/overlay1HandleCachedModeDrop4.ops \
	config/normalizations/overlay1HandleCachedMode.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1HandleCachedMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x8C bcea9f6b24cb4f41c9e727e2ef88828fabf72022c5f2d3aeb124a2f44dbc964c \
		@config/normalizations/overlay1HandleCachedModeDrop1.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x88 f4dea2f1c7d5728576df23f4baf30edcdc5a365bb9fc9ea23facfc4ae278b1c3 \
		@config/normalizations/overlay1HandleCachedModeDrop2.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x84 5549fdab1981f01b15e5b640df62b755bf0a1f54bea2a07249ea968d45ed7f3c \
		@config/normalizations/overlay1HandleCachedModeDrop3.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x80 0dcb1f9bae6fec230eef2cc393229bf07510bb0b81a5611cb3075e8ff26cc340 \
		@config/normalizations/overlay1HandleCachedModeDrop4.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x80 e29e178da3670cb4d34853c5f835ea1f7b97f445da69790743b16df39a7215ed \
		@config/normalizations/overlay1HandleCachedMode.ops && \
	$(OBJCOPY) \
		--redefine-sym overlay27CanUse=func_overlay_001_F0000000_184C3E0 \
		--redefine-sym overlay1DispatchMode=func_overlay_001_F0005ED4_18522B4 \
		--redefine-sym D_1DA0=overlay1CachedModeWorldReloc \
		--redefine-sym D_1D9C=overlay1CachedModeOwnerReloc \
		--redefine-sym D_83E4=overlay1CachedModeSelectorReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x58:overlay3RunCachedModeAction:func_overlay_001_F0000000_184C3E0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80

# The typed source preserves the complete 48-call physics/update semantics and
# all 184 runtime relocation roles. IDO's target-proved non-unrolled loop is
# expanded by a fail-loud representation pool, then a complete guarded,
# relocation-aware schedule/register/frame web selects the shipped form.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateObjectPhysics.c.o: \
	config/normalizations/overlay1UpdateObjectPhysics.prepare.py \
	config/normalizations/overlay1UpdateObjectPhysics.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateObjectPhysics.c.o: CFLAGS += \
	-Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1UpdateObjectPhysics.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay1UpdateObjectPhysics.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x1818 fc05dbdd8106c5dd59d1a00dd03f55b7add34aa1199c20e832e77cb8705b5839 \
		@config/normalizations/overlay1UpdateObjectPhysics.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x470:D_1DA0:D_1D9C 0x478:D_1DA0:D_1D9C \
		0x474:D_1D9C:D_1DA0 0x480:D_1D9C:D_1DA0

# Typed source preserves the complete two-path, five-phase transition state
# machine and all 13 runtime roles. A fail-loud three-word representation pool
# and complete guarded bijective schedule/register web select the shipped form.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1TransitionState.c.o: \
	config/normalizations/overlay1TransitionState.prepare.py \
	config/normalizations/overlay1TransitionState.ops \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1TransitionState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay1TransitionState.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x3B4 2e8eb27969fd7ecf850385c0eeba5425a54dfc126416cc486643cf406786fb40 \
		@config/normalizations/overlay1TransitionState.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x40:local_378:ext_o7_ccc \
		0x1DC:local_414:ext_o7_ccc \
		0x1E8:local_c0:ext_o7_ccc

# Typed source preserves the complete group/link and large-record build paths,
# all sixteen calls, and all 114 loader relocation identities. Expand the
# natural 0x8A0 text by its proved representation pool, select the complete
# guarded relocation-aware schedule/register/frame web, then retain only the
# 32 records owned by the raw static link. Descriptive loader-call identities
# collapse to retail's shared zero-addend carrier only after the runtime ledger
# has been preserved by the configured object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1LoadBuildRecords.c.o: \
	config/normalizations/overlay1LoadBuildRecords.prepare.py \
	config/normalizations/overlay1LoadBuildRecords.ops \
	config/normalizations/overlay1LoadBuildRecords.filter \
	$(TOOLS_DIR)/normalize_elf_instructions.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay1LoadBuildRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) config/normalizations/overlay1LoadBuildRecords.prepare.py $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/normalize_elf_instructions.py $@ .text \
		0x8F0 bd367c6e1060dc470e097b987064e9edaf9657177b463d88671033d8fec440bd \
		@config/normalizations/overlay1LoadBuildRecords.ops && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x44:D_1D7C:D_1D8C 0x58:D_1D7C:D_1D8C \
		0x48:D_1D8C:D_1D7C 0x54:D_1D8C:D_1D7C && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay1LoadBuildRecords.filter && \
	$(OBJCOPY) \
		--redefine-sym D_0_Clear=D_5 \
		--redefine-sym overlay1LoadPackedRecordsReloc=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x248:overlay1ReleaseBuildMemoryReloc:func_overlay_001_F0000000_184C3E0 \
		0x264:overlay1ReleaseBuildMemoryReloc:func_overlay_001_F0000000_184C3E0 \
		0x280:overlay1AllocateBuildMemoryReloc:func_overlay_001_F0000000_184C3E0 \
		0x298:overlay1AllocateBuildMemoryReloc:func_overlay_001_F0000000_184C3E0 \
		0x4B4:overlay1RejectBuildCycleReloc:func_overlay_001_F0000000_184C3E0 \
		0x50C:overlay1FinalizeBuildGroupReloc:func_overlay_001_F0000000_184C3E0 \
		0x524:overlay1SubmitBuildReloc:func_overlay_001_F0000000_184C3E0 \
		0x5B0:overlay1AllocateBuildMemoryReloc:func_overlay_001_F0000000_184C3E0 \
		0x5C8:overlay1ClearLargeRecordsReloc:func_overlay_001_F0000000_184C3E0 \
		0x614:overlay1DecodeLargeRecordReloc:func_overlay_001_F0000000_184C3E0 \
		0x678:overlay1ReportMissingLargeReloc:func_overlay_001_F0000000_184C3E0 \
		0x6F0:overlay1GetMetricSourceAReloc:func_overlay_001_F0000000_184C3E0 \
		0x6FC:overlay1GetMetricSourceBReloc:func_overlay_001_F0000000_184C3E0 \
		0x708:overlay1GetMetricSourceCReloc:func_overlay_001_F0000000_184C3E0
