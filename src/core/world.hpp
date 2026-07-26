#ifndef WORLD_HPP
#define WORLD_HPP

#include <unordered_map>
#include <renderer/vk/vk_resource.hpp>
#include <core/voxel.hpp>
#include <core/chunk.hpp>

class World
{
private:
  VkResource &vkResource;
  std::unordered_map<ChunkPos, Chunk, ChunkPosHash> chunks;

public:
  World(VkResource &resource);
  bool loadChunk(int x, int z);
  void generateChunkMeshes();
  void generateChunkMesh(int x, int z);
  std::unordered_map<ChunkPos, Chunk, ChunkPosHash> &getChunks() { return chunks; }
  Voxel *getVoxel(int x, int y, int z);
};

#endif // WORLD_HPP
