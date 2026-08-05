#ifndef CHUNK_GENERATOR_HPP
#define CHUNK_GENERATOR_HPP

#include <perlin_noise.hpp>
#include <core/chunk.hpp>

class ChunkGenerator
{
private:
  siv::PerlinNoise perlin;

  int getWorldHeight(int x, int y);

public:
  ChunkGenerator(uint32_t seed);

  std::vector<int> getRenderChunksYs(int x, int y);
  Chunk generateChunk(int x, int y, int z);
};

#endif // CHUNK_GENERATOR_HPP
