#include "Engine/Core/Buffer/StagingBuffer.hpp"

#include <cstring>
#include <stdexcept>

namespace Engine::Core::Buffer {

StagingBuffer::StagingBuffer(Device::Device &device, VkDeviceSize bufferSize)
    : Buffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
      mDevice(device), mSize(bufferSize) {
  if (vkMapMemory(mDevice.getDevice(), getMemory(), 0, mSize, 0,
                  &mMappedData) != VK_SUCCESS) {
    throw std::runtime_error("failed to map staging buffer memory!");
  }
}

StagingBuffer::~StagingBuffer() {
  if (mMappedData != nullptr) {
    vkUnmapMemory(mDevice.getDevice(), getMemory());
    mMappedData = nullptr;
  }
}

void *StagingBuffer::mapped() { return mMappedData; }

void StagingBuffer::upload(const void *data, VkDeviceSize size,
                           VkDeviceSize offset) {
  if (data == nullptr) {
    throw std::runtime_error("StagingBuffer::upload received null data");
  }

  if (offset + size > mSize) {
    throw std::runtime_error(
        "StagingBuffer::upload exceeds staging buffer size");
  }

  std::memcpy(static_cast<char *>(mMappedData) + offset, data,
              static_cast<size_t>(size));
}

} // namespace Engine::Core::Buffer
