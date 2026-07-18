#ifndef MESH_RESOURCE_HPP
#define MESH_RESOURCE_HPP

#include <resource.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vertex.hpp>

class Mesh : public Resource
{
private:
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::DeviceSize vertexBufferOffset;
  uint32_t vertexCount = 0;

  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;
  vk::DeviceSize indexBufferOffser;
  uint32_t indexCount = 0;

public:
  explicit Mesh(const std::string &id) : Resource(id) {}

  ~Mesh() override
  {
    unload();
  }

  bool doLoad() override
  {
    std::string filePath = "models/" + getId() + ".gltf";
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    if (!loadMeshData(filePath, vertices, indices))
    {
      return false;
    }

    createVertexBuffer(vertices);
    createIndexBuffer(indices);

    vertexCount = static_cast<uint32_t>(vertices.size());
    indexCount = static_cast<uint32_t>(indices.size());

    return true;
  }

  bool doUnload() override
  {
    if (isLoaded())
    {
      // TODO
    }
  }

  vk::raii::Buffer &getVertextBuffer()
  {
    return vertexBuffer;
  }

  vk::raii::Buffer &getIndexBuffer()
  {
    return indexBuffer;
  }

private:
  bool loadMeshData(std::string &filePath, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
  {
    // TODO
    return true;
  }

  void createVertexBuffer(std::vector<Vertex> &verticies)
  {
    // TODO
  }

  void createIndexBuffer(std::vector<uint32_t> &indices)
  {
    // TODO
  }
};

#endif // MESH_RESOURCE_HPP
