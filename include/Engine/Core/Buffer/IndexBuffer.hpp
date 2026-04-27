#pragma once

#include "Buffer.hpp"
#include "Engine/Core/Utility/UploadContext.hpp"

#include <span>

namespace Engine::Core::Buffer {
class IndexBuffer {
public:
  IndexBuffer(Device::Device &device, UploadContext &uploadContext,
              std::span<const uint32_t> indices);

  VkBuffer getBuffer() const { return mBuffer.getBuffer(); }
  uint32_t getIndexCount() const { return mIndexCount; }
  VkIndexType getIndexType() const { return VK_INDEX_TYPE_UINT32; }

private:
  Buffer mBuffer;
  uint32_t mIndexCount = 0;
};
} // namespace Engine::Core::Buffer
