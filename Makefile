CXX := g++
CXXFLAGS := -std=c++17 -O0 -g -Iinclude
LDFLAGS := -lvulkan -lglfw

ENGINE_SRC := $(shell find src/Engine -name '*.cpp')
TEST_SRC := $(shell find test -name '*.cpp')

BIN_DIR := bin/tests

.PHONY: all clean

all: $(BIN_DIR)/triangle

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/triangle: $(BIN_DIR) $(ENGINE_SRC) test/API/triangle.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(ENGINE_SRC) test/API/triangle.cpp $(LDFLAGS)

clean:
	rm -rf bin
