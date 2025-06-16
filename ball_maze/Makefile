# Compiler and flags
CXX = g++
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)

CXXFLAGS = -std=c++17 -Wall -g -fdiagnostics-color=always $(SDL_CFLAGS)
# LDFLAGS should contain library paths (-L) and library names (-l)
LDFLAGS = -L/usr/local/lib $(SDL_LIBS) -lSDL2_ttf -lbox2d

# Source files, object files, and target executable
SRCS = main.cpp Game.cpp Level.cpp Maze.cpp Ball.cpp ReverseItem.cpp Warp.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = ball_maze_game

# Default target
all: $(TARGET)

# Rule to link the target executable
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile .cpp files to .o files
# Explicitly listing main headers. If any of these change, relevant .cpp files recompile.
%.o: %.cpp constants.h Game.h Level.h Maze.h Ball.h ReverseItem.h Warp.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET) out.txt

# Phony targets
.PHONY: all clean
