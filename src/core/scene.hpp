#ifndef SCENE_HPP
#define SCENE_HPP

#include <ecs/entity.hpp>
#include <core/voxel.hpp>
#include <renderer/vertex.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <utility>
#include <vector>
#include <core/chunk.hpp>
#include <core/world.hpp>
#include <core/camera.hpp>

class Scene
{
private:
  VkResource &vkResource;
  std::vector<Entity> entities;
  World world;
  Camera &camera;

public:
  Scene(VkResource &resource, Camera &cam);

  void addEntity(Entity &&entity) { entities.push_back(std::move(entity)); }

  std::vector<Entity> &getEntities() { return entities; }
  World &getWorld() { return world; }
  void update();
};

#endif // SCENE_HPP
