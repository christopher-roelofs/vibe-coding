# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
LDFLAGS = 

# SDL2 flags - Using sdl2-config for portability
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)

# SDL_ttf flags
# Ensure SDL2_ttf headers are found. sdl2-config --cflags might cover it.
# If not, you may need to add something like: -I/opt/homebrew/include/SDL2 or -I/usr/local/include/SDL2
SDL_TTF_CFLAGS = 
SDL_TTF_LIBS = -lSDL2_ttf

# Source files and object files
SRCDIR = src
OBJDIR = build
# Find all .cpp files in SRCDIR
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Executable name
TARGET = scramble

# Include paths for project headers
# $(SRCDIR) allows #include "header.h" for files in src/
# SDL_CFLAGS should provide paths for SDL headers.
INCLUDE_PATHS = -I$(SRCDIR)

# Default target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(SDL_LIBS) $(SDL_TTF_LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(SDL_TTF_CFLAGS) $(INCLUDE_PATHS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean rule
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Run rule
run: all
	./$(TARGET)

.PHONY: all clean run
