#include <core/chunk_generator.hpp>
#include <core/voxel.hpp>

ChunkGenerator::ChunkGenerator(uint32_t seed) : perlin(seed)
{
}

Chunk ChunkGenerator::generateChunk(int x, int z)
{
  Chunk chunk;

  chunk.isMeshed = false;

  auto &voxels = chunk.voxels;

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

  chunk.pos = {x, 0, z};

  return chunk;
}
