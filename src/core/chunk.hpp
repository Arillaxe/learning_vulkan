#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <core/voxel.hpp>
#include <renderer/vk.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <renderer/vertex.hpp>

constexpr int CHUNK_SIZE = 16;

struct ChunkPos
{
  int x;
  int y;
  int z;

  bool operator==(const ChunkPos &) const = default;
};

struct ChunkPosHash
{
  std::size_t operator()(const ChunkPos &pos) const
  {
    std::size_t h1 = std::hash<int>{}(pos.x);
    std::size_t h2 = std::hash<int>{}(pos.y);
    std::size_t h3 = std::hash<int>{}(pos.z);

    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};

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
