#include <core/chunk_mesh_generator.hpp>
#include <bit>

ChunkMeshGenerator::ChunkMeshGenerator(World &w) : world(w)
{
}

void ChunkMeshGenerator::emitQuad(
    Axis axis,
    Face face,
    int slice,
    int row,
    int bit,
    int width,
    int height,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices)
{
  const float row0 = row * VOXEL_SIZE;
  const float row1 = (row + width) * VOXEL_SIZE;

  const float bit0 = bit * VOXEL_SIZE;
  const float bit1 = (bit + height) * VOXEL_SIZE;

  const float u1 = static_cast<float>(width);
  const float v1 = static_cast<float>(height);

  const glm::vec2 uv0{0.0f, 0.0f};
  const glm::vec2 uv1{u1, 0.0f};
  const glm::vec2 uv2{u1, v1};
  const glm::vec2 uv3{0.0f, v1};

  const uint32_t baseIndex =
      static_cast<uint32_t>(vertices.size());

  switch (axis)
  {
  case Axis::X:
  {
    const float x =
        ((face == Face::Positive) ? slice + 1 : slice) *
        VOXEL_SIZE;

    if (face == Face::Positive)
    {
      vertices.push_back({glm::vec3{x, row0, bit0}, uv0});
      vertices.push_back({glm::vec3{x, row0, bit1}, uv1});
      vertices.push_back({glm::vec3{x, row1, bit1}, uv2});
      vertices.push_back({glm::vec3{x, row1, bit0}, uv3});
    }
    else
    {
      vertices.push_back({glm::vec3{x, row1, bit0}, uv0});
      vertices.push_back({glm::vec3{x, row1, bit1}, uv1});
      vertices.push_back({glm::vec3{x, row0, bit1}, uv2});
      vertices.push_back({glm::vec3{x, row0, bit0}, uv3});
    }

    break;
  }

  case Axis::Y:
  {
    const float y =
        ((face == Face::Positive) ? slice + 1 : slice) *
        VOXEL_SIZE;

    if (face == Face::Positive)
    {
      vertices.push_back({glm::vec3{row0, y, bit0}, uv0});
      vertices.push_back({glm::vec3{row1, y, bit0}, uv1});
      vertices.push_back({glm::vec3{row1, y, bit1}, uv2});
      vertices.push_back({glm::vec3{row0, y, bit1}, uv3});
    }
    else
    {
      vertices.push_back({glm::vec3{row0, y, bit1}, uv0});
      vertices.push_back({glm::vec3{row1, y, bit1}, uv1});
      vertices.push_back({glm::vec3{row1, y, bit0}, uv2});
      vertices.push_back({glm::vec3{row0, y, bit0}, uv3});
    }

    break;
  }

  case Axis::Z:
  {
    const float z =
        ((face == Face::Positive) ? slice + 1 : slice) *
        VOXEL_SIZE;

    if (face == Face::Positive)
    {
      vertices.push_back({glm::vec3{row0, bit0, z}, uv0});
      vertices.push_back({glm::vec3{row1, bit0, z}, uv1});
      vertices.push_back({glm::vec3{row1, bit1, z}, uv2});
      vertices.push_back({glm::vec3{row0, bit1, z}, uv3});
    }
    else
    {
      vertices.push_back({glm::vec3{row0, bit1, z}, uv0});
      vertices.push_back({glm::vec3{row1, bit1, z}, uv1});
      vertices.push_back({glm::vec3{row1, bit0, z}, uv2});
      vertices.push_back({glm::vec3{row0, bit0, z}, uv3});
    }

    break;
  }
  }

  indices.push_back(baseIndex + 0);
  indices.push_back(baseIndex + 1);
  indices.push_back(baseIndex + 2);

  indices.push_back(baseIndex + 0);
  indices.push_back(baseIndex + 2);
  indices.push_back(baseIndex + 3);
}

BinaryGrids ChunkMeshGenerator::getBinaryGrids(Chunk &chunk)
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

void ChunkMeshGenerator::meshFace(
    const BinaryGrid &occupancy,
    Axis axis,
    Face face,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices)
{
  BinaryGrid mask{};

  for (int i = 0; i < CHUNK_SIZE; i++)
  {
    for (int j = 0; j < CHUNK_SIZE; j++)
    {
      uint32_t current = occupancy[i][j];
      uint32_t other;

      if (face == Face::Positive)
      {
        other = (i + 1 < CHUNK_SIZE) ? occupancy[i + 1][j] : 0;
      }
      else
      {
        other = (i > 0) ? occupancy[i - 1][j] : 0;
      }

      mask[i][j] = current & ~other;
    }
  }

  for (int slice = 0; slice < CHUNK_SIZE; slice++)
  {
    for (int row = 0; row < CHUNK_SIZE; row++)
    {
      uint32_t &bits = mask[slice][row];

      while (bits != 0)
      {
        uint32_t firstBit = std::countr_zero(bits);

        uint32_t shifted = bits >> firstBit;

        uint32_t height = std::countr_one(shifted);

        uint32_t bitMask = ((1u << height) - 1u) << firstBit;

        uint32_t width = 1;

        while (row + width < CHUNK_SIZE &&
               (mask[slice][row + width] & bitMask) == bitMask)
        {
          width++;
        }

        emitQuad(
            axis,
            face,
            slice,
            row,
            firstBit,
            width,
            height,
            vertices,
            indices);

        for (uint32_t w = 0; w < width; w++)
        {
          mask[slice][row + w] &= ~bitMask;
        }

        bits = mask[slice][row];
      }
    }
  }
}

ChunkMesh ChunkMeshGenerator::getChunkMesh(Chunk &chunk)
{
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  auto &pos = chunk.pos;

  BinaryGrids occupancy = getBinaryGrids(chunk);

  // X axis
  meshFace(occupancy.xy, Axis::X, Face::Positive, vertices, indices);
  meshFace(occupancy.xy, Axis::X, Face::Negative, vertices, indices);

  // Y axis
  meshFace(occupancy.xz, Axis::Y, Face::Positive, vertices, indices);
  meshFace(occupancy.xz, Axis::Y, Face::Negative, vertices, indices);

  // Z axis
  meshFace(occupancy.yz, Axis::Z, Face::Positive, vertices, indices);
  meshFace(occupancy.yz, Axis::Z, Face::Negative, vertices, indices);

  return {pos, vertices, indices};
}
