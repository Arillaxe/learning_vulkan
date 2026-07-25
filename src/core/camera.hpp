#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Camera
{
  glm::vec3 position;
  glm::quat rotation;
};

#endif // CAMERA_HPP
