#ifndef UBO_HPP
#define UBO_HPP

#include <glm/glm.hpp>

struct UniformBufferObject
{
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec3 cameraPosition;
  float deltaTime;
};

#endif // UBO_HPP
