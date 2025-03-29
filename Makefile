# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -W -Wall -O3 -DNDEBUG

# SFML library paths (adjust if SFML is installed in a custom location)
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system
SFML_INCLUDE = /usr/local/include
SFML_LIB = /usr/local/lib

# Source files
SRCS = main.cpp Solver.cpp GameWindow.cpp
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = c4solver

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -I$(SFML_INCLUDE) -L$(SFML_LIB) $(SFML_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(SFML_INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)