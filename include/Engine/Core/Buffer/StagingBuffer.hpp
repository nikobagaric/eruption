#pragma once

#include "Buffer.hpp"

namespace Engine::Core::Buffer {
class StagingBuffer : public Buffer {
public:
  explicit StagingBuffer(Device::Device &device, VkDeviceSize bufferSize);

  ~StagingBuffer();

  StagingBuffer(const StagingBuffer &) = delete;
  StagingBuffer &operator=(const StagingBuffer &) = delete;

  StagingBuffer(StagingBuffer &&) noexcept = default;
  StagingBuffer &operator=(StagingBuffer &&) = delete;

  void *mapped();
  void upload(const void *data, VkDeviceSize size, VkDeviceSize offset = 0);

private:
  Device::Device &mDevice;
  VkDeviceSize mSize{};
  void *mMappedData = nullptr;
};
} // namespace Engine::Core::Buffer
