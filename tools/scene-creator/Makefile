DEBUG_MODE := 1

INC_DIRS := include
SRC_DIRS := src
H_FILES := $(filter-out \
	   	include/nuklear.h, \
		include/nuklear_glfw_gl3.h \
		include/stb_image.h \
	     $(foreach dir,$(INC_DIRS),$(wildcard $(dir)/*.h)))
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

BUILD_DIR := build
TARGET := scene-creator
O_FILES := $(C_FILES:%.c=$(BUILD_DIR)/%.o)

CC := gcc
INC_FLAGS := $(INC_DIRS:%=-I%)
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c99
LIB := `pkgconf --libs glfw3 glew cglm assimp` -lm
ifeq ($(DEBUG_MODE), 1)
	CFLAGS += -O0 -ggdb3
else
	CFLAGS += -O3 -DNDEBUG
endif

final: $(TARGET)

$(TARGET): $(O_FILES)
	@echo "    [CCLD] $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LIB)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "    [CC] $@"
	@$(CC) $(CFLAGS) -o $@ -c $^ $(INC_FLAGS)

format: $(H_FILES) $(C_FILES)
	@clang-format -i --style=file $^

fixme: $(H_FILES) $(C_FILES)
	@grep -E 'todo|fixme' $^ -i --color=always

clean:
	@echo "Cleaning previous build..."
	@rm -rf $(BUILD_DIR) $(TARGET) compile_commands.json
