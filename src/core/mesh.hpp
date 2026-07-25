#ifndef MESH_HPP
#define MESH_HPP

#include <tiny_gltf.h>

#include <renderer/vertex.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <string>
#include <vector>

class Mesh
{
private:
  VkResource &vkResource;
  tinygltf::Model model;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  tinygltf::Model loadModel(const std::string &filename);
  std::vector<Vertex> calcVerticies();
  std::vector<uint32_t> calcIndices();
  void createVertexBuffer();
  void createIndexBuffer();

public:
  Mesh(VkResource &resource, const std::string &filename);

  vk::raii::Buffer &getVertexBuffer() { return vertexBuffer; }
  vk::raii::Buffer &getIndexBuffer() { return indexBuffer; }
  std::vector<uint32_t> &getIndices() { return indices; }
};

#endif // MESH_HPP
