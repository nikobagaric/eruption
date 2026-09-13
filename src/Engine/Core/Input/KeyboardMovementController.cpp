#include "Engine/Core/Input/KeyboardMovementController.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <limits>

namespace Engine::Core::Input {

void KeyboardMovementController::moveInPlaneXZ(
    GLFWwindow *window, float dt, Scene::Transform &transform) const {
  glm::vec3 rotate{0.0f};
  if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) {
    rotate.y += 1.0f;
  }
  if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) {
    rotate.y -= 1.0f;
  }
  if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) {
    rotate.x += 1.0f;
  }
  if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) {
    rotate.x -= 1.0f;
  }

  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
    transform.rotation += lookSpeed * dt * glm::normalize(rotate);
  }

  transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
  transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

  // Rotate the canonical -Z/+X axes by yaw only, matching the forward/up
  // vectors Camera::setViewYXZ derives from the same Transform rotation, so
  // "forward" here always agrees with what the camera is actually facing.
  const glm::mat4 yawMatrix = glm::rotate(glm::mat4(1.0f), transform.rotation.y,
                                          glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::vec3 forwardDir{yawMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)};
  const glm::vec3 rightDir{yawMatrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)};
  const glm::vec3 upDir{0.0f, 1.0f, 0.0f};

  glm::vec3 moveDir{0.0f};
  if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) {
    moveDir += forwardDir;
  }
  if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {
    moveDir -= forwardDir;
  }
  if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) {
    moveDir += rightDir;
  }
  if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) {
    moveDir -= rightDir;
  }
  if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {
    moveDir += upDir;
  }
  if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {
    moveDir -= upDir;
  }

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    transform.translation += moveSpeed * dt * glm::normalize(moveDir);
  }
}

} // namespace Engine::Core::Input
