# ERUPTION - High-performance Platform-agnostic Vulkan Game & Animation engine

In-dev

## Building

Requires the Vulkan SDK (for `glslc` and the validation layers), GLFW, and a C++20 compiler. Run `./compile_all.sh` to compile the shaders and build both test binaries (`bin/tests/triangle`, `bin/tests/sphere_scene`) in one step, or use `make` directly (`make shaders` to just recompile shaders, `make` / `make all` for everything, `make clean` to remove build output). Run a binary from the repo root, e.g. `./bin/tests/sphere_scene`, since asset paths (`shaders/`, `models/`, `textures/`) are resolved relative to the working directory. On Windows, run `compile_for_windows.bat` (needs `VULKAN_SDK` set and a MinGW `g++`/GLFW install).