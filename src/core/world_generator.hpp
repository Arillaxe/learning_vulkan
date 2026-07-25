#ifndef WORLD_GENERATOR_HPP
#define WORLD_GENERATOR_HPP

#include <core/voxel.hpp>
#include <renderer/vertex.hpp>
#include <vector>

constexpr int VOXEL_SIZE = 10;

std::vector<Voxel> createWorld()
{
  std::vector<Voxel> voxels;

  voxels.push_back({{0, 0, 0}, 1});

  return voxels;
}

std::array<glm::vec3, 8> offsets = {{
    {-1, 1, -1},
    {1, 1, -1},
    {-1, -1, -1},
    {1, -1, -1},

    {-1, 1, 1},
    {1, 1, 1},
    {-1, -1, 1},
    {1, -1, 1},
}};

std::vector<Vertex> voxelsToVertices(std::vector<Voxel> &voxels)
{
  std::vector<Vertex> vertices;

  for (auto &voxel : voxels)
  {
    for (size_t i = 0; i < offsets.size(); i++)
    {
      glm::vec3 vertexPos = voxel.position;

      vertexPos += offsets[i] * (static_cast<float>(VOXEL_SIZE) / 2.0f);
      vertices.push_back({vertexPos, {0.0, 0.0}});
    }
  }

  return vertices;
}

std::array<uint32_t, 36> voxelIndices = {
    // Front
    0, 3, 2,
    0, 1, 3,

    // Back
    5, 6, 7,
    5, 4, 6,

    // Left
    4, 2, 6,
    4, 0, 2,

    // Right
    1, 7, 3,
    1, 5, 7,

    // Top
    4, 1, 0,
    4, 5, 1,

    // Bottom
    2, 7, 6,
    2, 3, 7};

#endif // WORLD_GENERATOR_HPP
