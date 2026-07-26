#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <core/voxel.hpp>

constexpr int CHUNK_SIZE = 16;

struct ChunkPos
{
  int x;
  int y;
  int z;

  bool operator==(const ChunkPos &) const = default;
};

struct ChunkPosHash
{
  std::size_t operator()(const ChunkPos &pos) const
  {
    std::size_t h1 = std::hash<int>{}(pos.x);
    std::size_t h2 = std::hash<int>{}(pos.y);
    std::size_t h3 = std::hash<int>{}(pos.z);

    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};

struct Chunk
{
  ChunkPos pos;
  Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
  bool isMeshed = false;
};

#endif // CHUNK_HPP
