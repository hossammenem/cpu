# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -I./emu

# Directories
SRC_DIR = emu
BUILD_DIR = build

# Source and object files
SRC = $(SRC_DIR)/emu.c
OBJ = $(BUILD_DIR)/emu.o
EXECUTABLE = prog

# Default target
all: $(BUILD_DIR) $(EXECUTABLE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files into executable
$(EXECUTABLE): $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE)

# Run the program
run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all clean run
