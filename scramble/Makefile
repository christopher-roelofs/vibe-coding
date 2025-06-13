# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
LDFLAGS = 

# SDL2 flags - Using sdl2-config for portability
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)

# SDL_ttf flags
SDL_TTF_LIBS = -lSDL2_ttf

# Source files and object files
SRCDIR = src
OBJDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Executable name
TARGET = scramble

# Include paths
INCLUDE_PATHS = -I$(SRCDIR)

# --- Targets ---

.PHONY: all clean run

# Default target: build the executable
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(SDL_LIBS) $(SDL_TTF_LIBS)
	@echo "Build finished. Run with ./$(TARGET)"

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) $(SDL_CFLAGS) -c $< -o $@

# Create the build directory
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Clean up build files
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJDIR) $(TARGET)

# Run the executable
run:
	./$(TARGET)
