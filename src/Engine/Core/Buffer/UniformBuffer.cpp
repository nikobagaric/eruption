#include "Engine/Core/Buffer/UniformBuffer.hpp"

#include <cstring>
#include <stdexcept>

namespace Engine::Core::Buffer {

UniformBuffer::UniformBuffer(Device::Device &device, VkDeviceSize size)
    : mBuffer(device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
      mSize(size) {
  if (vkMapMemory(device.getDevice(), mBuffer.getMemory(), 0, mSize, 0,
                  &mMapped) != VK_SUCCESS) {
    throw std::runtime_error("failed to map uniform buffer memory!");
  }
}

void UniformBuffer::write(const void *data, VkDeviceSize size,
                          VkDeviceSize offset) {
  if (offset + size > mSize) {
    throw std::runtime_error("UniformBuffer::write exceeds buffer size");
  }

  std::memcpy(static_cast<char *>(mMapped) + offset, data,
              static_cast<size_t>(size));
}

VkBuffer UniformBuffer::getBuffer() const { return mBuffer.getBuffer(); }

VkDeviceSize UniformBuffer::getSize() const { return mSize; }

} // namespace Engine::Core::Buffer
