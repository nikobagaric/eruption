#include "Engine/Core/Buffer/IndexBuffer.hpp"
#include "Engine/Core/Buffer/StagingBuffer.hpp"

namespace Engine::Core::Buffer {

IndexBuffer::IndexBuffer(Device::Device &device, UploadContext &uploadContext,
                         std::span<const uint32_t> indices)
    : mBuffer(device, indices.size_bytes(),
              VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
      mIndexCount(static_cast<uint32_t>(indices.size())) {
  StagingBuffer staging(device, indices.size_bytes());
  staging.upload(indices.data(), indices.size_bytes());
  uploadContext.copyBuffer(staging.getBuffer(), mBuffer.getBuffer(),
                           indices.size_bytes());
}
} // namespace Engine::Core::Buffer
