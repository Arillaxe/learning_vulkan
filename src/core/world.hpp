#ifndef WORLD_HPP
#define WORLD_HPP

#include <unordered_map>
#include <unordered_set>
#include <core/voxel.hpp>
#include <core/chunk.hpp>
#include <core/chunk_generator.hpp>
#include <core/region.hpp>

class World
{
private:
  std::unordered_map<ChunkPos, Region, ChunkPosHash> regions;
  ChunkGenerator chunkGenerator;
  std::unordered_map<ChunkPos, Chunk, ChunkPosHash> chunks;
  // Empty chunks are not stored, but remember them so we don't regenerate.
  std::unordered_set<ChunkPos, ChunkPosHash> generatedEmpty;

public:
  World(uint32_t seed);
  std::unordered_map<ChunkPos, Chunk, ChunkPosHash> &getChunks() { return chunks; }

  // Returns true when a new non-empty chunk was inserted.
  bool loadRegion(int x, int y, int z);
  bool loadChunk(int x, int y, int z);
  void unloadChunk(int x, int y, int z);
  bool isChunkResolved(int x, int y, int z) const;
  Chunk *getChunk(int x, int y, int z);
  Region *getRegion(int x, int y, int z);
  Voxel *getVoxel(int x, int y, int z);
};

#endif // WORLD_HPP
