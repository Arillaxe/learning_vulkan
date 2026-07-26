#ifndef CHUNK_GENERATOR_HPP
#define CHUNK_GENERATOR_HPP

#include <perlin_noise.hpp>
#include <core/chunk.hpp>

class ChunkGenerator
{
private:
  siv::PerlinNoise perlin;

public:
  ChunkGenerator(uint32_t seed);

  Chunk generateChunk(int x, int z);
};

#endif // CHUNK_GENERATOR_HPP
