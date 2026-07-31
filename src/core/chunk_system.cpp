#include <core/chunk_system.hpp>
#include <unordered_set>
#include <core/math.hpp>
#include <core/chunk.hpp>
#include <core/voxel.hpp>
#include <core/chunk_mesh.hpp>
#include <iostream>

ChunkSystem::ChunkSystem(VkResource &resource, Camera &cam, World &w, ThreadQueue<GPUChunkMesh> &lQueue, ThreadQueue<ChunkPos> &uQueue)
    : vkResource(resource), camera(cam), loadQueue(lQueue), unloadQueue(uQueue), world(w), chunkMeshGenerator(w) {}

std::vector<glm::ivec3> ChunkSystem::getChunksAround(glm::vec3 &position)
{
  static int viewDistance = 32;
  constexpr int chunkWorldSize = CHUNK_SIZE * VOXEL_SIZE;

  std::vector<glm::ivec3> coords;

  int centerX = floorDiv((int)position.x, chunkWorldSize);
  int centerY = floorDiv((int)position.y, chunkWorldSize);
  int centerZ = floorDiv((int)position.z, chunkWorldSize);

  for (int cx = centerX - viewDistance; cx <= centerX + viewDistance; cx++)
  {
    for (int cy = 0; cy <= 3; cy++)
    {
      for (int cz = centerZ - viewDistance; cz <= centerZ + viewDistance; cz++)
      {
        int dx = cx - centerX;
        int dy = cy - centerY;
        int dz = cz - centerZ;

        if (dx * dx + dz * dz <= viewDistance * viewDistance)
        {
          coords.emplace_back(glm::ivec3{cx, cy, cz});
        }
      }
    }
  }

  return coords;
}

void ChunkSystem::update()
{
  glm::vec3 &cameraPosition = camera.position;
  // glm::vec3 cameraPosition = glm::vec3(0, 0, 0);

  auto chunks = getChunksAround(cameraPosition);

  for (auto &coord : chunks)
  {
    world.loadChunk(coord.x, coord.y, coord.z);
  }

  std::vector<Vertex> worldVertices;
  std::vector<uint32_t> worldIndices;

  for (auto &coord : chunks)
  {
    auto *chunk = world.getChunk(coord.x, coord.y, coord.z);

    if (!chunk)
      continue;

    if (!chunk->isMeshed)
    {
      chunkMeshes.insert_or_assign(chunk->pos, chunkMeshGenerator.getChunkMesh(*chunk));
      chunk->isMeshed = true;
    }

    auto &mesh = chunkMeshes.find(chunk->pos)->second;

    uint32_t baseIndex = worldVertices.size();
    worldVertices.insert(worldVertices.end(), mesh.vertices.begin(), mesh.vertices.end());

    for (auto index : mesh.indices)
    {
      worldIndices.push_back(baseIndex + index);
    }
  }

  if (worldVertices.size() == 0)
    return;

  GPUChunkMesh gpuMesh(&vkResource, {0});
  gpuMesh.generateRenderMesh(worldVertices, worldIndices);
  loadQueue.push(std::move(gpuMesh));
}
