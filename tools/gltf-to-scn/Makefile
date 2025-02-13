DEBUG_ENABLED := 1
BUILD_DIR := build
TARGET := gltf-to-scn

INC_DIRS := include .
SRC_DIRS := src

C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic -ansi -ggdb3 $(INC_DIRS:%=-I%)
ifeq ($(DEBUG_ENABLED),1)
	CFLAGS += -DGLTF_TO_SCN_DEBUG
endif

LIB := $(shell pkg-config --libs assimp) -lm

final: $(TARGET)

$(TARGET): $(O_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(TARGET) $(BUILD_DIR) *.scn *.json

format: $(H_FILES) $(C_FILES)
	clang-format-15 --style=file -i $^
