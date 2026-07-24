#ifndef MESH_COMPONENT_HPP
#define MESH_COMPONENT_HPP

#include <ecs/component.hpp>
#include <ecs/components/transform_component.hpp>
#include <ecs/entity.hpp>
#include <core/mesh.hpp>

class MeshComponent : public Component
{
private:
  Mesh &mesh;

public:
  MeshComponent(Mesh &_mesh)
      : mesh(_mesh) {}

  Mesh &getMesh()
  {
    return mesh;
  }
};

#endif // MESH_COMPONENT_HPP
