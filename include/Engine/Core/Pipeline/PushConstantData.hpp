#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace Engine::Core::Pipeline {

struct PushConstantData {
  glm::mat4 model;
  int32_t textureIndex{0};
  // Multiplies the mesh's authored UVs so a texture (e.g. a tiling grid on
  // the ground platform) can repeat across a large surface instead of
  // stretching once across it. 1.0 leaves UVs unchanged.
  float uvScale{1.0f};
};

} // namespace Engine::Core::Pipeline
