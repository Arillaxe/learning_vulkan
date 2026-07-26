#include <core/world_generator.hpp>

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

std::vector<Voxel> createWorld()
{
  std::vector<Voxel> voxels;

  voxels.push_back({{0, 0, 0}, 1});

  return voxels;
}

struct Direction
{
  glm::vec3 forward;
  std::array<glm::vec3, 4> vertexOffsets;
};

constexpr std::array<Direction, 6> directions = {{
    {{0, 0, -1}, {{{-1, 1, -1}, {1, 1, -1}, {1, -1, -1}, {-1, -1, -1}}}}, // Front (-Z)
    {{0, 0, 1}, {{{1, 1, 1}, {-1, 1, 1}, {-1, -1, 1}, {1, -1, 1}}}},      // Back (+Z)
    {{-1, 0, 0}, {{{-1, 1, 1}, {-1, 1, -1}, {-1, -1, -1}, {-1, -1, 1}}}}, // Left (-X)
    {{1, 0, 0}, {{{1, 1, -1}, {1, 1, 1}, {1, -1, 1}, {1, -1, -1}}}},      // Right (+X)
    {{0, 1, 0}, {{{-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1}}}},      // Top (+Y)
    {{0, -1, 0}, {{{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}}}}, // Bottom (-Y)
}};

constexpr std::array<uint32_t, 6> faceIndices = {{0, 1, 2, 0, 2, 3}};

std::pair<std::vector<Vertex>, std::vector<uint32_t>> chunkToVertices(World &world, Chunk &chunk, const ChunkPos &pos)
{
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  auto &voxels = chunk.getVoxels();

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

  return {vertices, indices};
}
