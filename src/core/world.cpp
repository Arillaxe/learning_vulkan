#include <core/world.hpp>
#include <perlin_noise.hpp>
#include <core/world.hpp>
#include <core/world_generator.hpp>
#include <core/math.hpp>

World::World(VkResource &resource) : vkResource(resource)
{
}

bool World::loadChunk(int x, int z)
{
  ChunkPos existingPos{x, 0, z};

  auto existingChunkIt = chunks.find(existingPos);

  if (existingChunkIt != chunks.end())
  {
    return false;
  }

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
        voxels[i][j][k] = {glm::vec3((x * CHUNK_SIZE * VOXEL_SIZE) + i * VOXEL_SIZE, j * VOXEL_SIZE, (z * CHUNK_SIZE * VOXEL_SIZE) + k * VOXEL_SIZE), 0};
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

  return true;
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

void World::generateChunkMesh(int x, int z)
{
  ChunkPos pos = {x, 0, z};
  auto chunkIt = chunks.find(pos);

  if (chunkIt == chunks.end())
  {
    return;
  }

  auto &chunk = chunkIt->second;
  auto &vertices = chunk.getVertices();
  auto &indices = chunk.getIndices();

  std::tie(vertices, indices) = chunkToVertices(*this, chunk, pos);

  chunk.calculateBuffers();
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

void World::regenNeighboringChunkMeshes(int x, int z)
{
  generateChunkMesh(x - 1, z);
  generateChunkMesh(x + 1, z);
  generateChunkMesh(x, z - 1);
  generateChunkMesh(x, z + 1);
}
