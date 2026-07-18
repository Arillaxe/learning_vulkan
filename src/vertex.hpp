#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>
#include <vk.hpp>

struct Vertex
{
  glm::vec3 position;
  glm::vec2 texCoord;

  static vk::VertexInputBindingDescription getBindingDescription()
  {
    return {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };
  }

  static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescription()
  {
    return {{
        {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, position),
        },
        {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, texCoord),
        },
    }};
  }
};

#endif // VERTEX_HPP
