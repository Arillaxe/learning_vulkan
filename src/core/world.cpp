#include <core/world.hpp>
#include <perlin_noise.hpp>
#include <core/world.hpp>
#include <core/world_generator.hpp>

World::World(VkResource &resource) : vkResource(resource)
{
}

void World::generateChunk(int x, int z)
{
  static const siv::PerlinNoise::seed_type seed = 123456u;
  static const siv::PerlinNoise perlin{seed};

  Chunk chunk(vkResource);

  auto &voxels = chunk.getVoxels();

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      for (int k = 0; k < CHUNK_SIZE; k++)
      {
        voxels[i][j][k] = {glm::vec3((x * CHUNK_SIZE * 10) + i * 10, j * 10, (z * CHUNK_SIZE * 10) + k * 10), 0};
      }
    }
  }

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      const double noise = perlin.octave2D_01(((i + x * CHUNK_SIZE) * 0.01), ((j + z * CHUNK_SIZE) * 0.01), 4);
      int y = std::min(
          static_cast<int>(CHUNK_SIZE * noise),
          CHUNK_SIZE - 1);

      for (int k = 0; k <= y; k++)
      {
        voxels[i][k][j].type = 1;
      }
    }
  }

  ChunkPos chunkPos = {x, 0, z};

  chunks.emplace(chunkPos, std::move(chunk));

  // auto &storedChunk = chunks.at(chunkPos);

  // auto &vertices = storedChunk.getVertices();
  // auto &indices = storedChunk.getIndices();

  // ChunkPos pos{x, 0, z};
  // std::tie(vertices, indices) = chunkToVertices(*this, storedChunk, pos);

  // storedChunk.calculateBuffers();
}

namespace
{
  int floorDiv(int value, int divisor)
  {
    int q = value / divisor;
    int r = value % divisor;

    if (r < 0)
      --q;

    return q;
  }

  int floorMod(int value, int divisor)
  {
    int r = value % divisor;

    if (r < 0)
      r += divisor;

    return r;
  }
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

  return &it->second.getVoxels()[localX][localY][localZ];
}

void World::generateChunkMeshes()
{
  for (auto &[pos, chunk] : chunks)
  {
    auto &vertices = chunk.getVertices();
    auto &indices = chunk.getIndices();

    std::tie(vertices, indices) = chunkToVertices(*this, chunk, pos);

    chunk.calculateBuffers();
  }
}
