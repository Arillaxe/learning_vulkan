#ifndef GPU_CHUNK_MESH_HPP
#define GPU_CHUNK_MESH_HPP

#include <renderer/vk/vk_resource.hpp>
#include <renderer/vertex.hpp>
#include <core/chunk.hpp>

class GPUChunkMesh
{
private:
  ChunkPos chunkPos;
  VkResource *vkResource;
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexMemory = nullptr;
  uint32_t indicesCount = 0;

public:
  GPUChunkMesh(VkResource *resource, ChunkPos pos);

  vk::raii::Buffer &getVertexBuffer() { return vertexBuffer; }
  vk::raii::Buffer &getIndexBuffer() { return indexBuffer; }
  uint32_t getIndicesCount() { return indicesCount; }
  const ChunkPos &getChunkPos() const { return chunkPos; }

  void generateRenderMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
};

#endif // GPU_CHUNK_MESH_HPP
