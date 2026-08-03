#include <core/region.hpp>

Region::Region(Chunks &&originalChunks)
{
  chunks[0] = originalChunks;
}

void Region::buildLOD(Chunks &prev, Chunks &next, int prevSize)
{
  const uint32_t nextSize = prevSize / 2;

  next.clear();
  next.reserve(nextSize * nextSize * nextSize);

  for (int i = 0; i < prevSize; i += 2)
  {
    for (int j = 0; j < prevSize; j += 2)
    {
      for (int k = 0; k < prevSize; k += 2)
      {
        Chunk newChunk;

        newChunk.pos = {i / 2, j / 2, k / 2};

        for (auto &offset : offsets)
        {
          auto chunkIt = prev.find({i + offset.x, j + offset.y, k + offset.z});

          for (int a = 0; a < CHUNK_SIZE; a += 2)
          {
            for (int b = 0; b < CHUNK_SIZE; b += 2)
            {
              for (int c = 0; c < CHUNK_SIZE; c += 2)
              {
                Voxel v = {0};
                uint32_t count = 0;

                if (chunkIt != prev.end())
                {
                  auto &chunk = chunkIt->second;

                  for (auto &voxelOffset : offsets)
                  {
                    auto &voxel = chunk.voxels[a + voxelOffset.x][b + voxelOffset.y][c + voxelOffset.z];

                    if (count == 0)
                    {
                      v = voxel;
                    }

                    if (v.type == voxel.type)
                    {
                      count++;
                    }
                    else
                    {
                      count--;
                    }
                  }
                }

                newChunk.voxels[a / 2 + offset.x * CHUNK_SIZE / 2][b / 2 + offset.y * CHUNK_SIZE / 2][c / 2 + offset.z * CHUNK_SIZE / 2] = v;
              }
            }
          }
        }

        next.emplace(newChunk.pos, std::move(newChunk));
      }
    }
  }
}

void Region::buildLODs()
{
  uint32_t size = REGION_SIZE / CHUNK_SIZE;

  for (int i = 0; i < LOD_LEVELS - 1; i++)
  {
    buildLOD(chunks[i], chunks[i + 1], size);
    size /= 2;
  }
}

Chunks &Region::getLODChunks(int lodLevel)
{
  return chunks[lodLevel];
}
