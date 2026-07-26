#include <core/chunk.hpp>

Chunk::Chunk(VkResource &resource) : vkResource(resource)
{
}

void Chunk::calculateBuffers()
{
  createVertexBuffer();
  createIndexBuffer();
}

void Chunk::createVertexBuffer()
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

void Chunk::createIndexBuffer()
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
