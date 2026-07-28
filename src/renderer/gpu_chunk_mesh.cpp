#include <renderer/gpu_chunk_mesh.hpp>
#include <iostream>

int GPUChunkMesh::destroyed = 0;

GPUChunkMesh::GPUChunkMesh(VkResource *resource, ChunkPos pos) : vkResource(resource), chunkPos(pos) {}

void GPUChunkMesh::generateRenderMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
  // Vertex stuff
  {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto stagingBuffer = (*vkResource).createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc);
    auto stagingBufferMemory = (*vkResource).getBufferMemory(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);

    memcpy(dataStaging, vertices.data(), bufferSize);

    stagingBufferMemory.unmapMemory();

    vertexBuffer = (*vkResource).createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
    vertexMemory = (*vkResource).getBufferMemory(vertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    (*vkResource).copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  }

  // Index stuff
  {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    auto stagingBuffer = (*vkResource).createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc);
    auto stagingBufferMemory = (*vkResource).getBufferMemory(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);

    memcpy(dataStaging, indices.data(), bufferSize);

    stagingBufferMemory.unmapMemory();

    indexBuffer = (*vkResource).createBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
    indexMemory = (*vkResource).getBufferMemory(indexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    (*vkResource).copyBuffer(stagingBuffer, indexBuffer, bufferSize);
  }

  indicesCount = indices.size();
}
