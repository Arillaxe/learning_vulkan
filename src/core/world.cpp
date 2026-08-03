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

bool World::loadRegion(int x, int y, int z)
{
  ChunkPos pos{x, y, z};

  if (regions.find(pos) != regions.end())
  {
    return false;
  }

  Chunks newRegionChunks;

  int chunksPerRegion = REGION_SIZE / CHUNK_SIZE;

  for (int i = 0; i < chunksPerRegion; i++)
  {
    for (int j = 0; j < chunksPerRegion; j++)
    {
      for (int k = 0; k < chunksPerRegion; k++)
      {
        Chunk c = chunkGenerator.generateChunk(x * chunksPerRegion + i, y * chunksPerRegion + j, z * chunksPerRegion + k);

        if (c.voxelCount > 0)
        {
          newRegionChunks.emplace(ChunkPos{i, j, k}, c);
        }
      }
    }
  }

  auto [newRegionIt, inserted] = regions.emplace(pos, std::move(newRegionChunks));

  newRegionIt->second.buildLODs();

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

Region *World::getRegion(int x, int y, int z)
{
  ChunkPos pos{x, y, z};

  auto regionIt = regions.find(pos);

  if (regionIt == regions.end())
  {
    return nullptr;
  }

  return &regionIt->second;
}
