#include "Engine/Core/Model/Model.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "ext/tiny_obj_loader.h"

#include <stdexcept>
#include <unordered_map>

namespace Engine::Core::Model {

void Model::loadFromFile(Device::Device &device, UploadContext &uploadContext,
                         const std::string &path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        path.c_str())) {
    throw std::runtime_error("failed to load model: " + path + " " + warn +
                             err);
  }

  std::vector<Buffer::Vertex> vertices;
  std::vector<uint32_t> indices;
  std::unordered_map<Buffer::Vertex, uint32_t> uniqueVertices{};

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Buffer::Vertex vertex{};

      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
      };

      if (index.texcoord_index >= 0) {
        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
        };
      } else {
        vertex.texCoord = {0.0f, 0.0f};
      }

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }

  mVertexBuffer = std::make_unique<Buffer::VertexBuffer<Buffer::Vertex>>(
      device, uploadContext, vertices);
  mIndexBuffer =
      std::make_unique<Buffer::IndexBuffer>(device, uploadContext, indices);
}

Model::Model(Device::Device &device, UploadContext &uploadContext,
            const std::string &path) {
  loadFromFile(device, uploadContext, path);
}

} // namespace Engine::Core::Model
