#ifndef CHUNK_MESH_GENERATOR_HPP
#define CHUNK_MESH_GENERATOR_HPP

#include <core/chunk_mesh.hpp>
#include <core/chunk.hpp>
#include <core/world.hpp>

using BinaryGrid = std::array<std::array<uint32_t, CHUNK_SIZE + 2>, CHUNK_SIZE + 2>;

enum Axis
{
  X,
  Y,
  Z
};

enum AxisDirection
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

struct FaceConfig
{
  Axis axis;
  AxisDirection axisDirection;
  BinaryGrid occupancy;
};

class ChunkMeshGenerator
{
  World &world;

  BinaryGrids getBinaryGrids(Chunk &chunk);
  BinaryGrid getAxisFaces(BinaryGrid &occupancy, bool positive);
  BinaryGrid swizzleFaces(BinaryGrid &axisFaces);
  void emitQuad(
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
      int size);
  void greedyMeshFaces(
      BinaryGrid &swizzledFaces,
      Axis axis,
      AxisDirection axisDirection,
      ChunkPos &chunkPos,
      std::vector<Vertex> &vertices,
      std::vector<uint32_t> &indices,
      int size);

public:
  ChunkMeshGenerator(World &w);

  ChunkMesh getChunkMesh(Chunk &chunk, int size);
};

#endif // CHUNK_MESH_GENERATOR_HPP
