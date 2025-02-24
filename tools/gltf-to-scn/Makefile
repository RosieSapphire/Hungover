BUILD_DIR := out
TARGET := gltf-to-scn

INC_DIRS := include
H_FILES := $(foreach dir,$(INC_DIRS),$(wildcard $(dir)/*.h))
SRC_DIRS := src
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c99 -ggdb3 $(INC_DIRS:%=-I%)
LIB := -lassimp -ljson-c -lm

$(TARGET): $(O_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean format tidy

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

format: $(H_FILES) $(C_FILES)
	clang-format-15 --style=file -i $^

tidy: $(H_FILES) $(C_FILES)
	@clang-tidy-19 $^
