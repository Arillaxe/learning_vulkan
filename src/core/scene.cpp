#include <core/scene.hpp>

#include <core/world_generator.hpp>
#include <cstring>
#include <random>
#include <iostream>

Scene::Scene(VkResource &resource) : vkResource(resource), world(resource)
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      world.generateChunk(i, j);
    }
  }

  world.generateChunkMeshes();
}
