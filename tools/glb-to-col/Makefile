TARGET := glb-to-col

C_FILES := main.c
O_FILES := $(C_FILES:%.c=%.o)

CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c99 -ggdb3
LIB := $(shell pkg-config --libs assimp) -lm

final: $(TARGET)

$(TARGET): $(O_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(TARGET) $(O_FILES) *.col *.json

format: $(C_FILES)
	clang-format --style=file -i $^
