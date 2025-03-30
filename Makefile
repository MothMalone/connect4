# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -W -Wall -O3 -DNDEBUG

# SFML library paths (try to detect system-specific paths)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)    # macOS
	SFML_INCLUDE ?= $(shell brew --prefix sfml)/include
	SFML_LIB ?= $(shell brew --prefix sfml)/lib
else ifeq ($(UNAME_S),Linux)    # Linux
	SFML_INCLUDE ?= /usr/include
	SFML_LIB ?= /usr/lib/x86_64-linux-gnu
else                            # Windows (assuming MSYS2/MinGW)
	SFML_INCLUDE ?= /mingw64/include
	SFML_LIB ?= /mingw64/lib
endif

SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Source files
SRCS = main.cpp Solver.cpp GameWindow.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

# Target executable
TARGET = c4solver

# Default target
.PHONY: all
all: check-sfml $(TARGET)

# Check SFML installation
.PHONY: check-sfml
check-sfml:
	@echo "Checking SFML installation..."
	@if [ ! -d "$(SFML_INCLUDE)/SFML" ]; then \
		echo "Error: SFML headers not found in $(SFML_INCLUDE)"; \
		echo "Please install SFML:"; \
		echo "  macOS:   brew install sfml"; \
		echo "  Linux:   sudo apt-get install libsfml-dev"; \
		echo "  Windows: pacman -S mingw-w64-x86_64-sfml"; \
		exit 1; \
	fi
	@echo "SFML found in $(SFML_INCLUDE)"

# Build the target
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -I$(SFML_INCLUDE) -L$(SFML_LIB) $(SFML_LIBS)
	@echo "Build successful! Run './$(TARGET)' to start the game."

# Generate dependencies and compile
%.o: %.cpp
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -MMD -MP -I$(SFML_INCLUDE) -c $< -o $@

# Include generated dependencies
-include $(DEPS)

# Clean build files
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@rm -f $(OBJS) $(DEPS) $(TARGET)
	@echo "Clean complete"

# Run the game
.PHONY: run
run: all
	@echo "Starting Connect 4..."
	@./$(TARGET)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make       - Build the game (default)"
	@echo "  make run   - Build and run the game"
	@echo "  make clean - Remove built files"
	@echo "  make help  - Show this help message"