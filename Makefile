DEBUG_ENABLED := 1
BUILD_DIR := out

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
ifeq ($(DEBUG_ENABLED),1)
	N64_CFLAGS += -DDEBUG_ENABLED
endif

ASSETS_PNG := $(wildcard assets/*.png)
ASSETS_WAV := $(wildcard assets/*.wav)
ASSETS_GLTF := $(wildcard assets/*.gltf)
ASSETS_SCN_GLTF := $(wildcard assets/Scn.*.gltf)
ASSETS_CONV := \
	$(ASSETS_PNG:assets/%.png=filesystem/%.sprite) \
	$(ASSETS_WAV:assets/%.wav=filesystem/%.wav64) \
	$(ASSETS_GLTF:assets/%.gltf=filesystem/%.t3dm) \
	$(ASSETS_GLTF:assets/Scn.%.gltf=filesystem/Scn.%.scn)

final: $(ROM)
$(ROM): N64_ROM_TITLE=$(TARGET_STR)
$(ROM): $(DFS) 
$(DFS): $(ASSETS_CONV)
$(ELF): $(O_FILES)

AUDIOCONV_FLAGS := --wav-compress 1
MKSPRITE_FLAGS := --compress 1
MKMODEL_FLAGS := --compress 1
GLTF_TO_SCN_FLAGS := --compress 1

GLTF_TO_SCN := ./tools/gltf-to-scn/gltf-to-scn

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o filesystem "$<"

filesystem/%.t3dm: assets/%.gltf
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@ --base-scale=64
	$(N64_BINDIR)/mkasset $(MKMODEL_FLAGS) -o filesystem $@

$(GLTF_TO_SCN):
	@echo "[COMPILING GLTF-TO-SCN] $@"
	@make -C tools/gltf-to-scn

filesystem/Scn.%.scn: assets/Scn.%.gltf $(GLTF_TO_SCN)
	@mkdir -p $(dir $@)
	@echo "    [SCENE] $@"
	$(GLTF_TO_SCN) $< $@
	$(N64_BINDIR)/mkasset -v $(GLTF_TO_SCN_FLAGS) -o filesystem $@

clean:
	rm -rf $(ROM) $(BUILD_DIR) filesystem

format: $(H_FILES) $(C_FILES)
	clang-format-15 --style=file -i $^

todo: $(H_FILES) $(C_FILES)
	grep -i "todo" $^
	grep -i "fixme" $^

-include $(wildcard $(BUILD_DIR)/*.d)
