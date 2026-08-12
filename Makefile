CXX := g++
CXXFLAGS := -std=c++20 -O3 -g -Iinclude
LDFLAGS := -lvulkan -lglfw -flto
GLSLC := glslc

ENGINE_SRC := $(shell find src/Engine -name '*.cpp')
UTIL_SRC := $(shell find src/Util -name '*.cpp')
TEST_SRC := $(shell find test -name '*.cpp')

SHADER_SRC := shaders/shader.vert shaders/shader.frag
SHADER_SPV := shaders/triangle.vert.spv shaders/triangle.frag.spv

BIN_DIR := bin/tests

.PHONY: all clean shaders

all: shaders $(BIN_DIR)/triangle

shaders: $(SHADER_SPV)

shaders/triangle.vert.spv: shaders/shader.vert
	$(GLSLC) $< -o $@

shaders/triangle.frag.spv: shaders/shader.frag
	$(GLSLC) $< -o $@

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/triangle: $(BIN_DIR) $(ENGINE_SRC) $(UTIL_SRC) test/API/triangle.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(ENGINE_SRC) $(UTIL_SRC) test/API/triangle.cpp $(LDFLAGS)

clean:
	rm -rf bin
	rm -f $(SHADER_SPV)
