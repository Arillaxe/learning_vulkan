#include <core/chunk_mesh_generator.hpp>
#include "chunk_mesh_generator.hpp"

ChunkMeshGenerator::ChunkMeshGenerator(World &w) : world(w)
{
}

ChunkMesh ChunkMeshGenerator::getChunkMesh(Chunk &chunk)
{
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  auto &voxels = chunk.voxels;
  auto &pos = chunk.pos;

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      for (int k = 0; k < CHUNK_SIZE; k++)
      {
        if (voxels[i][j][k].type == 0)
        {
          continue;
        }

        for (int d = 0; d < directions.size(); d++)
        {
          auto &direction = directions[d];

          auto *voxel = world.getVoxel(pos.x * CHUNK_SIZE + i + (int)direction.forward.x, pos.y * CHUNK_SIZE + j + (int)direction.forward.y, pos.z * CHUNK_SIZE + k + (int)direction.forward.z);

          if (voxel != nullptr && voxel->type != 0)
          {
            continue;
          }

          glm::vec3 vertexPos = voxels[i][j][k].position;

          uint32_t base = static_cast<uint32_t>(vertices.size());

          for (auto &offset : direction.vertexOffsets)
          {
            vertices.push_back({vertexPos + offset * (static_cast<float>(VOXEL_SIZE) / 2.0f), {0.0, 0.0}});
          }

          for (auto &index : faceIndices)
          {
            indices.push_back(base + index);
          }
        }
      }
    }
  }

  return {pos, vertices, indices};
}
