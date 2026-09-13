#pragma once

#include <glm/glm.hpp>

namespace Engine::Core::Scene {

// Builds the view/projection matrices consumed by the renderer's global UBO.
// The world here is Y-up, so setPerspectiveProjection/setOrthographicProjection
// apply Vulkan's clip-space Y flip internally: callers should not flip again.
class Camera {
public:
  void setOrthographicProjection(float left, float right, float bottom,
                                  float top, float near, float far);
  void setPerspectiveProjection(float fovy, float aspect, float near,
                                 float far);

  void setViewDirection(glm::vec3 position, glm::vec3 direction,
                         glm::vec3 up = {0.0f, 1.0f, 0.0f});
  void setViewTarget(glm::vec3 position, glm::vec3 target,
                      glm::vec3 up = {0.0f, 1.0f, 0.0f});
  // Builds the view matrix from a position and a Transform-style YXZ Euler
  // rotation (rotation.x = pitch, rotation.y = yaw, rotation.z = roll),
  // matching Scene::Transform::matrix()'s rotation order.
  void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

  const glm::mat4 &getProjection() const { return mProjectionMatrix; }
  const glm::mat4 &getView() const { return mViewMatrix; }

private:
  glm::mat4 mProjectionMatrix{1.0f};
  glm::mat4 mViewMatrix{1.0f};
};

} // namespace Engine::Core::Scene
