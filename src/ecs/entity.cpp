#include <ecs/entity.hpp>

void Entity::init()
{
  for (auto &component : components)
  {
    component->init();
  }
}

void Entity::update(float deltaTime)
{
  if (!active)
    return;

  for (auto &component : components)
  {
    component->update(deltaTime);
  }
}

void Entity::render()
{
  if (!active)
    return;

  for (auto &component : components)
  {
    component->render();
  }
}
