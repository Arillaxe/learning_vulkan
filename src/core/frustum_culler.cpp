#include <core/frustum_culler.hpp>

Plane makePlane(float a, float b, float c, float d)
{
  glm::vec3 normal(a, b, c);
  float length = glm::length(normal);

  return {
      normal / length,
      d / length,
  };
}

FrustumCuller::FrustumCuller(const Camera &camera, float aspectRatio)
{
  glm::mat4 view = glm::mat4_cast(glm::conjugate(camera.rotation)) *
                   glm::translate(glm::mat4(1.0f), -camera.position);
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100000.0f);
  glm::mat4 vp = projection * view;

  // Left
  planes[0] = makePlane(
      vp[0][3] + vp[0][0],
      vp[1][3] + vp[1][0],
      vp[2][3] + vp[2][0],
      vp[3][3] + vp[3][0]);

  // Right
  planes[1] = makePlane(
      vp[0][3] - vp[0][0],
      vp[1][3] - vp[1][0],
      vp[2][3] - vp[2][0],
      vp[3][3] - vp[3][0]);

  // Bottom
  planes[2] = makePlane(
      vp[0][3] + vp[0][1],
      vp[1][3] + vp[1][1],
      vp[2][3] + vp[2][1],
      vp[3][3] + vp[3][1]);

  // Top
  planes[3] = makePlane(
      vp[0][3] - vp[0][1],
      vp[1][3] - vp[1][1],
      vp[2][3] - vp[2][1],
      vp[3][3] - vp[3][1]);

  // Near
  planes[4] = makePlane(
      vp[0][3] + vp[0][2],
      vp[1][3] + vp[1][2],
      vp[2][3] + vp[2][2],
      vp[3][3] + vp[3][2]);

  // Far
  planes[5] = makePlane(
      vp[0][3] - vp[0][2],
      vp[1][3] - vp[1][2],
      vp[2][3] - vp[2][2],
      vp[3][3] - vp[3][2]);
}

bool FrustumCuller::testAABB(const glm::vec3 &min,
                             const glm::vec3 &max) const
{
  for (const Plane &plane : planes)
  {
    glm::vec3 p;

    p.x = plane.normal.x >= 0.0f ? max.x : min.x;
    p.y = plane.normal.y >= 0.0f ? max.y : min.y;
    p.z = plane.normal.z >= 0.0f ? max.z : min.z;

    if (glm::dot(plane.normal, p) + plane.d < 0.0f)
      return false;
  }

  return true;
}

bool FrustumCuller::isVisible(const ChunkPos &chunkPos) const
{
  glm::vec3 min(
      chunkPos.x * CHUNK_SIZE * VOXEL_SIZE,
      chunkPos.y * CHUNK_SIZE * VOXEL_SIZE,
      chunkPos.z * CHUNK_SIZE * VOXEL_SIZE);

  glm::vec3 max = min + glm::vec3(CHUNK_SIZE * VOXEL_SIZE);

  return testAABB(min, max);
}
