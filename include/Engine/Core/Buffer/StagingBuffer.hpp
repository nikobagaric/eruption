#pragma once

#include "Buffer.hpp"
#include "vulkan_core.h"

using namespace Engine::Core::Buffer;

namespace Engine::Core::Buffer {
class StagingBuffer : public Buffer {
public:
  explicit StagingBuffer(Device::Device &device, VkDeviceSize bufferSize)
      : Buffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {}

  StagingBuffer(const StagingBuffer &) = delete;
  StagingBuffer &operator=(const StagingBuffer &) = delete;

  StagingBuffer(StagingBuffer &&) noexcept = default;
  StagingBuffer &operator=(StagingBuffer &&) = delete;

  VkDeviceMemory mapped() { return getMemory(); }
  void upload(const void *data, VkDeviceSize size, VkDeviceSize offset = 0);

private:
};
} // namespace Engine::Core::Buffer
