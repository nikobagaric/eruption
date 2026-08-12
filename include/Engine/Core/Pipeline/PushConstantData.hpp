#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace Engine::Core::Pipeline {

struct PushConstantData {
  glm::mat4 model;
  int32_t textureIndex{0};
};

} // namespace Engine::Core::Pipeline
