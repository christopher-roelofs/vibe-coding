# Compiler and flags
CXX = g++
# Use pkg-config to get the correct flags for SDL2 and SDL2_image
# Make sure you have pkg-config installed (brew install pkg-config)
CXXFLAGS = -std=c++17 -Wall $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf)
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image SDL2_ttf)

# Project files
TARGET = revenge
SOURCES = main.cpp TextureManager.cpp Level.cpp GameObject.cpp Player.cpp Block.cpp Cat.cpp FontManager.cpp Cheese.cpp Trap.cpp Hole.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compiling source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean