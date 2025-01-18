BUILD_DIR := build
include $(N64_INST)/include/n64.mk
include $(T3D_INST)/t3d.mk

CC := gcc

INC_DIRS := include include/engine include/game
INC_FLAGS := $(INC_DIRS:%=-I%)
SRC_DIRS := src src/engine src/game
H_FILES := $(foreach dir,$(INC_DIRS),$(wildcard $(dir)/*.h))
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

TARGET := hungover-proto6
TARGET_STR := "Hungover Proto 6"
ROM := $(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
DFS := $(BUILD_DIR)/$(TARGET).dfs
N64_CFLAGS += -Wall -Wextra -Werror -Os $(INC_FLAGS)

ASSETS_PNG := $(wildcard assets/*.png)
ASSETS_WAV := $(wildcard assets/*.wav)
ASSETS_GLB := $(wildcard assets/*.glb)
ASSETS_CONV := \
	$(ASSETS_PNG:assets/%.png=filesystem/%.sprite) \
	$(ASSETS_WAV:assets/%.wav=filesystem/%.wav64) \
	$(ASSETS_GLB:assets/%.glb=filesystem/%.t3dm)

final: $(ROM)
$(ROM): N64_ROM_TITLE=$(TARGET_STR)
$(ROM): $(DFS) 
$(DFS): $(ASSETS_CONV)
$(ELF): $(O_FILES)

AUDIOCONV_FLAGS := --wav-compress 1
MKSPRITE_FLAGS := --compress 1
MKMODEL_FLAGS := --compress 1

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o filesystem "$<"

filesystem/%.t3dm: assets/%.glb
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@ --base-scale=26
	$(N64_BINDIR)/mkasset $(MKMODEL_FLAGS) -o filesystem $@

clean:
	rm -rf $(ROM) $(BUILD_DIR) filesystem

format: $(H_FILES) $(C_FILES)
	clang-format --style=file -i $^

-include $(wildcard $(BUILD_DIR)/*.d)
