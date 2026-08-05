#include <core/chunk_generator.hpp>
#include <core/voxel.hpp>
#include <unordered_set>
#include <core/math.hpp>

constexpr int MAX_WORLD_HEIGHT = 256;
constexpr double continentalFrequency = 0.002;
constexpr double hillFrequency = 0.008;
constexpr double detailFrequency = 0.03;

constexpr int baseHeight = 48;
constexpr int continentalAmplitude = 40;
constexpr int hillAmplitude = 14;
constexpr int detailAmplitude = 3;

ChunkGenerator::ChunkGenerator(uint32_t seed) : perlin(seed)
{
}

int ChunkGenerator::getWorldHeight(int x, int y)
{
  double continentalNoise = perlin.octave2D_01(
      x * continentalFrequency,
      y * continentalFrequency,
      4);

  // Medium-sized hills.
  double hillNoise = perlin.octave2D_01(
      x * hillFrequency,
      y * hillFrequency,
      4);

  // Small surface variation.
  double detailNoise = perlin.octave2D_01(
      x * detailFrequency,
      y * detailFrequency,
      2);

  // Convert noise from [0, 1] to approximately [-1, 1].
  double continental = continentalNoise * 2.0 - 1.0;
  double hills = hillNoise * 2.0 - 1.0;
  double detail = detailNoise * 2.0 - 1.0;

  return baseHeight +
         static_cast<int>(continental * continentalAmplitude) +
         static_cast<int>(hills * hillAmplitude) +
         static_cast<int>(detail * detailAmplitude);
}

std::vector<int> ChunkGenerator::getRenderChunksYs(int x, int y)
{
  std::unordered_set<int> set;

  for (int localX = 0; localX < CHUNK_SIZE; ++localX)
  {
    for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ)
    {
      int worldX = x * CHUNK_SIZE + localX;
      int worldZ = y * CHUNK_SIZE + localZ;
      int worldHeight = getWorldHeight(worldX, worldZ);
      int y = floorDiv(worldHeight, CHUNK_SIZE);

      set.emplace(y);
    }
  }

  return std::vector<int>(set.begin(), set.end());
}

Chunk ChunkGenerator::generateChunk(int chunkX, int chunkY, int chunkZ)
{
  Chunk chunk;

  chunk.isMeshed = false;
  chunk.voxelCount = 0;
  chunk.pos = {chunkX, chunkY, chunkZ};

  int chunkBottom = chunkY * CHUNK_SIZE;
  int chunkTop = chunkBottom + CHUNK_SIZE - 1;

  for (int localX = 0; localX < CHUNK_SIZE; ++localX)
  {
    for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ)
    {
      int worldX = chunkX * CHUNK_SIZE + localX;
      int worldZ = chunkZ * CHUNK_SIZE + localZ;
      int worldHeight = getWorldHeight(worldX, worldZ);

      for (int localY = 0; localY < CHUNK_SIZE; ++localY)
      {
        int worldY = chunkBottom + localY;

        Voxel &voxel = chunk.getVoxel(localX, localY, localZ);

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
