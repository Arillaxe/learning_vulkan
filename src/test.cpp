#include <iostream>
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <bitset>
#include <cassert>
#include <cmath>

struct Vertex
{
  glm::vec3 position;
  glm::vec2 texCoord;
};

struct Voxel
{
  glm::vec3 position;
  uint32_t type;
};

constexpr int VOXEL_SIZE = 1;

constexpr int CHUNK_SIZE = 4;

struct ChunkPos
{
  int x;
  int y;
  int z;

  bool operator==(const ChunkPos &) const = default;
};

struct Chunk
{
  ChunkPos pos;
  Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
  bool isMeshed = false;
};

using BinaryGrid = std::array<std::array<uint32_t, CHUNK_SIZE>, CHUNK_SIZE>;

struct BinaryGrids
{
  BinaryGrid xy{};
  BinaryGrid xz{};
  BinaryGrid yz{};
};

struct ChunkMesh
{
  ChunkPos chunkPos;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

BinaryGrids getBinaryGrids(Chunk &chunk)
{
  BinaryGrids grids;

  const auto &voxels = chunk.voxels;

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      for (int k = 0; k < CHUNK_SIZE; k++)
      {
        auto &voxel = voxels[i][j][k];

        if (voxel.type == 0)
          continue;

        grids.xy[i][j] |= (1u << k);
        grids.xz[i][k] |= (1u << j);
        grids.yz[j][k] |= (1u << i);
      }
    }
  }

  return grids;
}

ChunkMesh getChunkMesh(Chunk &chunk)
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
      uint32_t row = occupancy.yz[y][z];
      poxXfaces[y][z] = row & ~(row >> 1);
    }
  }

  BinaryGrid poxXfacesSwizzled{};

  for (int y = 0; y < CHUNK_SIZE; y++)
  {
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
      uint32_t row = poxXfaces[y][z];

      while (row)
      {
        uint32_t x = std::countr_zero(row);
        poxXfacesSwizzled[y][x] |= (1u << z);
        row &= row - 1;
      }
    }
  }

  for (int y = 0; y < CHUNK_SIZE; y++)
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      uint32_t &row = poxXfacesSwizzled[y][x];

      while (row > 0)
      {
        uint32_t offset = std::countr_zero(row);
        uint32_t offsetedRow = row >> offset;
        uint32_t height = std::countr_one(offsetedRow);
        uint32_t mask = ((1u << height) - 1) << offset;

        uint32_t width = 1;
        for (int i = y + 1; i < CHUNK_SIZE; i++)
        {
          if ((poxXfacesSwizzled[i][x] & mask) != mask)
            break;
          width++;
          poxXfacesSwizzled[i][x] &= ~mask;
        }

        uint32_t baseVertex = vertices.size();
        glm::vec3 x1 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y, pos.z * CHUNK_SIZE + offset};
        glm::vec3 x2 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y, pos.z * CHUNK_SIZE + offset + height};
        glm::vec3 y1 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y + width, pos.z * CHUNK_SIZE + offset};
        glm::vec3 y2 = {pos.x * CHUNK_SIZE + x + 1, pos.y * CHUNK_SIZE + y + width, pos.z * CHUNK_SIZE + offset + height};

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

void validatePositiveXMesh(const Chunk &chunk, const ChunkMesh &mesh)
{
  assert(mesh.vertices.size() % 4 == 0);
  assert(mesh.indices.size() % 6 == 0);
  assert(mesh.indices.size() / 6 == mesh.vertices.size() / 4);

  for (uint32_t index : mesh.indices)
  {
    if (index >= mesh.vertices.size())
    {
      std::cerr << "INVALID INDEX: " << index
                << ", vertex count: " << mesh.vertices.size() << '\n';
      std::abort();
    }
  }

  uint32_t expectedUnitFaces = 0;

  for (int x = 0; x < CHUNK_SIZE; ++x)
  {
    for (int y = 0; y < CHUNK_SIZE; ++y)
    {
      for (int z = 0; z < CHUNK_SIZE; ++z)
      {
        if (chunk.voxels[x][y][z].type == 0)
          continue;

        const bool positiveNeighborSolid =
            x + 1 < CHUNK_SIZE &&
            chunk.voxels[x + 1][y][z].type != 0;

        if (!positiveNeighborSolid)
          ++expectedUnitFaces;
      }
    }
  }

  uint32_t emittedUnitFaces = 0;

  for (size_t i = 0; i < mesh.vertices.size(); i += 4)
  {
    const glm::vec3 &v0 = mesh.vertices[i + 0].position;
    const glm::vec3 &v1 = mesh.vertices[i + 1].position;
    const glm::vec3 &v2 = mesh.vertices[i + 2].position;
    const glm::vec3 &v3 = mesh.vertices[i + 3].position;

    std::cout << "Quad " << i / 4 << ":\n"
              << "  v0 = (" << v0.x << ", " << v0.y << ", " << v0.z << ")\n"
              << "  v1 = (" << v1.x << ", " << v1.y << ", " << v1.z << ")\n"
              << "  v2 = (" << v2.x << ", " << v2.y << ", " << v2.z << ")\n"
              << "  v3 = (" << v3.x << ", " << v3.y << ", " << v3.z << ")\n";

    // A +X quad must lie entirely on one X plane.
    assert(v0.x == v1.x);
    assert(v0.x == v2.x);
    assert(v0.x == v3.x);

    const float yExtent = std::abs(v2.y - v0.y);
    const float zExtent = std::abs(v1.z - v0.z);

    assert(yExtent > 0.0f);
    assert(zExtent > 0.0f);

    emittedUnitFaces += static_cast<uint32_t>(
        (yExtent / VOXEL_SIZE) *
        (zExtent / VOXEL_SIZE));
  }

  std::cout << "Expected unit +X faces: " << expectedUnitFaces << '\n';
  std::cout << "Emitted unit +X area:  " << emittedUnitFaces << '\n';

  assert(expectedUnitFaces == emittedUnitFaces);
}

int main()
{
  Chunk chunk{};
  chunk.pos = {0, 0, 0};

  // Plane x = 0: a 2x2 rectangle.
  chunk.voxels[0][0][0].type = 1;
  chunk.voxels[0][0][1].type = 1;
  chunk.voxels[0][1][0].type = 1;
  chunk.voxels[0][1][1].type = 1;

  // This voxel hides one +X face from the x = 0 rectangle.
  chunk.voxels[1][0][0].type = 1;

  // Separate voxels on other X planes.
  chunk.voxels[2][2][2].type = 1;
  chunk.voxels[3][3][3].type = 1;

  ChunkMesh mesh = getChunkMesh(chunk);

  std::cout << "Vertices: " << mesh.vertices.size() << '\n';
  std::cout << "Indices:  " << mesh.indices.size() << "\n\n";

  validatePositiveXMesh(chunk, mesh);

  return EXIT_SUCCESS;
}
