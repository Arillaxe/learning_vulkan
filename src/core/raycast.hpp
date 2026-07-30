#ifndef RAYCAST_HPP
#define RAYCAST_HPP

#include <glm/glm.hpp>
#include <core/world.hpp>

struct RaycastHit
{
  bool hit = false;

  glm::ivec3 voxel;
  glm::ivec3 previousVoxel;

  float distance = 0.0f;
};

RaycastHit raycast(
    World &world,
    const glm::vec3 &origin,
    const glm::vec3 &direction,
    float maxDistance)
{
  RaycastHit result;

  glm::vec3 dir = glm::normalize(direction);

  glm::ivec3 voxel(
      static_cast<int>(std::floor(origin.x / VOXEL_SIZE)),
      static_cast<int>(std::floor(origin.y / VOXEL_SIZE)),
      static_cast<int>(std::floor(origin.z / VOXEL_SIZE)));

  glm::ivec3 previousVoxel = voxel;

  glm::ivec3 step;
  step.x = dir.x >= 0 ? 1 : -1;
  step.y = dir.y >= 0 ? 1 : -1;
  step.z = dir.z >= 0 ? 1 : -1;

  auto intBound = [](float s, float ds)
  {
    if (ds == 0.0f)
      return std::numeric_limits<float>::infinity();

    if (ds > 0.0f)
      return (std::ceil(s) - s) / ds;
    else
      return (s - std::floor(s)) / -ds;
  };

  glm::vec3 voxelPos = origin / (float)VOXEL_SIZE;

  float tMaxX = intBound(voxelPos.x, dir.x);
  float tMaxY = intBound(voxelPos.y, dir.y);
  float tMaxZ = intBound(voxelPos.z, dir.z);

  float tDeltaX = dir.x != 0.0f
                      ? std::abs(1.0f / dir.x)
                      : std::numeric_limits<float>::infinity();

  float tDeltaY = dir.y != 0.0f
                      ? std::abs(1.0f / dir.y)
                      : std::numeric_limits<float>::infinity();

  float tDeltaZ = dir.z != 0.0f
                      ? std::abs(1.0f / dir.z)
                      : std::numeric_limits<float>::infinity();

  float distance = 0.0f;

  while (distance <= maxDistance)
  {
    if (Voxel *v = world.getVoxel(voxel.x, voxel.y, voxel.z))
    {
      if (v->type != 0)
      {
        result.hit = true;
        result.voxel = voxel;
        result.previousVoxel = previousVoxel;
        result.distance = distance * VOXEL_SIZE;
        return result;
      }
    }

    previousVoxel = voxel;

    if (tMaxX < tMaxY)
    {
      if (tMaxX < tMaxZ)
      {
        voxel.x += step.x;
        distance = tMaxX;
        tMaxX += tDeltaX;
      }
      else
      {
        voxel.z += step.z;
        distance = tMaxZ;
        tMaxZ += tDeltaZ;
      }
    }
    else
    {
      if (tMaxY < tMaxZ)
      {
        voxel.y += step.y;
        distance = tMaxY;
        tMaxY += tDeltaY;
      }
      else
      {
        voxel.z += step.z;
        distance = tMaxZ;
        tMaxZ += tDeltaZ;
      }
    }
  }

  return result;
}

#endif // RAYCAST_HPP
