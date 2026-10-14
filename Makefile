CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude $(shell pkg-config --cflags raylib)
LDLIBS = $(shell pkg-config --libs raylib)

SRC = $(wildcard src/*.cpp)
TARGET = main

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean