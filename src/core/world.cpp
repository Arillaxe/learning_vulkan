#include <core/world.hpp>
#include <core/chunk_generator.hpp>
#include <core/math.hpp>

World::World(uint32_t seed) : chunkGenerator(seed)
{
}

void World::loadChunk(int x, int z)
{
  std::vector<int> chunkYs = chunkGenerator.getRenderChunksYs(x, z);

  for (auto &y : chunkYs)
  {
    glm::ivec3 pos{x, y, z};

    if (chunks.find(pos) != chunks.end() || generatedEmpty.find(pos) != generatedEmpty.end())
    {
      continue;
    }

    auto chunk = chunkGenerator.generateChunk(x, y, z);

    if (chunk.voxelCount == 0)
    {
      generatedEmpty.insert(pos);
    }

    chunks.emplace(pos, std::move(chunk));
  }
}

void World::unloadChunk(int x, int y, int z)
{
  glm::ivec3 pos{x, y, z};

  chunks.erase(pos);
  generatedEmpty.erase(pos);
}

bool World::isChunkResolved(int x, int y, int z) const
{
  glm::ivec3 pos{x, y, z};
  return chunks.find(pos) != chunks.end() || generatedEmpty.find(pos) != generatedEmpty.end();
}

Voxel *World::getVoxel(int x, int y, int z)
{
  const int chunkX = floorDiv(x, CHUNK_SIZE);
  const int chunkY = floorDiv(y, CHUNK_SIZE);
  const int chunkZ = floorDiv(z, CHUNK_SIZE);

  glm::ivec3 pos{chunkX, chunkY, chunkZ};

  auto it = chunks.find(pos);

  if (it == chunks.end())
    return nullptr;

  const int localX = floorMod(x, CHUNK_SIZE);
  const int localY = floorMod(y, CHUNK_SIZE);
  const int localZ = floorMod(z, CHUNK_SIZE);

  return &it->second.getVoxel(localX, localY, localZ);
}

std::vector<Chunk *> World::getRenderChunks(int x, int z)
{
  std::vector<Chunk *> ret;

  auto chunkYs = chunkGenerator.getRenderChunksYs(x, z);

  for (auto &y : chunkYs)
  {
    auto chunkIt = chunks.find({x, y, z});

    if (chunkIt != chunks.end())
    {
      ret.push_back(&chunkIt->second);
    }
  }

  return ret;
}

Chunk *World::getChunk(int x, int y, int z)
{
  glm::ivec3 pos{x, y, z};

  auto chunkIt = chunks.find(pos);

  if (chunkIt == chunks.end())
  {
    return nullptr;
  }

  return &chunkIt->second;
}
