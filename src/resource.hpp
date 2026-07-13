#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <string>

class Resource
{
private:
  std::string resourceId;
  bool loaded = false;

public:
  explicit Resource(const std::string &id) : resourceId(id) {}
  virtual ~Resource() = default;

  const std::string &getId() const { return resourceId; }
  bool isLoaded() const { return loaded; }

  bool load()
  {
    loaded = doLoad();

    return loaded;
  }

  void unload()
  {
    doUnload();
    loaded = false;
  }

protected:
  virtual bool doLoad() = 0;
  virtual bool doUnload() = 0;
};

#endif // RESOURCE_HPP
