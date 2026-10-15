CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror
TARGET = build/engine-macos
SOURCES = main.cpp

.PHONY: all

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)
