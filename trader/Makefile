CXX = g++
CXXFLAGS = -Wall -std=c++17
LDFLAGS = -lSDL2 -lSDL2_ttf
EXECUTABLE = trader
SRC = main.cpp

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXECUTABLE) $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE)
