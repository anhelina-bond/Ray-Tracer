# Detect Operating System
ifeq ($(OS),Windows_NT)
    # Windows Settings
    MKDIR = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
    RM = rmdir /s /q
    TARGET_EXT = .exe
    # Use quotes for paths with spaces if necessary
    FIX_PATH = $(subst /,\,$(1))
else
    # Linux/Mac Settings
    MKDIR = mkdir -p $(1)
    RM = rm -rf
    TARGET_EXT = 
    FIX_PATH = $(1)
endif

# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall -pthread -Iinclude -Iexternal

# Directories
SRC_DIR = src
INC_DIR = include
EXT_DIR = external
OBJ_DIR = build
BIN_DIR = bin

# Target executable name
TARGET = $(BIN_DIR)/raytracer$(TARGET_EXT)

# Find all .cpp files
SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(EXT_DIR)/tinyxml2.cpp
# Map sources to object files
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Default rule
all: prepare $(TARGET)

# Create necessary directories
prepare:
	@$(call MKDIR, $(OBJ_DIR))
	@$(call MKDIR, $(BIN_DIR))
	@$(call MKDIR, $(OBJ_DIR)/$(SRC_DIR))
	@$(call MKDIR, $(OBJ_DIR)/$(EXT_DIR))

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp files to .o files
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	$(RM) $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean prepare