#ifndef WORLD_HPP
#define WORLD_HPP

#include <unordered_map>
#include <core/voxel.hpp>
#include <core/chunk.hpp>
#include <core/chunk_generator.hpp>

class World
{
private:
  ChunkGenerator chunkGenerator;
  std::unordered_map<ChunkPos, Chunk, ChunkPosHash> chunks;

public:
  World(uint32_t seed);
  std::unordered_map<ChunkPos, Chunk, ChunkPosHash> &getChunks() { return chunks; }

  void loadChunk(int x, int y, int z);
  void unloadChunk(int x, int y, int z);
  Chunk *getChunk(int x, int y, int z);
  Voxel *getVoxel(int x, int y, int z);
};

#endif // WORLD_HPP
