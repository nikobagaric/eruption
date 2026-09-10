#pragma once

#include "Engine/Core/Buffer/IndexBuffer.hpp"
#include "Engine/Core/Buffer/VertexBuffer.hpp"
#include "Engine/Core/Utility/UploadContext.hpp"

#include <memory>
#include <string>

namespace Engine::Core::Model {

class Model {
public:
  Model(Device::Device &device, UploadContext &uploadContext,
        const std::string &path);

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  Model(Model &&) noexcept = default;
  Model &operator=(Model &&) noexcept = delete;

  VkBuffer getVertexBuffer() const { return mVertexBuffer->getBuffer(); }
  VkBuffer getIndexBuffer() const { return mIndexBuffer->getBuffer(); }
  uint32_t getIndexCount() const { return mIndexBuffer->getIndexCount(); }
  static VkIndexType getIndexType() { return Buffer::IndexBuffer::getIndexType(); }

private:
  std::unique_ptr<Buffer::VertexBuffer<Buffer::Vertex>> mVertexBuffer;
  std::unique_ptr<Buffer::IndexBuffer> mIndexBuffer;

  void loadFromFile(Device::Device &device, UploadContext &uploadContext,
                    const std::string &path);
};

} // namespace Engine::Core::Model
