# 编译器与标志
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = 

# 项目目录结构
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = $(SRC_DIR)/zlib

# 搜索所有源文件
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TARGET = $(BUILD_DIR)/test.exe

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJS)
	@mkdir -p $(@D)  # 确保输出目录存在
	$(CC) $(LDFLAGS) -o $@ $^

# 编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)  # 确保输出目录存在
	$(CC) $(CFLAGS) -c $< -o $@

# 编译规则 (针对子目录中的文件)
$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
