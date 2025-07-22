CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
LDFLAGS = -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

TARGET = dice_merge
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run