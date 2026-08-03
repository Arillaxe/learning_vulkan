#ifndef REGION_HPP
#define REGION_HPP

#include <core/chunk.hpp>

typedef std::unordered_map<ChunkPos, Chunk, ChunkPosHash> Chunks;

constexpr int LOD_LEVELS = 5;
constexpr int REGION_SIZE = 256;

static constexpr std::array<glm::ivec3, 8> offsets = {{
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {0, 1, 1},
    {1, 1, 1},
}};

class Region
{
private:
  std::array<Chunks, LOD_LEVELS> chunks;

  void buildLOD(Chunks &prev, Chunks &next, int prevSize);

public:
  Region(Chunks &&originalChunks);

  void buildLODs();
  Chunks &getLODChunks(int lodLevel);
};

#endif // REGION_HPP
