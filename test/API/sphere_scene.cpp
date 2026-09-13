#include "Engine/Engine.hpp"

int main() {
  try {
    Engine::Engine engine(1280, 720, "Eruption - Sphere Scene",
                          "models/sphere.obj", /*enableSkyboxScene=*/true);
    engine.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
