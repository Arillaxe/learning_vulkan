#ifndef VOXEL_HPP
#define VOXEL_HPP

#include <glm/glm.hpp>

struct Voxel
{
  glm::vec3 position;
  uint32_t type;
};

#endif // VOXEL_HPP
