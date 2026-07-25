#ifndef SCENE_HPP
#define SCENE_HPP

#include <ecs/entity.hpp>
#include <core/voxel.hpp>
#include <renderer/vertex.hpp>
#include <renderer/vk/vk_resource.hpp>
#include <utility>
#include <vector>

class Scene
{
private:
  VkResource &vkResource;
  std::vector<Entity> entities;
  std::vector<Voxel> voxels;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  void createVertexBuffer();
  void createIndexBuffer();

public:
  Scene(VkResource &resource);

  void addEntity(Entity &&entity) { entities.push_back(std::move(entity)); }

  std::vector<Entity> &getEntities() { return entities; }
  vk::raii::Buffer &getVertexBuffer() { return vertexBuffer; }
  vk::raii::Buffer &getIndexBuffer() { return indexBuffer; }
  std::vector<Vertex> &getVertices() { return vertices; }
  std::vector<uint32_t> &getIndices() { return indices; }
};

#endif // SCENE_HPP
