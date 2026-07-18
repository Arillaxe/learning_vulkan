#ifndef MESH_COMPONENT_HPP
#define MESH_COMPONENT_HPP

#include <component.hpp>
#include <transform_component.hpp>
#include <entity.hpp>
#include <buffer.hpp>
#include <mesh_resource.hpp>

class Material;

class MeshComponent : public Component
{
private:
  Mesh *mesh = nullptr;
  Material *material = nullptr;

public:
  MeshComponent(Mesh *m, Material *mat) : mesh(m), material(mat) {}

  void setMesh(Mesh *m) { mesh = m; }
  void setMaterial(Material *mat) { material = mat; }

  Mesh *getMesh() { return mesh; }
  Material *getMaterial() { return material; }

  void render() override
  {
    if (!mesh || !material)
      return;

    auto transform = getOwner()->getComponent<TransformComponent>();
    if (!transform)
      return;

    // material->bind();
    // material->setUniform("modelMatrix", transform->getTransformMatrix());
    // mesh->render();
  }
};

#endif // MESH_COMPONENT_HPP
