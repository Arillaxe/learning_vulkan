#ifndef WORLD_HPP
#define WORLD_HPP

#include <unordered_map>
#include <unordered_set>
#include <core/voxel.hpp>
#include <core/chunk.hpp>
#include <core/chunk_generator.hpp>

class World
{
private:
  ChunkGenerator chunkGenerator;
  std::unordered_map<glm::ivec3, Chunk> chunks;
  std::unordered_set<glm::ivec3> generatedEmpty;

public:
  World(uint32_t seed);
  std::unordered_map<glm::ivec3, Chunk> &getChunks() { return chunks; }

  // Returns true when a new non-empty chunk was inserted.
  void loadChunk(int x, int z);
  void unloadChunk(int x, int y, int z);
  bool isChunkResolved(int x, int y, int z) const;
  std::vector<Chunk *> getRenderChunks(int x, int z);
  Chunk *getChunk(int x, int y, int z);
  Voxel *getVoxel(int x, int y, int z);
};

#endif // WORLD_HPP
