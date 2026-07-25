#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <core/voxel.hpp>
#include <renderer/vk.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <renderer/vertex.hpp>

constexpr int CHUNK_SIZE = 16;

class Chunk
{
private:
  VkResource &vkResource;
  Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  void createVertexBuffer();
  void createIndexBuffer();

public:
  Chunk(VkResource &resource);

  Voxel (&getVoxels()) [CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]
  { return voxels; }

  vk::raii::Buffer &getVertexBuffer()
  {
    return vertexBuffer;
  }
  vk::raii::Buffer &getIndexBuffer() { return indexBuffer; }
  std::vector<Vertex> &getVertices() { return vertices; }
  std::vector<uint32_t> &getIndices() { return indices; }

  void calculateBuffers();
};

#endif // CHUNK_HPP
