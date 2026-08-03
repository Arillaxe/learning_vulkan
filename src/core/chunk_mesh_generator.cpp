#include <core/chunk_mesh_generator.hpp>
#include <bit>

ChunkMeshGenerator::ChunkMeshGenerator(World &w) : world(w) {}

BinaryGrids ChunkMeshGenerator::getBinaryGrids(Chunk &chunk)
{
  BinaryGrids grids{};

  const auto &voxels = chunk.voxels;
  const auto &chunkPos = chunk.pos;

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

  if (Chunk *east = world.getChunk(chunkPos.x + 1, chunkPos.y, chunkPos.z))
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

  if (Chunk *west = world.getChunk(chunkPos.x - 1, chunkPos.y, chunkPos.z))
  {
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
      for (int z = 0; z < CHUNK_SIZE; z++)
      {
        if (west->voxels[CHUNK_SIZE - 1][y][z].type != 0)
          grids.yz[y + 1][z + 1] |= (1u << (0));
      }
    }
  }

  if (Chunk *top = world.getChunk(chunkPos.x, chunkPos.y + 1, chunkPos.z))
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      for (int z = 0; z < CHUNK_SIZE; z++)
      {
        if (top->voxels[x][0][z].type != 0)
          grids.xz[x + 1][z + 1] |= (1u << (CHUNK_SIZE + 1));
      }
    }
  }

  if (Chunk *top = world.getChunk(chunkPos.x, chunkPos.y - 1, chunkPos.z))
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      for (int z = 0; z < CHUNK_SIZE; z++)
      {
        if (top->voxels[x][CHUNK_SIZE - 1][z].type != 0)
          grids.xz[x + 1][z + 1] |= (1u << (0));
      }
    }
  }

  if (Chunk *north = world.getChunk(chunkPos.x, chunkPos.y, chunkPos.z + 1))
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      for (int y = 0; y < CHUNK_SIZE; y++)
      {
        if (north->voxels[x][y][0].type != 0)
          grids.xy[x + 1][y + 1] |= (1u << (CHUNK_SIZE + 1));
      }
    }
  }

  if (Chunk *south = world.getChunk(chunkPos.x, chunkPos.y, chunkPos.z - 1))
  {
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      for (int y = 0; y < CHUNK_SIZE; y++)
      {
        if (south->voxels[x][y][CHUNK_SIZE - 1].type != 0)
          grids.xy[x + 1][y + 1] |= (1u << (0));
      }
    }
  }

  return grids;
}

BinaryGrid ChunkMeshGenerator::getAxisFaces(BinaryGrid &occupancy, bool positive)
{
  BinaryGrid axisFaces{};

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      uint32_t row = occupancy[i + 1][j + 1];
      uint32_t shifted = positive ? row >> 1 : row << 1;
      axisFaces[i + 1][j + 1] = row & ~(shifted);
    }
  }

  return axisFaces;
}

BinaryGrid ChunkMeshGenerator::swizzleFaces(BinaryGrid &axisFaces)
{
  BinaryGrid swizzledFaces{};

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      uint32_t row = axisFaces[i + 1][j + 1];

      while (row)
      {
        uint32_t k = std::countr_zero(row);
        swizzledFaces[i + 1][k] |= (1u << j);
        row &= row - 1;
      }
    }
  }

  return swizzledFaces;
}

