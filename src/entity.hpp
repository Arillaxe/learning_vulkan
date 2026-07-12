#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <component.hpp>

class Entity
{
private:
  std::string name;
  bool active = true;
  std::vector<std::unique_ptr<Component>> components;
  std::unordered_map<size_t, Component *> componentMap;

public:
  explicit Entity(const std::string &entityName) : name(entityName) {}

  const std::string &getName() const { return name; }
  bool isActive() const { return active; }
  void setActive(bool isActive) { active = isActive; };

  void init()
  {
    for (auto &component : components)
    {
      component->init();
    }
  }

  void update(float deltaTime)
  {
    if (!active)
      return;

    for (auto &component : components)
    {
      component->update(deltaTime);
    }
  }

  void render()
  {
    if (!active)
      return;

    for (auto &component : components)
    {
      component->render();
    }
  }

  template <typename T, typename... Args>
  T *addComponent(Args &&...args)
  {
    static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

    size_t typeID = Component::getTypeID<T>();

    auto it = componentMap.find(typeID);
    if (it != componentMap.end())
    {
      return static_cast<T *>(it->second);
    }

    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    T *componentPtr = component.get();
    componentMap[typeID] = componentPtr;
    componentPtr->setOwner(this);
    components.push_back(std::move(component));

    return componentPtr;
  }

  template <typename T>
  T *getComponent()
  {
    size_t = typeID = Component::getTypeID<T>();
    auto it = componentMap.find(typeID);

    if (it != componentMap.end())
    {
      return static_cast<T *>(it->second);
    }

    return nullptr;
  }

  template <typename T>
  bool removeComponent()
  {
    size_t typeID = Component::getTypeID<T>();
    auto it = componentMap.find(typeID);
    if (it != componentMap.end())
    {
      Component *componentPtr = it->second;
      componentMap.erase(it);

      for (auto compIt = components.begin(); compIt != components.end(); ++compIt)
      {
        if (compIt->get() == componentPtr)
        {
          components.erase(compIt);
          return true;
        }
      }
    }

    return false;
  }
};

#endif // ENTITY_HPP
