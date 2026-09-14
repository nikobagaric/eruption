# ERUPTION - Visokoperformantni, platformski neovisan Vulkan pokretač za igre i animaciju

U razvoju

## Izgradnja

Potreban je Vulkan SDK (za `glslc` i validation layere), GLFW, GLM i C++20 kompajler. Pokreni `./compile_all.sh` da u jednom koraku kompajliraš shadere i izgradiš oba test binarna (`bin/tests/triangle`, `bin/tests/sphere_scene`), ili koristi `make` izravno (`make shaders` samo za ponovno kompajliranje shadera, `make` / `make all` za sve, `make clean` za brisanje build outputa). Binarne datoteke pokreći iz roota repozitorija, npr. `./bin/tests/sphere_scene`, jer se putanje do asseta (`shaders/`, `models/`, `textures/`) rješavaju relativno na radni direktorij. Na Windowsima pokreni `compile_for_windows.bat` (potrebno je postaviti `VULKAN_SDK` i imati instaliran MinGW `g++`/GLFW).

## Struktura

- `Core/Device` - Instance, PhysicalDevice, Device, SwapChain - inicijalizacija Vulkana i komunikacija s GPU-om.
- `Core/Pipeline` - GraphicsPipeline, Shader, Framebuffer, PushConstantData - konfiguracija cjevovoda za iscrtavanje.
- `Core/Commands` - CommandPool, CommandBuffer - snimanje i slanje naredbi na GPU.
- `Core/ECS` - Registry, SparseSet, Entity - Entity Component System temeljen na sparse/dense spremištu komponenti radi cache-friendly pristupa podacima.
- `Core/Scene` - Transform, Camera, SphereCollider - podaci o objektima u sceni.
