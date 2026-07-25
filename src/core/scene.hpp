#ifndef SCENE_HPP
#define SCENE_HPP

#include <ecs/entity.hpp>
#include <core/voxel.hpp>
#include <renderer/vertex.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <utility>
#include <vector>
#include <core/chunk.hpp>

class Scene
{
private:
  VkResource &vkResource;
  std::vector<Entity> entities;
  std::vector<Chunk> chunks;

public:
  Scene(VkResource &resource);

  void addEntity(Entity &&entity) { entities.push_back(std::move(entity)); }

  std::vector<Entity> &getEntities() { return entities; }
  std::vector<Chunk> &getChunks() { return chunks; }
};

#endif // SCENE_HPP
