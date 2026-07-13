#ifndef RESOURCE_HANDLE_HPP
#define RESOURCE_HANDLE_HPP

#include <string>

#include <resource_manager.hpp>

template <typename T>
class ResourceHandle
{
private:
  std::string resourceId;
  ResourceManager *resourceManager;

public:
  ResourceHandle() : resourceManager(nullptr) {}

  ResourceHandle(const std::string &id, ResourceManager *manager) : resourceId(id), resourceManager(manager) {}

  T *get() const
  {
    if (!resourceManager)
      return nullptr;
    return resourceManager->getResource<T>(resourceId);
  }

  bool isValid() const
  {
    return resourceManager && resourceManager->hasResource<T>(resourceId);
  }

  const std::string &getId() const
  {
    return resourceId;
  }

  T *operator->() const
  {
    return get();
  }

  T &operator*() const
  {
    return *get();
  }

  operator bool() const
  {
    return isValid();
  }
};

#endif // RESOURCE_HANDLE_HPP
