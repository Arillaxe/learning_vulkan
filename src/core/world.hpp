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
  Chunk &loadChunk(int x, int z);
  void unloadChunk(int x, int z);
  Chunk *getChunk(int x, int z);
  void generateChunkMeshes();
  void generateChunkMesh(int x, int z);
  void regenNeighboringChunkMeshes(int x, int z);
  std::unordered_map<ChunkPos, Chunk, ChunkPosHash> &getChunks() { return chunks; }
  Voxel *getVoxel(int x, int y, int z);
};

#endif // WORLD_HPP
