#include "Engine/Core/Scene/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Engine::Core::Scene {

void Camera::setOrthographicProjection(float left, float right, float bottom,
                                        float top, float near, float far) {
  mProjectionMatrix = glm::ortho(left, right, bottom, top, near, far);
  mProjectionMatrix[1][1] *= -1.0f;
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near,
                                       float far) {
  mProjectionMatrix = glm::perspective(fovy, aspect, near, far);
  mProjectionMatrix[1][1] *= -1.0f;
}

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction,
                               glm::vec3 up) {
  mViewMatrix = glm::lookAt(position, position + direction, up);
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target,
                            glm::vec3 up) {
  setViewDirection(position, target - position, up);
}

void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
  // Same Y, X, Z rotation composition as Transform::matrix(), so a camera's
  // rotation reads identically to any other entity's. glm::lookAt (used by
  // setViewDirection) expects eye space to look down -Z, so we rotate the
  // canonical -Z/+Y axes by that same matrix rather than reusing
  // littleVulkanEngine's raw formula, which assumes its own +Z-forward,
  // Y-down convention and would otherwise face the camera the wrong way here.
  glm::mat4 rotationMatrix(1.0f);
  rotationMatrix = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  rotationMatrix = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  rotationMatrix = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

  const glm::vec3 forward = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
  const glm::vec3 up = glm::vec3(rotationMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
  setViewDirection(position, forward, up);
}

} // namespace Engine::Core::Scene
