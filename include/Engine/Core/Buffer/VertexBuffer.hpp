#pragma once

#include "Buffer.hpp"
#include "Engine/Core/Utility/UploadContext.hpp"
#include "StagingBuffer.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Engine::Core::Buffer {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  }
};

template <typename VertexType> class VertexBuffer : public Buffer {
public:
  explicit VertexBuffer(Device::Device &device,
                        Engine::Core::UploadContext &uploadContext,
                        const std::vector<VertexType> &vertices)
      : Buffer(device, vertices.size() * sizeof(VertexType),
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
        mVertexCount(static_cast<uint32_t>(vertices.size())) {
    const VkDeviceSize bufferSize =
        static_cast<VkDeviceSize>(vertices.size() * sizeof(VertexType));

    StagingBuffer stagingBuffer{device, bufferSize};
    stagingBuffer.upload(vertices.data(), bufferSize);

    uploadContext.copyBuffer(stagingBuffer.getBuffer(), getBuffer(),
                             bufferSize);
  }

  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;

  VertexBuffer(VertexBuffer &&) noexcept = default;
  VertexBuffer &operator=(VertexBuffer &&) noexcept = default;

  uint32_t getVertexCount() const { return mVertexCount; }

private:
  uint32_t mVertexCount{};
};

} // namespace Engine::Core::Buffer
