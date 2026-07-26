#ifndef VOXEL_HPP
#define VOXEL_HPP

#include <glm/glm.hpp>

constexpr int VOXEL_SIZE = 10;

struct Voxel
{
  glm::vec3 position;
  uint32_t type;
};

#endif // VOXEL_HPP
