CXX := g++
CXXFLAGS := -std=c++20 -O3 -g -Iinclude
LDFLAGS := -lvulkan -lglfw -flto
GLSLC := glslc

ENGINE_SRC := $(shell find src/Engine -name '*.cpp')
UTIL_SRC := $(shell find src/Util -name '*.cpp')
TEST_SRC := $(shell find test -name '*.cpp')

SCENE_SRC := $(shell find src/Engine/Core/Scene -name '*.cpp')

SHADER_SRC := shaders/shader.vert shaders/shader.frag shaders/lit.vert shaders/lit.frag shaders/skybox.vert shaders/skybox.frag
SHADER_SPV := shaders/triangle.vert.spv shaders/triangle.frag.spv shaders/lit.vert.spv shaders/lit.frag.spv shaders/skybox.vert.spv shaders/skybox.frag.spv

BIN_DIR := bin/tests

.PHONY: all clean shaders ecs-test

all: shaders $(BIN_DIR)/triangle $(BIN_DIR)/sphere_scene

shaders: $(SHADER_SPV)

shaders/triangle.vert.spv: shaders/shader.vert
	$(GLSLC) $< -o $@

shaders/triangle.frag.spv: shaders/shader.frag
	$(GLSLC) $< -o $@

shaders/lit.vert.spv: shaders/lit.vert
	$(GLSLC) $< -o $@

shaders/lit.frag.spv: shaders/lit.frag
	$(GLSLC) $< -o $@

shaders/skybox.vert.spv: shaders/skybox.vert
	$(GLSLC) $< -o $@

shaders/skybox.frag.spv: shaders/skybox.frag
	$(GLSLC) $< -o $@

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/triangle: $(BIN_DIR) $(ENGINE_SRC) $(UTIL_SRC) test/API/triangle.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(ENGINE_SRC) $(UTIL_SRC) test/API/triangle.cpp $(LDFLAGS)

$(BIN_DIR)/sphere_scene: $(BIN_DIR) $(ENGINE_SRC) $(UTIL_SRC) test/API/sphere_scene.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(ENGINE_SRC) $(UTIL_SRC) test/API/sphere_scene.cpp $(LDFLAGS)

$(BIN_DIR)/ecs_test: $(BIN_DIR) $(SCENE_SRC) test/ECS/registry_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(SCENE_SRC) test/ECS/registry_test.cpp

ecs-test: $(BIN_DIR)/ecs_test
	./$(BIN_DIR)/ecs_test

clean:
	rm -rf bin
	rm -f $(SHADER_SPV)
