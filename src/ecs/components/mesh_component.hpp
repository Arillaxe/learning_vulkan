#ifndef MESH_COMPONENT_HPP
#define MESH_COMPONENT_HPP

#include <ecs/component.hpp>
#include <ecs/components/transform_component.hpp>
#include <ecs/entity.hpp>
#include <core/mesh.hpp>
#include <core/material.hpp>

class MeshComponent : public Component
{
private:
  Mesh &mesh;
  Material &material;

public:
  MeshComponent(Mesh &_mesh, Material &mat)
      : mesh(_mesh),
        material(mat) {}

  Mesh &getMesh()
  {
    return mesh;
  }

  Material &getMaterial()
  {
    return material;
  }
};

#endif // MESH_COMPONENT_HPP
