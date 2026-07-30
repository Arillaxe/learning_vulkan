#include <core/chunk_system.hpp>
#include <unordered_set>
#include <core/math.hpp>
#include <core/chunk.hpp>
#include <core/voxel.hpp>
#include <core/chunk_mesh.hpp>
#include <iostream>

ChunkSystem::ChunkSystem(VkResource &resource, Camera &cam, World &w, ThreadQueue<GPUChunkMesh> &lQueue, ThreadQueue<ChunkPos> &uQueue)
    : vkResource(resource), camera(cam), loadQueue(lQueue), unloadQueue(uQueue), world(w), chunkMeshGenerator(w) {}

std::vector<glm::ivec2> ChunkSystem::getChunksAround(glm::vec3 &position)
{
  static int viewDistance = 10;
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
  // glm::vec3 &cameraPosition = camera.position;
  glm::vec3 cameraPosition = glm::vec3(0, 0, 0);

  auto chunks = getChunksAround(cameraPosition);
  auto chunksToUnload = getChunksDiff(chunks, prevChunks);

  for (auto &coord : chunksToUnload)
  {
    auto *chunk = world.getChunk(coord.x, coord.y);

    if (!chunk)
      continue;

    chunk->isMeshed = false;

    unloadQueue.push(std::move(chunk->pos));
  }

  static std::array<glm::ivec2, 4> neighbors = {{
      {-1, 0},
      {1, 0},
      {0, -1},
      {0, 1},
  }};

  // Load every chunk in view before meshing any of them, so face culling at chunk
  // borders sees the real neighbor voxels instead of empty space.
  std::vector<glm::ivec2> unmeshedChunks;

  for (auto &coord : chunks)
  {
    auto &chunk = world.loadChunk(coord.x, coord.y);

    if (!chunk.isMeshed)
      unmeshedChunks.push_back(coord);
  }

  std::unordered_set<glm::ivec2, IVec2Hash> dirtyChunks(unmeshedChunks.begin(), unmeshedChunks.end());

  // A neighbor meshed on an earlier tick has stale border faces now that a chunk
  // appeared next to it.
  for (auto &coord : unmeshedChunks)
  {
    for (auto &neighbor : neighbors)
    {
      glm::ivec2 neighborCoord = coord + neighbor;
      auto *chunk = world.getChunk(neighborCoord.x, neighborCoord.y);

      if (chunk && chunk->isMeshed)
        dirtyChunks.insert(neighborCoord);
    }
  }

  for (auto &coord : dirtyChunks)
  {
    auto *chunk = world.getChunk(coord.x, coord.y);

    bool hadMesh = chunk->isMeshed;

    auto mesh = chunkMeshGenerator.getChunkMesh(*chunk);
    if (mesh.vertices.empty())
    {
      if (hadMesh)
      {
        unloadQueue.push(std::move(mesh.chunkPos));
      }
      continue;
    }
    GPUChunkMesh gpuMesh(&vkResource, mesh.chunkPos);
    gpuMesh.generateRenderMesh(mesh.vertices, mesh.indices);
    loadQueue.push(std::move(gpuMesh));

    chunk->isMeshed = true;
  }

  prevChunks = std::move(chunks);
}
