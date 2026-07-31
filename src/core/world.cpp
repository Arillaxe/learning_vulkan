#include <core/world.hpp>
#include <core/chunk_generator.hpp>
#include <core/math.hpp>

World::World(uint32_t seed) : chunkGenerator(seed)
{
}

bool World::loadChunk(int x, int y, int z)
{
  ChunkPos pos{x, y, z};

  if (chunks.find(pos) != chunks.end() || generatedEmpty.find(pos) != generatedEmpty.end())
  {
    return false;
  }

  auto chunk = chunkGenerator.generateChunk(x, y, z);

  if (chunk.voxelCount == 0)
  {
    generatedEmpty.insert(pos);
    return false;
  }

  chunks.emplace(pos, std::move(chunk));
  return true;
}

void World::unloadChunk(int x, int y, int z)
{
  ChunkPos pos{x, y, z};

  chunks.erase(pos);
  generatedEmpty.erase(pos);
}

bool World::isChunkResolved(int x, int y, int z) const
{
  ChunkPos pos{x, y, z};
  return chunks.find(pos) != chunks.end() || generatedEmpty.find(pos) != generatedEmpty.end();
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

Chunk *World::getChunk(int x, int y, int z)
{
  ChunkPos pos{x, y, z};

  auto chunkIt = chunks.find(pos);

  if (chunkIt == chunks.end())
  {
    return nullptr;
  }

  return &chunkIt->second;
}
