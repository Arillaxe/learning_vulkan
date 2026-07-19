#ifndef SCENE_HPP
#define SCENE_HPP

#include <ecs/entity.hpp>

class Scene
{
private:
  std::vector<Entity> entities;

public:
  Scene() {}

  void addEntity(Entity &&entity)
  {
    entities.push_back(std::move(entity));
  }

  std::vector<Entity> &getEntities()
  {
    return entities;
  }
};

#endif // SCENE_HPP
