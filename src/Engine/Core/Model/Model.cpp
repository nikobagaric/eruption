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

  // tinyobj triangulates every face by default, so mesh.indices is always a
  // flat run of consecutive triangles: walk it three at a time rather than
  // per-index, so a face lacking vn data can still get a normal computed
  // from its own triangle.
  for (const auto &shape : shapes) {
    const auto &meshIndices = shape.mesh.indices;
    for (size_t f = 0; f + 2 < meshIndices.size(); f += 3) {
      std::array<Buffer::Vertex, 3> faceVertices{};
      bool hasNormals = true;

      for (int k = 0; k < 3; ++k) {
        const auto &index = meshIndices[f + k];
        Buffer::Vertex &vertex = faceVertices[k];

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

        if (index.normal_index >= 0 && !attrib.normals.empty()) {
          vertex.normal = {
              attrib.normals[3 * index.normal_index + 0],
              attrib.normals[3 * index.normal_index + 1],
              attrib.normals[3 * index.normal_index + 2],
          };
        } else {
          hasNormals = false;
        }
      }

      // Fall back to a flat face normal for meshes authored without vn data,
      // so loading never silently produces zero-length normals.
      if (!hasNormals) {
        const glm::vec3 faceNormal = glm::normalize(
            glm::cross(faceVertices[1].pos - faceVertices[0].pos,
                      faceVertices[2].pos - faceVertices[0].pos));
        for (auto &vertex : faceVertices) {
          vertex.normal = faceNormal;
        }
      }

      for (auto &vertex : faceVertices) {
        if (uniqueVertices.count(vertex) == 0) {
          uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
          vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
      }
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
