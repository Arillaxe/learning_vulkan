#include <core/world.hpp>
#include <core/chunk_generator.hpp>
#include <core/math.hpp>

World::World(uint32_t seed) : chunkGenerator(seed)
{
}

Chunk &World::loadChunk(int x, int z)
{
  ChunkPos pos{x, 0, z};

  auto exisitingChunkIt = chunks.find(pos);

  auto [it, inserted] = chunks.try_emplace(pos);

  if (exisitingChunkIt != chunks.end())
  {
    return exisitingChunkIt->second;
  }

  chunks.emplace(pos, chunkGenerator.generateChunk(x, z));
  Chunk &newChunk = chunks.find(pos)->second;

  return newChunk;
}

void World::unloadChunk(int x, int z)
{
  ChunkPos pos{x, 0, z};

  chunks.erase(pos);
}

Voxel *World::getVoxel(int x, int y, int z)
{
  const int chunkX = floorDiv(x, CHUNK_SIZE);
  const int chunkY = floorDiv(y, CHUNK_SIZE);
  const int chunkZ = floorDiv(z, CHUNK_SIZE);

  ChunkPos pos{chunkX, chunkY, chunkZ};

  auto it = chunks.find(pos);

  if (it == chunks.end())
    return nullptr;

  const int localX = floorMod(x, CHUNK_SIZE);
  const int localY = floorMod(y, CHUNK_SIZE);
  const int localZ = floorMod(z, CHUNK_SIZE);

  return &it->second.voxels[localX][localY][localZ];
}

Chunk *World::getChunk(int x, int z)
{
  ChunkPos pos{x, 0, z};

  auto chunkIt = chunks.find(pos);

  if (chunkIt == chunks.end())
  {
    return nullptr;
  }

  return &chunkIt->second;
}
