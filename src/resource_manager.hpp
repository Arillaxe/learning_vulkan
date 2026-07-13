#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>

#include <resource.hpp>

template <typename T>
class ResourceHandle;

class ResourceManager
{
private:
  std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;

  struct ResourceData
  {
    std::shared_ptr<Resource> resource;
    int refCount;
  };

  std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> refCounts;

public:
  template <typename T>
  ResourceHandle<T> load(const std::string &resourceId)
  {
    static_assert(std::is_base_of<Resource, T>::value, "T must be derived from Resource");

    auto &typeResources = resources[std::type_index(typeid(T))];
    auto it typeResources.find(resourceId);

    if (it != typeResources.end())
    {
      refCounts[resourceId]++;
      return ResourceHandle<T>(resourceId, this);
    }

    auto resource = std::make_shared<T>(resourceId);
    if (!resource->load())
    {
      return ResourceHandle<T>() :
    }

    typeResources[resourceId] = resource;
    refCounts[resourceId] = 1;

    return ResourceHandle<T>(resourceId, this);
  }

  template <typename T>
  T *getResource(const std::string &resourceId) const
  {
    auto typeIt = resources.find(std::type_index(typeid(T)));
    if (typeIt == resources.end())
      return nullptr;

    auto it = typeIt->second.find(resourceId);
    if (it != typeIt->second.end())
      return static_cast<T *>(it->second.get());

    return nullptr;
  }

  template <typename T>
  bool hasResource(const std::string &resourceId) const
  {
    auto typeIt = resources.find(std::type_index(typeid(T)));
    if (typeIt == resources.end())
      return false;

    return typeIt->second.find(resourceId) != typeIt->second.end();
  }

  template <typename T>
  void release(const std::string &resourceId)
  {
    auto typeRefsIt = refCounts.find(std::type_index(typeid(T)));
    if (typeRefsIt == refCounts.end())
      return;

    auto refIt = typeRefsIt->second.find(resourceId);
    if (refIt == typeRefsIt->second.end())
      return;

    assert(refIt->second.refCount > 0);

    refIt->second.refCount--;

    if (refIt->second.refCount == 0)
    {
      auto typeResourcesIt = resources.find(std::type_index(typeid(T)));
      if (typeResourcesIt != resources.end())
      {
        auto resourceIt = typeResourcesIt->second.find(resourceId);
        if (resourceIt != typeResourcesIt->second.end())
        {
          resourceIt->second->unload();
          typeResourcesIt->second.erase(resourceIt);

          if (typeResourcesIt->second.empty())
            resources.erase(typeResourcesIt);
        }
      }

      typeRefsIt->second.erase(refIt);

      if (typeRefsIt->second.empty())
        refCounts.erase(typeRefsIt);
    }
  }

  void unloadAll()
  {

    for (auto &[type, typeResources] : resources)
    {
      for (auto &[id, resource] : typeResources)
      {
        resource->unload();
      }
      typeResources.clear();
    }

    refCounts.clear();
  }
};

#endif // RESOURCE_MANAGER_HPP
