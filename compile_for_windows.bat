@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
set SRC=
for /r src\Engine %%f in (*.cpp) do set SRC=!SRC! "%%f"
for /r src\Util %%f in (*.cpp) do set SRC=!SRC! "%%f"
for %%s in (shader lit skybox) do (
  glslc shaders\%%s.vert -o shaders\%%s.vert.spv
  glslc shaders\%%s.frag -o shaders\%%s.frag.spv
)
ren shaders\shader.vert.spv triangle.vert.spv
ren shaders\shader.frag.spv triangle.frag.spv
if not exist bin\tests mkdir bin\tests
g++ -std=c++20 -O3 -Iinclude -I"%VULKAN_SDK%\Include" -L"%VULKAN_SDK%\Lib" -o bin\tests\triangle.exe !SRC! test\API\triangle.cpp -lvulkan-1 -lglfw3
g++ -std=c++20 -O3 -Iinclude -I"%VULKAN_SDK%\Include" -L"%VULKAN_SDK%\Lib" -o bin\tests\sphere_scene.exe !SRC! test\API\sphere_scene.cpp -lvulkan-1 -lglfw3
