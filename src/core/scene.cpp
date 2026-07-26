#include <core/scene.hpp>

#include <core/world_generator.hpp>
#include <cstring>
#include <random>
#include <iostream>
#include <core/math.hpp>

Scene::Scene(VkResource &resource, Camera &cam) : vkResource(resource), world(resource), camera(cam)
{
  // for (int i = 0; i < 10; i++)
  // {
  //   for (int j = 0; j < 10; j++)
  //   {
  //     world.loadChunk(i, j);
  //   }
  // }

  // world.generateChunkMeshes();
}

std::vector<glm::ivec2> getChunksAround(glm::vec3 &position)
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

void Scene::update()
{
  glm::vec3 &cameraPosition = camera.position;

  for (auto &coord : getChunksAround(cameraPosition))
  {
    if (world.loadChunk(coord.x, coord.y))
    {
      world.regenNeighboringChunkMeshes(coord.x, coord.y);
      world.generateChunkMesh(coord.x, coord.y);
    }
  }
}
