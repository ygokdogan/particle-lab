CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude $(shell pkg-config --cflags raylib)
LDLIBS = $(shell pkg-config --libs raylib)

SRC = $(wildcard src/*.cpp src/physics/*.cpp)
TARGET = particle_lab

all: $(TARGET)

$(TARGET): $(SRC) $(wildcard include/sandbox_lab/*.hpp)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean