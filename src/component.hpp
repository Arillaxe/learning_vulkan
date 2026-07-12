#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <stddef.h>

class ComponentTypeIDSystem
{
private:
  static size_t nextTypeID;

public:
  template <typename T>
  static size_t getTypeID()
  {
    static size_t typeID = nextTypeID++;
    return typeID;
  }
};

size_t ComponentTypeIDSystem::nextTypeID = 0;

class Entity;

class Component
{
public:
  enum class State
  {
    Uninitialized,
    Initializing,
    Active,
    Destroying,
    Destroyed
  };

private:
  State state = State::Uninitialized;
  Entity *owner = nullptr;

public:
  virtual ~Component()
  {
    if (state != State::Destroyed)
    {
      onDestroy();
      state = State::Destroyed;
    }
  }

  template <typename T>
  static size_t getTypeID()
  {
    return ComponentTypeIDSystem::getTypeID<T>();
  }

  void init()
  {
    if (state == State::Uninitialized)
    {
      state = State::Initializing;
      onInit();
      state = State::Active;
    }
  }

  void destroy()
  {
    if (state == State::Active)
    {
      state = State::Destroying;
      onDestroy();
      state = State::Destroyed;
    }
  }

  bool isActive() const { return state == State::Active; }

  void setOwner(Entity *entity) { owner = entity; }
  Entity *getOwner() const { return owner; }

protected:
  virtual void onInit() {}
  virtual void onDestroy() {}
  virtual void update(float deltaTime) {}
  virtual void render() {}

  friend class Entity;
};
#endif // COMPONENT_HPP
