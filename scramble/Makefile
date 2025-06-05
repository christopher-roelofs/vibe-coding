CXX = g++
CXXFLAGS = -std=c++11 -Wall `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf

TARGET = word_scramble
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
