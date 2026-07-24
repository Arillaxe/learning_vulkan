#ifndef MESH_HPP
#define MESH_HPP

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <renderer/vk/vk_context.hpp>
#include <renderer/vertex.hpp>
#include <renderer/vk/vk_resource.hpp>

class Mesh
{
private:
  VkResource &vkResource;
  tinygltf::Model model;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vk::raii::Buffer vertexBuffer = nullptr;
  vk::raii::DeviceMemory vertexBufferMemory = nullptr;
  vk::raii::Buffer indexBuffer = nullptr;
  vk::raii::DeviceMemory indexBufferMemory = nullptr;

  tinygltf::Model loadModel(const std::string &filename)
  {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

    if (!warn.empty())
    {
      std::cout << "glTF warning: " << warn << std::endl;
    }

    if (!err.empty())
    {
      std::cout << "glTF error: " << err << std::endl;
    }

    if (!ret)
    {
      throw std::runtime_error("Failed to load glTF model");
    }

    return model;
  }

  std::vector<Vertex> calcVerticies()
  {
    std::vector<Vertex> vertices;

    vertices.clear();

    for (const auto &mesh : model.meshes)
    {
      for (const auto &primitive : mesh.primitives)
      {
        const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.at("POSITION")];
        const tinygltf::BufferView &posBufferView = model.bufferViews[posAccessor.bufferView];
        const tinygltf::Buffer &posBuffer = model.buffers[posBufferView.buffer];

        bool hasTexCoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
        const tinygltf::Accessor *texCoordAccessor = nullptr;
        const tinygltf::BufferView *texCoordBufferView = nullptr;
        const tinygltf::Buffer *texCoordBuffer = nullptr;

        if (hasTexCoords)
        {
          texCoordAccessor = &model.accessors[primitive.attributes.at("TEXCOORD_0")];
          texCoordBufferView = &model.bufferViews[texCoordAccessor->bufferView];
          texCoordBuffer = &model.buffers[texCoordBufferView->buffer];
        }

        uint32_t baseVertex = static_cast<uint32_t>(vertices.size());

        for (size_t i = 0; i < posAccessor.count; i++)
        {
          Vertex vertex{};

          const float *pos = reinterpret_cast<const float *>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset + i * 12]);
          vertex.position = {pos[0], pos[1], pos[2]};

          if (hasTexCoords)
          {
            const float *texCoord = reinterpret_cast<const float *>(&texCoordBuffer->data[texCoordBufferView->byteOffset + texCoordAccessor->byteOffset + i * 8]);
            vertex.texCoord = {texCoord[0], texCoord[1]};
          }
          else
          {
            vertex.texCoord = {0.0f, 0.0f};
          }

          vertices.push_back(vertex);
        }
      }
    }

    return vertices;
  }

  std::vector<uint32_t> calcIndices()
  {
    std::vector<uint32_t> indices;
    uint32_t vertexOffset = 0;

    for (const auto &mesh : model.meshes)
    {
      for (const auto &primitive : mesh.primitives)
      {
        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexBuffer = model.buffers[indexBufferView.buffer];

        const unsigned char *indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
        size_t indexCount = indexAccessor.count;

        indices.reserve(indices.size() + indexCount);

        for (size_t i = 0; i < indexCount; i++)
        {
          uint32_t index = 0;

          switch (indexAccessor.componentType)
          {
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            index = *reinterpret_cast<const uint16_t *>(indexData + i * sizeof(uint16_t));
            break;
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            index = *reinterpret_cast<const uint32_t *>(indexData + i * sizeof(uint32_t));
            break;
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            index = *reinterpret_cast<const uint8_t *>(indexData + i * sizeof(uint8_t));
            break;

          default:
            throw std::runtime_error("Unsupported index component type");
            break;
          }

          indices.push_back(vertexOffset + index);
        }

        const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.at("POSITION")];
        vertexOffset += static_cast<uint32_t>(posAccessor.count);
      }
    }

    return indices;
  }

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
  Mesh(VkResource &resource, const std::string &filename)
      : vkResource(resource),
        model(loadModel(filename)),
        vertices(calcVerticies()),
        indices(calcIndices())
  {
    createVertexBuffer();
    createIndexBuffer();
  }

  vk::raii::Buffer &getVertexBuffer()
  {
    return vertexBuffer;
  }

  vk::raii::Buffer &getIndexBuffer()
  {
    return indexBuffer;
  }

  std::vector<uint32_t> &getIndices()
  {
    return indices;
  }
};

#endif // MESH_HPP
