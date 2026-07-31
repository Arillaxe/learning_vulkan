#include <core/chunk_generator.hpp>
#include <core/voxel.hpp>

constexpr int MAX_WORLD_HEIGHT = 256;

ChunkGenerator::ChunkGenerator(uint32_t seed) : perlin(seed)
{
}

Chunk ChunkGenerator::generateChunk(int chunkX, int chunkY, int chunkZ)
{
  Chunk chunk;

  chunk.isMeshed = false;
  chunk.voxelCount = 0;
  chunk.pos = {chunkX, chunkY, chunkZ};

  auto &voxels = chunk.voxels;

  constexpr double continentalFrequency = 0.002;
  constexpr double hillFrequency = 0.008;
  constexpr double detailFrequency = 0.03;

  constexpr int baseHeight = 48;
  constexpr int continentalAmplitude = 40;
  constexpr int hillAmplitude = 14;
  constexpr int detailAmplitude = 3;

  const int chunkBottom = chunkY * CHUNK_SIZE;
  const int chunkTop = chunkBottom + CHUNK_SIZE - 1;

  for (int localX = 0; localX < CHUNK_SIZE; ++localX)
  {
    for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ)
    {
      const int worldX = chunkX * CHUNK_SIZE + localX;
      const int worldZ = chunkZ * CHUNK_SIZE + localZ;

      // Large-scale terrain shape.
      const double continentalNoise = perlin.octave2D_01(
          worldX * continentalFrequency,
          worldZ * continentalFrequency,
          4);

      // Medium-sized hills.
      const double hillNoise = perlin.octave2D_01(
          worldX * hillFrequency,
          worldZ * hillFrequency,
          4);

      // Small surface variation.
      const double detailNoise = perlin.octave2D_01(
          worldX * detailFrequency,
          worldZ * detailFrequency,
          2);

      // Convert noise from [0, 1] to approximately [-1, 1].
      const double continental = continentalNoise * 2.0 - 1.0;
      const double hills = hillNoise * 2.0 - 1.0;
      const double detail = detailNoise * 2.0 - 1.0;

      const int worldHeight =
          baseHeight +
          static_cast<int>(continental * continentalAmplitude) +
          static_cast<int>(hills * hillAmplitude) +
          static_cast<int>(detail * detailAmplitude);

      for (int localY = 0; localY < CHUNK_SIZE; ++localY)
      {
        const int worldY = chunkBottom + localY;

        Voxel &voxel = voxels[localX][localY][localZ];

        voxel.position = glm::vec3(
            worldX * VOXEL_SIZE,
            worldY * VOXEL_SIZE,
            worldZ * VOXEL_SIZE);

        if (worldY <= worldHeight)
        {
          voxel.type = 1;
          ++chunk.voxelCount;
        }
        else
        {
          voxel.type = 0;
        }
      }
    }
  }

  return chunk;
}
