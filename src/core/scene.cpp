#include <core/scene.hpp>

#include <core/world_generator.hpp>
#include <cstring>

Scene::Scene(VkResource &resource) : vkResource(resource)
{
  Chunk chunk(resource);

  auto &voxels = chunk.getVoxels();

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      for (int k = 0; k < CHUNK_SIZE; k++)
      {
        voxels[i][j][k] = {glm::vec3(i * 10, j * 10, k * 10), 1};
      }
    }
  }

  chunk.calculateBuffers();

  chunks.push_back(std::move(chunk));
}
