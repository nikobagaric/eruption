#pragma once

#include <string>

namespace Engine::Core::Scene {

// Marks an entity's Transform as a sphere for the ray-sphere picking demo in
// Engine::performRaycast(). radius is in object space and scaled by the
// entity's Transform::scale.x (uniform scale assumed).
struct SphereCollider {
  float radius{1.0f};
  std::string name;
};

} // namespace Engine::Core::Scene
