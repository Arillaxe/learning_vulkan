#include <core/scene.hpp>

#include <core/world_generator.hpp>
#include <cstring>
#include <random>
#include <iostream>
#include <core/math.hpp>

Scene::Scene(VkResource &resource, Camera &cam) : vkResource(resource), world(resource), camera(cam)
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      world.loadChunk(i, j);
    }
  }

  world.generateChunkMeshes();
}

std::array<glm::ivec2, 4> chunkOffsets = {{{0, 0}, {0, CHUNK_SIZE}, {CHUNK_SIZE, 0}, {CHUNK_SIZE, CHUNK_SIZE}}};

std::vector<glm::ivec2> getChunksAround(glm::vec3 &position)
{
  static int viewDistance = 3;

  std::vector<glm::ivec2> coords;

  glm::ivec2 v1{(int)position.x - viewDistance * CHUNK_SIZE, (int)position.z - viewDistance * CHUNK_SIZE};
  glm::ivec2 v2{(int)position.x + viewDistance * CHUNK_SIZE, (int)position.z + viewDistance * CHUNK_SIZE};
  float radius = viewDistance * CHUNK_SIZE;

  for (int i = v1.x; i <= v2.x; i += CHUNK_SIZE)
  {
    for (int j = v1.y; j <= v2.y; j += CHUNK_SIZE)
    {
      for (auto &offset : chunkOffsets)
      {
        float distance = glm::distance(glm::vec2{position.x, position.z}, glm::vec2(i + offset.x, j + offset.y));

        if (distance <= radius)
        {
          int chunkX = floorDiv(i, CHUNK_SIZE);
          int chunkY = floorDiv(j, CHUNK_SIZE);
          coords.emplace_back(glm::ivec2{chunkX, chunkY});

          break;
        }
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
      world.generateChunkMesh(coord.x, coord.y);
    }
  }
}
