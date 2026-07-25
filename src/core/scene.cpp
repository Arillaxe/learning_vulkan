#include <core/scene.hpp>

#include <core/world_generator.hpp>
#include <cstring>

Scene::Scene(VkResource &resource) : vkResource(resource)
{
  for (int i = 0; i < 5; i++)
  {
    voxels.push_back({{0, i * 10, 0}});
  }

  vertices = voxelsToVertices(voxels);
  indices.assign(voxelIndices.begin(), voxelIndices.end());

  createVertexBuffer();
  createIndexBuffer();
}

void Scene::createVertexBuffer()
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

void Scene::createIndexBuffer()
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
