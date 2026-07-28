#ifndef CHUNK_MESH_GENERATOR_HPP
#define CHUNK_MESH_GENERATOR_HPP

#include <core/chunk_mesh.hpp>
#include <core/chunk.hpp>
#include <core/world.hpp>

using BinaryGrid = std::array<std::array<uint32_t, CHUNK_SIZE>, CHUNK_SIZE>;

enum class Axis
{
  X,
  Y,
  Z
};

enum class Face
{
  Positive,
  Negative
};

struct BinaryGrids
{
  BinaryGrid xy{};
  BinaryGrid xz{};
  BinaryGrid yz{};
};

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

class ChunkMeshGenerator
{
  World &world;

  BinaryGrids getBinaryGrids(Chunk &chunk);
  void emitQuad(
      Axis axis,
      Face face,
      int slice,
      int row,
      int bit,
      int width,
      int height,
      std::vector<Vertex> &vertices,
      std::vector<uint32_t> &indices);
  void meshFace(
      const BinaryGrid &occupancy,
      Axis axis,
      Face face,
      std::vector<Vertex> &vertices,
      std::vector<uint32_t> &indices);

public:
  ChunkMeshGenerator(World &w);

  ChunkMesh getChunkMesh(Chunk &chunk);
};

#endif // CHUNK_MESH_GENERATOR_HPP
