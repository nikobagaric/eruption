#pragma once
#include "Buffer.hpp"
#include <glm/glm.hpp>

namespace Engine::Core::Buffer {

constexpr VkDeviceSize alignUp(VkDeviceSize size, VkDeviceSize alignment) {
  return (size + alignment - 1) & ~(alignment - 1);
}

struct UniformBufferObject {
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec4 lightPos;
  float time;
  float padding[3];
};

class UniformBuffer {
public:
  UniformBuffer(Device::Device &device, VkDeviceSize size);

  void write(const void *data, VkDeviceSize size, VkDeviceSize offset = 0);
  VkBuffer getBuffer() const;
  VkDeviceSize getSize() const;

private:
  Buffer mBuffer;
  VkDeviceSize mSize;
  void *mMapped;
};

} // namespace Engine::Core::Buffer
