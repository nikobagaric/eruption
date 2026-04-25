#pragma once

#include "CommandPool.hpp"
#include "Engine/Core/Device/Device.hpp"

#include <vector>
#include <vulkan/vulkan.h>

namespace Engine::Core::Commands {
class CommandBuffer {
public:
  CommandBuffer(Device::Device &device, CommandPool &commandPool,
                uint32_t bufferCount = 1);
  ~CommandBuffer();

  CommandBuffer(const CommandBuffer &) = delete;
  CommandBuffer &operator=(const CommandBuffer &) = delete;

  CommandBuffer(CommandBuffer &&) noexcept = delete;
  CommandBuffer &operator=(CommandBuffer &&) noexcept = delete;

  const std::vector<VkCommandBuffer> &getCommandBuffers() const {
    return mCommandBuffers;
  }
  VkCommandBuffer getCommandBuffer(uint32_t index = 0) const {
    return mCommandBuffers[index];
  }

private:
  void allocateCommandBuffers();
  void freeCommandBuffers();

  Device::Device &mDevice;
  CommandPool &mCommandPool;
  uint32_t mBufferCount;

  std::vector<VkCommandBuffer> mCommandBuffers;
};
} // namespace Engine::Core::Commands
