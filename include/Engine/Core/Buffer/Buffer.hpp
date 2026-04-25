#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "Engine/Core/Device/Device.hpp"

namespace Engine::Core::Buffer {
class Buffer {
public:
  Buffer(Device::Device &device, VkDeviceSize size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties);
  ~Buffer();

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  Buffer(Buffer &&) noexcept = default;
  Buffer &operator=(Buffer &&) noexcept = delete;

  VkBuffer getBuffer() const { return mBuffer; }
  VkDeviceMemory getMemory() const { return mBufferMemory; }

private:
  Device::Device &mDevice;

  VkBuffer mBuffer;
  VkDeviceMemory mBufferMemory;

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties);
};
} // namespace Engine::Core::Buffer
