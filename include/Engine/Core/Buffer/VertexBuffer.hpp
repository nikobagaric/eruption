#pragma once
#include "Buffer.hpp"
#include <cstring>
#include <vector>

#include <glm/glm.hpp>

namespace Engine::Core::Buffer {
struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 2>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
  }
};

template <typename VertexType> class VertexBuffer : public Buffer {
public:
  explicit VertexBuffer(Device::Device &device,
                        const std::vector<VertexType> &vertices)
      : Buffer(device, vertices.size() * sizeof(VertexType),
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        mVertexCount(static_cast<uint32_t>(vertices.size())) {
    copyData(device, vertices.data(), vertices.size() * sizeof(VertexType));
  }

  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;

  VertexBuffer(VertexBuffer &&) noexcept = default;
  VertexBuffer &operator=(VertexBuffer &&) noexcept = default;

  uint32_t getVertexCount() const { return mVertexCount; }

private:
  uint32_t mVertexCount;

  void copyData(Device::Device &device, const void *data, VkDeviceSize size) {
    void *mappedData = nullptr;
    if (vkMapMemory(device.getDevice(), getMemory(), 0, size, 0, &mappedData) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to map vertex buffer memory!");
    }

    std::memcpy(mappedData, data, size);
    vkUnmapMemory(device.getDevice(), getMemory());
  }
};
} // namespace Engine::Core::Buffer
