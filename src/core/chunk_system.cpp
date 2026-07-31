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
  constexpr int MIN_CHUNK_Y = 0;
  constexpr int MAX_CHUNK_Y = 10;

  constexpr std::array<glm::ivec3, 6> NEIGHBOR_OFFSETS = {{
      {1, 0, 0},
      {-1, 0, 0},
      {0, 1, 0},
      {0, -1, 0},
      {0, 0, 1},
      {0, 0, -1},
  }};

  bool neighborsResolved(World &world, const ChunkPos &pos)
  {
    for (const auto &offset : NEIGHBOR_OFFSETS)
    {
      const int ny = pos.y + offset.y;

      if (ny < MIN_CHUNK_Y || ny > MAX_CHUNK_Y)
        continue;

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

ChunkSystem::ChunkSystem(VkResource &resource, Camera &cam, World &w, ThreadQueue<GPUChunkMesh> &lQueue, ThreadQueue<ChunkPos> &uQueue)
    : vkResource(resource), camera(cam), loadQueue(lQueue), unloadQueue(uQueue), world(w), chunkMeshGenerator(w) {}

std::vector<glm::ivec3> ChunkSystem::getChunksAround(glm::vec3 &position)
{
  constexpr int chunkWorldSize = CHUNK_SIZE * VOXEL_SIZE;

  std::vector<glm::ivec3> coords;

  int centerX = floorDiv((int)position.x, chunkWorldSize);
  int centerZ = floorDiv((int)position.z, chunkWorldSize);

  for (int cx = centerX - LOAD_DISTANCE; cx <= centerX + LOAD_DISTANCE; cx++)
  {
    for (int cy = MIN_CHUNK_Y; cy <= MAX_CHUNK_Y; cy++)
    {
      for (int cz = centerZ - LOAD_DISTANCE; cz <= centerZ + LOAD_DISTANCE; cz++)
      {
        int dx = cx - centerX;
        int dz = cz - centerZ;

        if (dx * dx + dz * dz <= LOAD_DISTANCE * LOAD_DISTANCE)
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

  auto chunks = getChunksAround(cameraPosition);

  constexpr int chunkWorldSize = CHUNK_SIZE * VOXEL_SIZE;
  int centerX = floorDiv((int)cameraPosition.x, chunkWorldSize);
  int centerZ = floorDiv((int)cameraPosition.z, chunkWorldSize);

  for (auto &coord : chunks)
  {
    if (!world.loadChunk(coord.x, coord.y, coord.z))
      continue;

    for (const auto &offset : NEIGHBOR_OFFSETS)
    {
      dirtyChunkMesh(world.getChunk(coord.x + offset.x, coord.y + offset.y, coord.z + offset.z));
    }
  }

  std::vector<Vertex> worldVertices;
  std::vector<uint32_t> worldIndices;

  for (auto &coord : chunks)
  {
    int dx = coord.x - centerX;
    int dz = coord.z - centerZ;
    if (dx * dx + dz * dz > VIEW_DISTANCE * VIEW_DISTANCE)
      continue;

    auto *chunk = world.getChunk(coord.x, coord.y, coord.z);

    if (!chunk)
      continue;

    if (!chunk->isMeshed)
    {
      if (!neighborsResolved(world, chunk->pos))
        continue;

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
