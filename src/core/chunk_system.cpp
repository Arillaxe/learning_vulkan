#include <core/chunk_system.hpp>
#include <array>
#include <core/math.hpp>
#include <core/chunk.hpp>
#include <core/voxel.hpp>
#include <core/chunk_mesh.hpp>
#include <iostream>

namespace
{
  constexpr int VIEW_DISTANCE = 12;
  constexpr int LOAD_DISTANCE = VIEW_DISTANCE + 1;

  constexpr std::array<glm::ivec3, 6> NEIGHBOR_OFFSETS = {{
      {1, 0, 0},
      {-1, 0, 0},
      {0, 1, 0},
      {0, -1, 0},
      {0, 0, 1},
      {0, 0, -1},
  }};

  bool neighborsResolved(World &world, const glm::ivec3 &pos)
  {
    for (const auto &offset : NEIGHBOR_OFFSETS)
    {
      const int ny = pos.y + offset.y;

      // if (ny < MIN_CHUNK_Y || ny > MAX_CHUNK_Y)
      //   continue;

      if (!world.isChunkResolved(pos.x + offset.x, ny, pos.z + offset.z))
        return false;
    }

    return true;
  }

  void dirtyChunkMesh(Chunk *chunk)
  {
    if (chunk)
      chunk->isMeshed = false;
  }
} // namespace

ChunkSystem::ChunkSystem(VkResource &resource, Camera &cam, World &w, ThreadQueue<GPUChunkMesh> &lQueue, ThreadQueue<glm::ivec3> &uQueue)
    : vkResource(resource), camera(cam), loadQueue(lQueue), unloadQueue(uQueue), world(w), chunkMeshGenerator(w) {}

std::vector<glm::ivec2> ChunkSystem::getChunksAround(glm::vec3 &position)
{
  constexpr int chunkWorldSize = CHUNK_SIZE * VOXEL_SIZE;

  std::vector<glm::ivec2> coords;

  int centerX = floorDiv((int)position.x, chunkWorldSize);
  int centerZ = floorDiv((int)position.z, chunkWorldSize);

  for (int cx = centerX - LOAD_DISTANCE; cx <= centerX + LOAD_DISTANCE; cx++)
  {
    for (int cz = centerZ - LOAD_DISTANCE; cz <= centerZ + LOAD_DISTANCE; cz++)
    {
      int dx = cx - centerX;
      int dz = cz - centerZ;

      if (dx * dx + dz * dz <= LOAD_DISTANCE * LOAD_DISTANCE)
      {
        coords.emplace_back(glm::ivec2{cx, cz});
      }
    }
  }

  return coords;
}

void ChunkSystem::update()
{
  // glm::vec3 &cameraPosition = camera.position;
  glm::vec3 cameraPosition = glm::vec3(0, 0, 0);

  constexpr int chunkWorldSize = CHUNK_SIZE * VOXEL_SIZE;
  int centerX = floorDiv((int)cameraPosition.x, chunkWorldSize);
  int centerZ = floorDiv((int)cameraPosition.z, chunkWorldSize);

  auto chunks = getChunksAround(cameraPosition);

  for (auto &coord : chunks)
  {
    world.loadChunk(coord.x, coord.y);
  }

  for (auto &coord : chunks)
  {
    auto yChunks = world.getRenderChunks(coord.x, coord.y);

    for (auto *chunk : yChunks)
    {

      if (!chunk->isMeshed)
      {
        // if (!neighborsResolved(world, chunk->pos))
        //   continue;

        chunkMeshes.insert_or_assign(chunk->pos, chunkMeshGenerator.getChunkMesh(*chunk, 1));
        chunk->isMeshed = true;

        auto &mesh = chunkMeshes.find(chunk->pos)->second;

        GPUChunkMesh gpuMesh(&vkResource, chunk->pos);
        gpuMesh.generateRenderMesh(mesh.vertices, mesh.indices);
        loadQueue.push(std::move(gpuMesh));
      }
    }
  }
}
