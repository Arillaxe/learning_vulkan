#include <core/chunk_system.hpp>
#include <unordered_set>
#include <core/math.hpp>
#include <core/chunk.hpp>
#include <core/voxel.hpp>
#include <core/chunk_mesh.hpp>
#include <iostream>

ChunkSystem::ChunkSystem(Camera &cam, World &w, ThreadQueue<ChunkMesh> &lQueue, ThreadQueue<ChunkPos> &uQueue)
    : camera(cam), loadQueue(lQueue), unloadQueue(uQueue), world(w), chunkMeshGenerator(w) {}

std::vector<glm::ivec2> ChunkSystem::getChunksAround(glm::vec3 &position)
{
  static int viewDistance = 12;
  constexpr int chunkWorldSize = CHUNK_SIZE * VOXEL_SIZE;

  std::vector<glm::ivec2> coords;

  int centerX = floorDiv((int)position.x, chunkWorldSize);
  int centerZ = floorDiv((int)position.z, chunkWorldSize);

  for (int cx = centerX - viewDistance; cx <= centerX + viewDistance; cx++)
  {
    for (int cz = centerZ - viewDistance; cz <= centerZ + viewDistance; cz++)
    {
      int dx = cx - centerX;
      int dz = cz - centerZ;

      if (dx * dx + dz * dz <= viewDistance * viewDistance)
      {
        coords.emplace_back(glm::ivec2{cx, cz});
      }
    }
  }

  return coords;
}

std::vector<glm::ivec2> ChunkSystem::getChunksDiff(
    const std::vector<glm::ivec2> &oldChunks,
    const std::vector<glm::ivec2> &newChunks)
{
  std::unordered_set<glm::ivec2, IVec2Hash> oldSet(oldChunks.begin(), oldChunks.end());

  std::vector<glm::ivec2> diff;
  diff.reserve(newChunks.size());

  for (const auto &chunk : newChunks)
  {
    if (!oldSet.contains(chunk))
      diff.push_back(chunk);
  }

  return diff;
}

void ChunkSystem::update()
{
  glm::vec3 &cameraPosition = camera.position;

  auto chunks = getChunksAround(cameraPosition);
  auto chunksToUnload = getChunksDiff(chunks, prevChunks);

  for (auto &coord : chunksToUnload)
  {
    auto *chunk = world.getChunk(coord.x, coord.y);

    if (!chunk)
      continue;

    chunk->isMeshed = false;

    unloadQueue.push(chunk->pos);
  }

  static std::array<glm::ivec2, 4> neighbors = {{
      {-1, 0},
      {1, 0},
      {0, -1},
      {0, 1},
  }};

  for (auto &coord : chunks)
  {
    auto &newChunk = world.loadChunk(coord.x, coord.y);

    if (!newChunk.isMeshed)
    {
      auto mesh = chunkMeshGenerator.getChunkMesh(newChunk);
      loadQueue.push(mesh);
      newChunk.isMeshed = true;

      for (auto &neighbor : neighbors)
      {
        auto *chunk = world.getChunk(coord.x + neighbor.x, coord.y + neighbor.y);

        if (!chunk || !chunk->isMeshed)
          continue;

        auto neighborMesh = chunkMeshGenerator.getChunkMesh(*chunk);
        loadQueue.push(neighborMesh);
      }
    }
  }

  prevChunks = std::move(chunks);
}
