#include "Engine/Core/Buffer/IndexBuffer.hpp"
#include "Engine/Core/Buffer/StagingBuffer.hpp"

namespace Engine::Core::Buffer {

IndexBuffer::IndexBuffer(Device::Device &device, UploadContext &uploadContext,
                         std::span<const uint32_t> indices)
    : mBuffer(device, sizeof(indices[0]) * indices.size(),
              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
  StagingBuffer staging(device, sizeof(indices[0]) * indices.size());
  staging.upload(indices.data(), indices.size_bytes());
  uploadContext.copyBuffer(staging.getBuffer(), mBuffer.getBuffer(),
                           indices.size_bytes());
}
} // namespace Engine::Core::Buffer
