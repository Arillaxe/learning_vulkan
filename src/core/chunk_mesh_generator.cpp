#include <core/chunk_mesh_generator.hpp>
#include <bit>

ChunkMeshGenerator::ChunkMeshGenerator(World &w) : world(w) {}

BinaryGrids ChunkMeshGenerator::getBinaryGrids(Chunk &chunk)
{
  BinaryGrids grids{};

  const auto &voxels = chunk.voxels;
  const auto &pos = chunk.pos;

  for (int x = 0; x < CHUNK_SIZE; x++)
  {
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
      for (int z = 0; z < CHUNK_SIZE; z++)
      {
        auto &voxel = voxels[x][y][z];

        if (voxel.type == 0)
          continue;

        grids.xy[x + 1][y + 1] |= (1u << (z + 1));
        grids.xz[x + 1][z + 1] |= (1u << (y + 1));
        grids.yz[y + 1][z + 1] |= (1u << (x + 1));
      }
    }
  }

  if (Chunk *east = world.getChunk(pos.x + 1, pos.z))
  {
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
      for (int z = 0; z < CHUNK_SIZE; z++)
      {
        if (east->voxels[0][y][z].type != 0)
          grids.yz[y + 1][z + 1] |= (1u << (CHUNK_SIZE + 1));
      }
    }
  }

  return grids;
}

ChunkMesh ChunkMeshGenerator::getChunkMesh(Chunk &chunk)
{
  std::vector<Vertex> vertices{};
  std::vector<uint32_t> indices{};

  auto &pos = chunk.pos;

  BinaryGrids occupancy = getBinaryGrids(chunk);

  BinaryGrid poxXfaces{};

  for (int y = 0; y < CHUNK_SIZE; y++)
  {
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
      uint32_t row = occupancy.yz[y + 1][z + 1];
      poxXfaces[y + 1][z + 1] = row & ~(row >> 1);
    }
  }

  BinaryGrid poxXfacesSwizzled{};

  for (int y = 0; y < CHUNK_SIZE; y++)
  {
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
      uint32_t row = poxXfaces[y + 1][z + 1];

      while (row)
      {
        uint32_t x = std::countr_zero(row);
        poxXfacesSwizzled[y + 1][x] |= (1u << z);
        row &= row - 1;
      }
    }
  }

  for (int y = 0; y < CHUNK_SIZE; y++)
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      uint32_t &row = poxXfacesSwizzled[y + 1][x + 1];

      while (row > 0)
      {
        uint32_t offset = std::countr_zero(row);
        uint32_t offsetedRow = row >> offset;
        uint32_t height = std::countr_one(offsetedRow);
        uint32_t mask = ((1u << height) - 1) << offset;

        uint32_t width = 1;
        for (int i = y + 1; i < CHUNK_SIZE; i++)
        {
          if ((poxXfacesSwizzled[i + 1][x + 1] & mask) != mask)
            break;
          width++;
          poxXfacesSwizzled[i + 1][x + 1] &= ~mask;
        }

        uint32_t baseVertex = vertices.size();
        glm::vec3 x1 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y, pos.z * CHUNK_SIZE + (int)offset};
        glm::vec3 x2 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y, pos.z * CHUNK_SIZE + (int)offset + (int)height};
        glm::vec3 y1 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y + (int)width, pos.z * CHUNK_SIZE + (int)offset};
        glm::vec3 y2 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y + (int)width, pos.z * CHUNK_SIZE + (int)offset + (int)height};

        vertices.push_back({x1 * (float)VOXEL_SIZE});
        vertices.push_back({x2 * (float)VOXEL_SIZE});
        vertices.push_back({y1 * (float)VOXEL_SIZE});
        vertices.push_back({y2 * (float)VOXEL_SIZE});

        indices.push_back(baseVertex + 0);
        indices.push_back(baseVertex + 2);
        indices.push_back(baseVertex + 1);

        indices.push_back(baseVertex + 1);
        indices.push_back(baseVertex + 2);
        indices.push_back(baseVertex + 3);

        row &= ~mask;
      }
    }
  }

  return {pos, vertices, indices};
}
