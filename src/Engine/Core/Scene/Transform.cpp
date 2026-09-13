#include "Engine/Core/Scene/Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Engine::Core::Scene {

glm::mat4 Transform::matrix() const {
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);
  transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
  transform = glm::scale(transform, scale);
  return transform;
}

glm::mat3 Transform::normalMatrix() const {
  return glm::mat3(glm::transpose(glm::inverse(matrix())));
}

glm::vec3 forwardFromRotation(const glm::vec3 &rotation) {
  glm::mat4 rotationMatrix(1.0f);
  rotationMatrix = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  rotationMatrix = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  rotationMatrix = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
  return glm::normalize(
      glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
}

} // namespace Engine::Core::Scene
