#ifndef SCENE_HPP
#define SCENE_HPP

#include <ecs/entity.hpp>
#include <core/voxel.hpp>
#include <core/world_generator.hpp>
#include <renderer/vk/vk_resource.hpp>

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

  void createVertexBuffer()
  {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto stagingBuffer = vkResource.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc);
    auto stagingBufferMemory = vkResource.getBufferMemory(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);

    memcpy(dataStaging, vertices.data(), bufferSize);

    stagingBufferMemory.unmapMemory();

    vertexBuffer = vkResource.createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
    vertexBufferMemory = vkResource.getBufferMemory(vertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vkResource.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  }

  void createIndexBuffer()
  {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    auto stagingBuffer = vkResource.createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc);
    auto stagingBufferMemory = vkResource.getBufferMemory(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);

    memcpy(dataStaging, indices.data(), bufferSize);

    stagingBufferMemory.unmapMemory();

    indexBuffer = vkResource.createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
    indexBufferMemory = vkResource.getBufferMemory(indexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vkResource.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
  }

public:
  Scene(VkResource &resource) : vkResource(resource)
  {
    for (int i = 0; i < 5; i++)
    {
      voxels.push_back({{0, i, 0}});
    }

    vertices = voxelsToVertices(voxels);
    indices.assign(voxelIndices.begin(), voxelIndices.end());

    createVertexBuffer();
    createIndexBuffer();
  }

  void addEntity(Entity &&entity)
  {
    entities.push_back(std::move(entity));
  }

  std::vector<Entity> &getEntities()
  {
    return entities;
  }

  vk::raii::Buffer &getVertexBuffer()
  {
    return vertexBuffer;
  }

  vk::raii::Buffer &getIndexBuffer()
  {
    return indexBuffer;
  }

  std::vector<Vertex> &getVertices()
  {
    return vertices;
  }

  std::vector<uint32_t> &getIndices()
  {
    return indices;
  }
};

#endif // SCENE_HPP