void ChunkMeshGenerator::emitQuad(
    Axis axis,
    AxisDirection axisDirection,
    int slice,
    int row,
    int bit,
    int width,
    int height,
    ChunkPos &chunkPos,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices,
    int size)
{
  glm::ivec3 origin = glm::ivec3(chunkPos.x, chunkPos.y, chunkPos.z) * CHUNK_SIZE;
  glm::vec3 v0, v1, v2, v3;
  float shade;

  switch (axis)
  {
  case Axis::X:
  {
    shade = 0.8f;

    float x = origin.x + (axisDirection == AxisDirection::Positive ? slice + 1 : slice);

    v0 = {x, origin.y + row, origin.z + bit};
    v1 = {x, origin.y + row, origin.z + bit + height};
    v2 = {x, origin.y + row + width, origin.z + bit};
    v3 = {x, origin.y + row + width, origin.z + bit + height};
    break;
  }

  case Axis::Y:
  {
    shade = axisDirection == AxisDirection::Positive ? 1.0f : 0.5f;

    float y = origin.y + (axisDirection == AxisDirection::Positive ? slice + 1 : slice);

    v0 = {origin.x + row, y, origin.z + bit};
    v1 = {origin.x + row + width, y, origin.z + bit};
    v2 = {origin.x + row, y, origin.z + bit + height};
    v3 = {origin.x + row + width, y, origin.z + bit + height};
    break;
  }

  case Axis::Z:
  {
    shade = 0.65;

    float z = origin.z + (axisDirection == AxisDirection::Positive ? slice + 1 : slice);

    v0 = {origin.x + row, origin.y + bit, z};
    v1 = {origin.x + row, origin.y + bit + height, z};
    v2 = {origin.x + row + width, origin.y + bit, z};
    v3 = {origin.x + row + width, origin.y + bit + height, z};
    break;
  }
  }

  uint32_t baseVertex = static_cast<uint32_t>(vertices.size());

  vertices.push_back({v0 * (float)VOXEL_SIZE * (float)size, {0, 0}, shade});
  vertices.push_back({v1 * (float)VOXEL_SIZE * (float)size, {0, 0}, shade});
  vertices.push_back({v2 * (float)VOXEL_SIZE * (float)size, {0, 0}, shade});
  vertices.push_back({v3 * (float)VOXEL_SIZE * (float)size, {0, 0}, shade});

  if (axisDirection == AxisDirection::Positive)
  {
    indices.insert(indices.end(), {baseVertex + 0, baseVertex + 2, baseVertex + 1,
                                   baseVertex + 1, baseVertex + 2, baseVertex + 3});
  }
  else
  {
    indices.insert(indices.end(), {baseVertex + 0, baseVertex + 1, baseVertex + 2,
                                   baseVertex + 1, baseVertex + 3, baseVertex + 2});
  }
}
void ChunkMeshGenerator::greedyMeshFaces(
    BinaryGrid &swizzledFaces,
    Axis axis,
    AxisDirection axisDirection,
    ChunkPos &chunkPos,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices,
    int size)
{
  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      uint32_t &row = swizzledFaces[i + 1][j + 1];

      while (row > 0)
      {
        uint32_t offset = std::countr_zero(row);
        uint32_t offsetedRow = row >> offset;
        uint32_t height = std::countr_one(offsetedRow);
        uint32_t mask = ((1u << height) - 1) << offset;

        uint32_t width = 1;
        for (int k = i + 1; k < CHUNK_SIZE; k++)
        {
          if ((swizzledFaces[k + 1][j + 1] & mask) != mask)
            break;
          width++;
          swizzledFaces[k + 1][j + 1] &= ~mask;
        }

        emitQuad(axis, axisDirection, j, i, offset, width, height, chunkPos, vertices, indices, size);

        row &= ~mask;
      }
    }
  }
}

ChunkMesh ChunkMeshGenerator::getChunkMesh(Chunk &chunk, int size)
{
  std::vector<Vertex> vertices{};
  std::vector<uint32_t> indices{};

  auto &chunkPos = chunk.pos;

  BinaryGrids occupancy = getBinaryGrids(chunk);

  std::array<FaceConfig, 6> faceConfigs = {{
      {Axis::X, AxisDirection::Positive, occupancy.yz},
      {Axis::X, AxisDirection::Negative, occupancy.yz},
      {Axis::Y, AxisDirection::Positive, occupancy.xz},
      {Axis::Y, AxisDirection::Negative, occupancy.xz},
      {Axis::Z, AxisDirection::Positive, occupancy.xy},
      {Axis::Z, AxisDirection::Negative, occupancy.xy},
  }};

  for (auto &config : faceConfigs)
  {
    BinaryGrid axisFaces = getAxisFaces(config.occupancy, config.axisDirection == AxisDirection::Positive);
    BinaryGrid axisFacesSwizzled = swizzleFaces(axisFaces);
    greedyMeshFaces(axisFacesSwizzled, config.axis, config.axisDirection, chunkPos, vertices, indices, size);
  }

  return {chunkPos, vertices, indices};
}
