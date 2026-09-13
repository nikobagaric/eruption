#pragma once

#include <glm/glm.hpp>

namespace Engine::Core::Scene {

// Position, orientation and scale of a scene entity. rotation is in radians and
// applied in Y, X, Z order (yaw, pitch, roll). matrix() produces the model
// transform consumed by Core::Pipeline::PushConstantData::model.
struct Transform {
  glm::vec3 translation{0.0f, 0.0f, 0.0f};
  glm::vec3 rotation{0.0f, 0.0f, 0.0f};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};

  glm::mat4 matrix() const;
  glm::mat3 normalMatrix() const;
};

// Returns the world-space forward direction (local -Z) for a YXZ Euler
// rotation, using the same rotation composition as Transform::matrix() and
// Camera::setViewYXZ, so a direction computed here always agrees with where
// a Transform with that rotation is actually facing.
glm::vec3 forwardFromRotation(const glm::vec3 &rotation);

} // namespace Engine::Core::Scene
