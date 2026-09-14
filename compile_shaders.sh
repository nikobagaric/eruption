#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

glslc shaders/shader.vert -o shaders/triangle.vert.spv
glslc shaders/shader.frag -o shaders/triangle.frag.spv
glslc shaders/lit.vert -o shaders/lit.vert.spv
glslc shaders/lit.frag -o shaders/lit.frag.spv
glslc shaders/skybox.vert -o shaders/skybox.vert.spv
glslc shaders/skybox.frag -o shaders/skybox.frag.spv
