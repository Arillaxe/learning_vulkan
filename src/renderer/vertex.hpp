#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>

struct Vertex
{
  glm::vec3 position;
  glm::vec2 texCoord;
  float shade;
};

#endif // VERTEX_HPP
