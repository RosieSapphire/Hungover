DEBUG_ENABLED := 1
COMPRESS_LEVEL := 3
MDL_SCALE := 100
BUILD_DIR := build

include $(N64_INST)/include/n64.mk
include $(T3D_INST)/t3d.mk

INC_DIRS := include
INC_FLAGS := $(INC_DIRS:%=-I%)
SRC_DIRS := src
H_FILES := $(foreach dir,$(INC_DIRS),$(wildcard $(dir)/*.h))
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

TARGET := hungover
ROM := $(TARGET).z64
ELF := $(BUILD_DIR)/$(TARGET).elf
DFS := $(BUILD_DIR)/$(TARGET).dfs
N64_CFLAGS += -Wall -Wextra -Werror -Ofast $(INC_FLAGS) \
	      -DCOMPRESS_LEVEL=$(COMPRESS_LEVEL) -DMDL_SCALE=$(MDL_SCALE)
ifeq ($(DEBUG_ENABLED),1)
	N64_CFLAGS += -DDEBUG
endif

ASSETS_PNG := $(wildcard assets/*.png)
ASSETS_WAV := $(wildcard assets/*.wav)
ASSETS_GLTF := $(wildcard assets/*.gltf)
ASSETS_CONV := $(ASSETS_PNG:assets/%.png=filesystem/%.sprite) \
	       $(ASSETS_WAV:assets/%.wav=filesystem/%.wav64) \
	       $(ASSETS_GLTF:assets/%.gltf=filesystem/%.t3dm) \

final: $(ROM)
$(ROM): N64_ROM_TITLE="Hungover"
$(ROM): $(DFS) 
$(DFS): $(ASSETS_CONV)
$(ELF): $(O_FILES)

AUDIOCONV_FLAGS := --wav-compress $(COMPRESS_LEVEL)
MKSPRITE_FLAGS := --compress $(COMPRESS_LEVEL)
MKMODEL_FLAGS := --compress $(COMPRESS_LEVEL)

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o $(dir $@) "$<"

filesystem/%.wav64: assets/%.wav
	@mkdir -p $(dir $@)
	@echo "    [AUDIO] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

filesystem/%.t3dm: assets/%.gltf
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@ --base-scale=$(MDL_SCALE)
	$(N64_BINDIR)/mkasset $(MKMODEL_FLAGS) -o $(dir $@) $@

.PHONY: clean todo

clean:
	rm -rf $(ROM) $(BUILD_DIR) filesystem

todo: $(H_FILES) $(C_FILES)
	grep -i "todo" $^
	grep -i "fixme" $^

-include $(wildcard $(BUILD_DIR)/*.d)
