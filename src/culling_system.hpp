#ifndef CULLING_SYSTEM_HPP
#define CULLING_SYSTEM_HPP

#include <entity.hpp>
#include <vector>
#include <camera.hpp>

class CullingSystem
{
  std::vector<Entity *> entities;
  Camera *camera = nullptr;

public:
  std::vector<Entity *> &GetVisibleEntities()
  {
    return entities;
  }

  void SetCamera(Camera *cam)
  {
    camera = cam;
  }

  void CullScene(const std::vector<Entity *> &entities)
  {
  }
};

#endif // CULLING_SYSTEM_HPP
